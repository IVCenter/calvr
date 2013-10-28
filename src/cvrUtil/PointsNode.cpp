#include <cvrUtil/PointsNode.h>
#include <cvrUtil/LocalToWorldVisitor.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace cvr;

//OpenThreads::Mutex PointsNode::_textureCreationLock;
//osg::ref_ptr<osg::Texture2D> PointsNode::_sphereTexture;

std::string vertPointShaderSrc =
        "#version 150 compatibility                         \n\
#extension GL_ARB_gpu_shader5 : enable              \n\
#extension GL_ARB_explicit_attrib_location : enable \n\
                                                    \n\
layout(location = 4) in float size;                 \n\
                                                    \n\
void main(void)                                     \n\
{                                                   \n\
                                                    \n\
    gl_FrontColor = gl_Color;                       \n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;   \n\
    gl_PointSize = size;                            \n\
}                                                   \n";

std::string fragPointShaderSrc =
        "#version 150 compatibility                         \n\
#extension GL_ARB_gpu_shader5 : enable              \n\
                                                    \n\
void main(void)                                     \n\
{                                                   \n\
    gl_FragColor = gl_Color;                        \n\
}                                                   \n";

std::string vertSphereShaderSrc =
        "#version 150 compatibility                         \n\
#extension GL_ARB_gpu_shader5 : enable              \n\
#extension GL_ARB_explicit_attrib_location : enable \n\
                                                    \n\
layout(location = 4) in float size;                 \n\
                                                    \n\
out float pointSize;                                \n\
                                                    \n\
void main(void)                                     \n\
{                                                   \n\
                                                    \n\
    gl_FrontColor = gl_Color;                       \n\
    gl_Position = gl_ModelViewMatrix * gl_Vertex;   \n\
    pointSize = size;                               \n\
}                                                   \n";

std::string geomSphereShaderSrc =
        "#version 150 compatibility                         \n\
#extension GL_EXT_geometry_shader4: enable          \n\
#extension GL_ARB_gpu_shader5 : enable              \n\
                                                    \n\
uniform float objectScale;                          \n\
in float pointSize[];                               \n\
                                                    \n\
flat out vec3 vertex_light_position;                \n\
flat out vec4 eye_position;                         \n\
flat out float sphere_radius;                       \n\
                                                    \n\
void main(void)                                     \n\
{                                                   \n\
        sphere_radius =  objectScale * pointSize[0] * 0.5;                     \n\
                                                                               \n\
        gl_FrontColor = gl_FrontColorIn[0];                                    \n\
        gl_FrontColor.a = 1.0;                                                 \n\
                                                                               \n\
        vertex_light_position = normalize(gl_LightSource[0].position.xyz);     \n\
        eye_position = gl_PositionIn[0];                                       \n\
                                                                               \n\
        gl_TexCoord[0].st = vec2(-1.0,-1.0);                                   \n\
        gl_Position = gl_PositionIn[0];                                        \n\
        gl_Position.xy += vec2(-sphere_radius, -sphere_radius);                \n\
        gl_Position = gl_ProjectionMatrix * gl_Position;                       \n\
        EmitVertex();                                                          \n\
                                                                               \n\
        gl_TexCoord[0].st = vec2(-1.0,1.0);                                    \n\
        gl_Position = gl_PositionIn[0];                                        \n\
        gl_Position.xy += vec2(-sphere_radius, sphere_radius);                 \n\
        gl_Position = gl_ProjectionMatrix * gl_Position;                       \n\
        EmitVertex();                                                          \n\
                                                                               \n\
        gl_TexCoord[0].st = vec2(1.0,-1.0);                                    \n\
        gl_Position = gl_PositionIn[0];                                        \n\
        gl_Position.xy += vec2(sphere_radius, -sphere_radius);                 \n\
        gl_Position = gl_ProjectionMatrix * gl_Position;                       \n\
        EmitVertex();                                                          \n\
                                                                               \n\
        gl_TexCoord[0].st = vec2(1.0,1.0);                                     \n\
        gl_Position = gl_PositionIn[0];                                        \n\
        gl_Position.xy += vec2(sphere_radius, sphere_radius);                  \n\
        gl_Position = gl_ProjectionMatrix * gl_Position;                       \n\
        EmitVertex();                                                          \n\
                                                                               \n\
        EndPrimitive();                                                        \n\
}                                                                              \n";

std::string fragSphereShaderSrc =
        "#version 150 compatibility                                                    \n\
#extension GL_ARB_gpu_shader5 : enable                                         \n\
                                                                               \n\
flat in float sphere_radius;                                                   \n\
flat in vec3 vertex_light_position;                                            \n\
flat in vec4 eye_position;                                                     \n\
                                                                               \n\
uniform sampler2D tex;	       						       \n\
                                                                               \n\
void main (void)                                                               \n\
{                                                                              \n\
                                                                               \n\
    //vec4 value = texture2D(tex,gl_TexCoord[0].st);                           \n\
    float x = gl_TexCoord[0].x;                                                \n\
    float y = gl_TexCoord[0].y;                                                \n\
    float zz = 1.0 - x*x - y*y;                                                \n\
                                                                               \n\
    if (zz <= 0.0 )                                                            \n\
        discard;                                                               \n\
                                                                               \n\
    float z = sqrt(zz);                                                        \n\
                                                                               \n\
    vec3 normal = vec3(x, y, z);                                               \n\
                                                                               \n\
    // Lighting                                                                \n\
    float diffuse_value = max(dot(normal, vertex_light_position), 0.0);        \n\
                                                                               \n\
    vec4 pos = eye_position;                                                   \n\
    pos.z += z*sphere_radius;                                                  \n\
    pos = gl_ProjectionMatrix * pos;                                           \n\
                                                                               \n\
    gl_FragDepth = (pos.z / pos.w + 1.0) / 2.0;                                \n\
    gl_FragColor = gl_Color * diffuse_value;                                   \n\
}                                                                              \n";

PointsNode::PointsNode(PointsMode mode, int startingNumPoints,
        float defaultPointSize, float defaultRadius, osg::Vec4ub defaultColor,
        PointsBinding sizeBinding, PointsBinding radiusBinding,
        PointsBinding colorBinding) :
        osg::Group()
{
    init(mode,startingNumPoints,defaultPointSize,defaultRadius,defaultColor,
            sizeBinding,radiusBinding,colorBinding);
}

PointsNode::PointsNode(PointsMode mode, int startingNumPoints,
        float defaultPointSize, float defaultRadius, osg::Vec4 defaultColor,
        PointsBinding sizeBinding, PointsBinding radiusBinding,
        PointsBinding colorBinding) :
        osg::Group()
{
    osg::Vec4ub newColor;
    newColor.r() = (unsigned char)(defaultColor.x() * 255.0);
    newColor.g() = (unsigned char)(defaultColor.y() * 255.0);
    newColor.b() = (unsigned char)(defaultColor.z() * 255.0);
    newColor.a() = (unsigned char)(defaultColor.w() * 255.0);
    init(mode,startingNumPoints,defaultPointSize,defaultRadius,newColor,
            sizeBinding,radiusBinding,colorBinding);
}

PointsNode::PointsNode(const PointsNode & pn, const osg::CopyOp & copyop)
{
}

PointsNode::~PointsNode()
{
}

void PointsNode::setVertexArray(osg::Vec3Array * vertArray)
{
    if(vertArray)
    {
        _vertArray = vertArray;
        refreshGeometry();
        calcSize();
    }
}

void PointsNode::setColorArray(osg::Vec4ubArray * colorArray)
{
    if(colorArray)
    {
        _colorArray = colorArray;
        refreshGeometry();
        calcSize();
    }
}

void PointsNode::setColor(osg::Vec4ub color)
{
    if(_colorBinding == PointsNode::POINTS_OVERALL)
    {
        if(_colorArray->size())
        {
            _colorArray->at(0) = color;
        }
    }
    else
    {
        for(int i = 0; i < _colorArray->size(); i++)
        {
            _colorArray->at(i) = color;
        }
    }
    _colorArray->dirty();
    _pointColor = color;
}

void PointsNode::setColor(osg::Vec4 color)
{
    osg::Vec4ub newColor;
    newColor.r() = (unsigned char)(color.x() * 255.0);
    newColor.g() = (unsigned char)(color.y() * 255.0);
    newColor.b() = (unsigned char)(color.z() * 255.0);
    newColor.a() = (unsigned char)(color.w() * 255.0);
    setColor(newColor);
}

void PointsNode::setPointSizeArray(osg::FloatArray * sizeArray)
{
    if(sizeArray)
    {
        _sizeArray = sizeArray;
        refreshGeometry();
        calcSize();
    }
}

void PointsNode::setPointSize(float size)
{
    if(_sizeBinding == PointsNode::POINTS_OVERALL)
    {
        if(_sizeArray->size())
        {
            _sizeArray->at(0) = size;
        }
    }
    else
    {
        for(int i = 0; i < _sizeArray->size(); i++)
        {
            _sizeArray->at(i) = size;
        }
    }
    _point->setSize(size);
    _sizeArray->dirty();
    _pointSize = size;
}

void PointsNode::setRadiusArray(osg::FloatArray * radiusArray)
{
    if(radiusArray)
    {
        _radiusArray = radiusArray;
        refreshGeometry();
        calcSize();
    }
}

void PointsNode::setRadius(float radius)
{
    if(_radiusBinding == PointsNode::POINTS_OVERALL)
    {
        if(_radiusArray->size())
        {
            _radiusArray->at(0) = radius;
        }
    }
    else
    {
        for(int i = 0; i < _radiusArray->size(); i++)
        {
            _radiusArray->at(i) = radius;
        }
    }
    _radiusArray->dirty();
    _pointRadius = radius;
}

void PointsNode::setPoint(int pointIndex, osg::Vec3 position, osg::Vec4ub color,
        float radius, float size)
{
    if(pointIndex >= 0 && pointIndex < _vertArray->size())
    {
        _vertArray->at(pointIndex) = position;
        _vertArray->dirty();
    }

    if(pointIndex >= 0 && pointIndex < _colorArray->size())
    {
        _colorArray->at(pointIndex) = color;
        _colorArray->dirty();
    }

    if(pointIndex >= 0 && pointIndex < _radiusArray->size())
    {
        _radiusArray->at(pointIndex) = radius;
        _radiusArray->dirty();
    }

    if(pointIndex >= 0 && pointIndex < _sizeArray->size())
    {
        _sizeArray->at(pointIndex) = size;
        if(!pointIndex)
        {
            _point->setSize(size);
        }
        _sizeArray->dirty();
    }
}

void PointsNode::setPoint(int pointIndex, osg::Vec3 position, osg::Vec4 color,
        float radius, float size)
{
    osg::Vec4ub newColor;
    newColor.r() = (unsigned char)(color.x() * 255.0);
    newColor.g() = (unsigned char)(color.y() * 255.0);
    newColor.b() = (unsigned char)(color.z() * 255.0);
    newColor.a() = (unsigned char)(color.w() * 255.0);
    setPoint(pointIndex,position,newColor,radius,size);
}

void PointsNode::setPointPosition(int pointIndex, osg::Vec3 position)
{
    if(pointIndex >= 0 && pointIndex < _vertArray->size())
    {
        _vertArray->at(pointIndex) = position;
        _vertArray->dirty();
    }
}

void PointsNode::setPointColor(int pointIndex, osg::Vec4ub color)
{
    if(pointIndex >= 0 && pointIndex < _colorArray->size())
    {
        _colorArray->at(pointIndex) = color;
        _colorArray->dirty();
    }
}

void PointsNode::setPointColor(int pointIndex, osg::Vec4 color)
{
    osg::Vec4ub newColor;
    newColor.r() = (unsigned char)(color.x() * 255.0);
    newColor.g() = (unsigned char)(color.y() * 255.0);
    newColor.b() = (unsigned char)(color.z() * 255.0);
    newColor.a() = (unsigned char)(color.w() * 255.0);
    setPointColor(pointIndex,newColor);
}

void PointsNode::setPointRadius(int pointIndex, float radius)
{
    if(pointIndex >= 0 && pointIndex < _radiusArray->size())
    {
        _radiusArray->at(pointIndex) = radius;
        _radiusArray->dirty();
    }
}

void PointsNode::setPointSize(int pointIndex, float size)
{
    if(pointIndex >= 0 && pointIndex < _sizeArray->size())
    {
        _sizeArray->at(pointIndex) = size;
        if(!pointIndex)
        {
            _point->setSize(size);
        }
        _sizeArray->dirty();
    }
}

osg::Vec3 PointsNode::getPointPosition(int pointIndex)
{
    if(pointIndex >= 0 && pointIndex < _vertArray->size())
    {
        return _vertArray->at(pointIndex);
    }
    return osg::Vec3();
}

osg::Vec4ub PointsNode::getPointColor(int pointIndex)
{
    if(_colorBinding == POINTS_OVERALL)
    {
        if(_colorArray->size())
        {
            return _colorArray->at(0);
        }
    }
    else
    {
        if(pointIndex >= 0 && pointIndex < _colorArray->size())
        {
            return _colorArray->at(pointIndex);
        }
    }
    return osg::Vec4ub();
}

osg::Vec4 PointsNode::getPointColorF(int pointIndex)
{
    osg::Vec4ub color;

    if(_colorBinding == POINTS_OVERALL)
    {
        if(_colorArray->size())
        {
            color = _colorArray->at(0);
        }
    }
    else
    {
        if(pointIndex >= 0 && pointIndex < _colorArray->size())
        {
            color = _colorArray->at(pointIndex);
        }
    }

    return osg::Vec4(((float)color.r()) / 255.0f,((float)color.g()) / 255.0f,
            ((float)color.b()) / 255.0f,((float)color.a()) / 255.0f);
}

float PointsNode::getPointRadius(int pointIndex)
{
    if(_radiusBinding == POINTS_OVERALL)
    {
        if(_radiusArray->size())
        {
            return _radiusArray->at(0);
        }
    }
    else
    {
        if(pointIndex >= 0 && pointIndex < _radiusArray->size())
        {
            return _radiusArray->at(pointIndex);
        }
    }
    return 0.0f;
}

float PointsNode::getPointSize(int pointIndex)
{
    if(_sizeBinding == POINTS_OVERALL)
    {
        if(_sizeArray->size())
        {
            return _sizeArray->at(0);
        }
    }
    else
    {
        if(pointIndex >= 0 && pointIndex < _sizeArray->size())
        {
            return _sizeArray->at(pointIndex);
        }
    }
    return 0.0f;
}

void PointsNode::addPoint(osg::Vec3 position)
{
    int index = _size;

    if(_vertArray->size() == index)
    {
        _vertArray->push_back(position);
    }
    else
    {
        _vertArray->at(index) = position;
    }
    _vertArray->dirty();

    if(_colorBinding == POINTS_OVERALL)
    {
        if(!_colorArray->size())
        {
            _colorArray->push_back(_pointColor);
            _colorArray->dirty();
        }
    }
    else
    {
        if(_colorArray->size() == index)
        {
            _colorArray->push_back(_pointColor);
        }
        else
        {
            _colorArray->at(index) = _pointColor;
        }
        _colorArray->dirty();
    }

    if(_radiusBinding == POINTS_OVERALL)
    {
        if(!_radiusArray->size())
        {
            _radiusArray->push_back(_pointRadius);
            _radiusArray->dirty();
        }
    }
    else
    {
        if(_radiusArray->size() == index)
        {
            _radiusArray->push_back(_pointRadius);
        }
        else
        {
            _radiusArray->at(index) = _pointRadius;
        }
        _radiusArray->dirty();
    }

    if(_sizeBinding == POINTS_OVERALL)
    {
        if(!_sizeArray->size())
        {
            _sizeArray->push_back(_pointSize);
            _sizeArray->dirty();
        }
    }
    else
    {
        if(_sizeArray->size() == index)
        {
            _sizeArray->push_back(_pointSize);
        }
        else
        {
            _sizeArray->at(index) = _pointSize;
        }
        _sizeArray->dirty();
    }

    calcSize();
}

void PointsNode::addPoint(osg::Vec3 position, osg::Vec4ub color, float radius,
        float size)
{
    int index = _size;

    if(_vertArray->size() == index)
    {
        _vertArray->push_back(position);
    }
    else
    {
        _vertArray->at(index) = position;
    }
    _vertArray->dirty();

    if(_colorBinding == POINTS_OVERALL)
    {
        if(!_colorArray->size())
        {
            _colorArray->push_back(color);
            _colorArray->dirty();
        }
    }
    else
    {
        if(_colorArray->size() == index)
        {
            _colorArray->push_back(color);
        }
        else
        {
            _colorArray->at(index) = color;
        }
        _colorArray->dirty();
    }

    if(_radiusBinding == POINTS_OVERALL)
    {
        if(!_radiusArray->size())
        {
            if(radius > 0.0)
            {
                _radiusArray->push_back(radius);
            }
            else
            {
                _radiusArray->push_back(_pointRadius);
            }
            _radiusArray->dirty();
        }
        else if(!index)
        {
            if(radius > 0.0)
            {
                _radiusArray->at(0) = radius;
                _radiusArray->dirty();
            }
        }
    }
    else
    {
        if(_radiusArray->size() == index)
        {
            if(radius > 0.0)
            {
                _radiusArray->push_back(radius);
            }
            else
            {
                _radiusArray->push_back(_pointRadius);
            }
        }
        else
        {
            if(radius > 0.0)
            {
                _radiusArray->at(index) = radius;
            }
            else
            {
                _radiusArray->at(index) = _pointRadius;
            }
        }
        _radiusArray->dirty();
    }

    if(_sizeBinding == POINTS_OVERALL)
    {
        if(!_sizeArray->size())
        {
            if(size > 0.0)
            {
                _sizeArray->push_back(size);
            }
            else
            {
                _sizeArray->push_back(_pointSize);
            }
            _sizeArray->dirty();
        }
        else if(!index)
        {
            if(size > 0.0)
            {
                _sizeArray->at(0) = size;
                _sizeArray->dirty();
            }
        }
    }
    else
    {
        if(_sizeArray->size() == index)
        {
            if(size > 0.0)
            {
                _sizeArray->push_back(size);
            }
            else
            {
                _sizeArray->push_back(_pointSize);
            }
        }
        else
        {
            if(size > 0.0)
            {
                _sizeArray->at(index) = size;
            }
            else
            {
                _sizeArray->at(index) = _pointSize;
            }
        }
        _sizeArray->dirty();
    }

    calcSize();
}

void PointsNode::addPoint(osg::Vec3 position, osg::Vec4 color, float radius,
        float size)
{
    osg::Vec4ub newColor;
    newColor.r() = (unsigned char)(color.x() * 255.0);
    newColor.g() = (unsigned char)(color.y() * 255.0);
    newColor.b() = (unsigned char)(color.z() * 255.0);
    newColor.a() = (unsigned char)(color.w() * 255.0);

    addPoint(position,newColor,radius,size);
}

void PointsNode::removePoint(int pointIndex)
{
    if(pointIndex >= 0 && pointIndex < _vertArray->size())
    {
        osg::Vec3Array::iterator it = _vertArray->begin();
        it = it + pointIndex;
        _vertArray->erase(it);
        _vertArray->dirty();
    }

    if(_colorBinding != POINTS_OVERALL)
    {
        if(pointIndex >= 0 && pointIndex < _colorArray->size())
        {
            osg::Vec4ubArray::iterator it = _colorArray->begin();
            it = it + pointIndex;
            _colorArray->erase(it);
            _colorArray->dirty();
        }
    }

    if(_radiusBinding != POINTS_OVERALL)
    {
        if(pointIndex >= 0 && pointIndex < _radiusArray->size())
        {
            osg::FloatArray::iterator it = _radiusArray->begin();
            it = it + pointIndex;
            _radiusArray->erase(it);
            _radiusArray->dirty();
        }
    }

    if(_sizeBinding != POINTS_OVERALL)
    {
        if(pointIndex >= 0 && pointIndex < _sizeArray->size())
        {
            osg::FloatArray::iterator it = _sizeArray->begin();
            it = it + pointIndex;
            _sizeArray->erase(it);
            _sizeArray->dirty();
        }
    }

    calcSize();
}

void PointsNode::removePoints(int startPoint, int numPoints)
{
    int endPoint = std::min(startPoint + numPoints,_size);
    if(startPoint >= endPoint)
    {
        return;
    }

    if(startPoint >= 0 && startPoint < _vertArray->size())
    {
        osg::Vec3Array::iterator its = _vertArray->begin();
        its = its + startPoint;
        osg::Vec3Array::iterator ite = _vertArray->begin();
        ite = ite + (endPoint);
        _vertArray->erase(its,ite);
        _vertArray->dirty();
    }

    if(_colorBinding != POINTS_OVERALL)
    {
        if(startPoint >= 0 && startPoint < _colorArray->size())
        {
            osg::Vec4ubArray::iterator its = _colorArray->begin();
            its = its + startPoint;
            osg::Vec4ubArray::iterator ite = _colorArray->begin();
            ite = ite + (endPoint);
            _colorArray->erase(its,ite);
            _colorArray->dirty();
        }
    }

    if(_radiusBinding != POINTS_OVERALL)
    {
        if(startPoint >= 0 && startPoint < _radiusArray->size())
        {
            osg::FloatArray::iterator its = _radiusArray->begin();
            its = its + startPoint;
            osg::FloatArray::iterator ite = _radiusArray->begin();
            ite = ite + (endPoint);
            _radiusArray->erase(its,ite);
            _radiusArray->dirty();
        }
    }

    if(_sizeBinding != POINTS_OVERALL)
    {
        if(startPoint >= 0 && startPoint < _sizeArray->size())
        {
            osg::FloatArray::iterator its = _sizeArray->begin();
            its = its + startPoint;
            osg::FloatArray::iterator ite = _sizeArray->begin();
            ite = ite + (endPoint);
            _sizeArray->erase(its,ite);
            _sizeArray->dirty();
        }
    }

    calcSize();
}

void PointsNode::clear()
{
    _vertArray->erase(_vertArray->begin(),_vertArray->end());
    _colorArray->erase(_colorArray->begin(),_colorArray->end());
    _radiusArray->erase(_radiusArray->begin(),_radiusArray->end());
    _sizeArray->erase(_sizeArray->begin(),_sizeArray->end());

    if(_colorBinding == POINTS_OVERALL)
    {
        _colorArray->push_back(_pointColor);
    }
    if(_radiusBinding == POINTS_OVERALL)
    {
        _radiusArray->push_back(_pointRadius);
    }
    if(_sizeBinding == POINTS_OVERALL)
    {
        _sizeArray->push_back(_pointSize);
    }

    _vertArray->dirty();
    _colorArray->dirty();
    _radiusArray->dirty();
    _sizeArray->dirty();

    calcSize();
}

void PointsNode::setSpriteTexture(osg::Texture2D * texture)
{
    if(_spriteTexture)
    {
        getOrCreateStateSet()->removeAssociatedTextureModes(0,_spriteTexture);
        getOrCreateStateSet()->removeTextureAttribute(0,
                osg::StateAttribute::TEXTURE);
    }

    _spriteTexture = texture;

    if(_mode == POINTS_POINT_SPRITES)
    {
        setPointsMode(_mode);
    }
}

void PointsNode::setPointsMode(PointsMode mode)
{
    _geometry->setVertexAttribArray(4,NULL);
    getOrCreateStateSet()->removeAttribute(osg::StateAttribute::PROGRAM);
    getOrCreateStateSet()->removeMode(GL_VERTEX_PROGRAM_POINT_SIZE);
    getOrCreateStateSet()->removeAttribute(osg::StateAttribute::POINT);
    getOrCreateStateSet()->removeAssociatedModes(_blendFunc);
    getOrCreateStateSet()->removeAssociatedModes(_pointSprite);
    getOrCreateStateSet()->removeAttribute(osg::StateAttribute::POINTSPRITE);
    getOrCreateStateSet()->removeAttribute(osg::StateAttribute::BLENDFUNC);
    if(_spriteTexture)
    {
        getOrCreateStateSet()->removeAssociatedTextureModes(0,_spriteTexture);
        getOrCreateStateSet()->removeTextureAttribute(0,
                osg::StateAttribute::TEXTURE);
    }

    switch(mode)
    {
        case POINTS_GL_POINTS:
        {
            if(_sizeBinding == POINTS_OVERALL)
            {
                _geometry->setVertexAttribBinding(4,
                        osg::Geometry::BIND_OVERALL);
            }
            else
            {
                _geometry->setVertexAttribBinding(4,
                        osg::Geometry::BIND_PER_VERTEX);
            }
            _geometry->setVertexAttribArray(4,_sizeArray);
            _geometry->setVertexAttribNormalize(4,false);

            if(!_programPoints)
            {
                _programPoints = new osg::Program();
                _programPoints->setName("Points");
                _programPoints->addShader(
                        new osg::Shader(osg::Shader::VERTEX,
                                vertPointShaderSrc));
                //_programPoints->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragPointShaderSrc));
            }
            getOrCreateStateSet()->setAttribute(_programPoints);
            getOrCreateStateSet()->setMode(GL_VERTEX_PROGRAM_POINT_SIZE,
                    osg::StateAttribute::ON);
            break;
        }
        case POINTS_SHADED_SPHERES:
        {
            if(_radiusBinding == POINTS_OVERALL)
            {
                _geometry->setVertexAttribBinding(4,
                        osg::Geometry::BIND_OVERALL);
            }
            else
            {
                _geometry->setVertexAttribBinding(4,
                        osg::Geometry::BIND_PER_VERTEX);
            }
            _geometry->setVertexAttribArray(4,_radiusArray);
            _geometry->setVertexAttribNormalize(4,false);

            if(!_programSphere)
            {
                _programSphere = new osg::Program();
                _programSphere->setName("PointSphere");
                _programSphere->addShader(
                        new osg::Shader(osg::Shader::VERTEX,
                                vertSphereShaderSrc));
                _programSphere->addShader(
                        new osg::Shader(osg::Shader::GEOMETRY,
                                geomSphereShaderSrc));
                _programSphere->addShader(
                        new osg::Shader(osg::Shader::FRAGMENT,
                                fragSphereShaderSrc));
                _programSphere->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT,4);
                _programSphere->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT,
                        GL_POINTS);
                _programSphere->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT,
                        GL_TRIANGLE_STRIP);
            }
            getOrCreateStateSet()->setAttribute(_programSphere);
            break;
        }
        case POINTS_POINT_SPRITES:
        {
            getOrCreateStateSet()->setAttribute(_point);
            getOrCreateStateSet()->setAttributeAndModes(_blendFunc,
                    osg::StateAttribute::ON);
            getOrCreateStateSet()->setTextureAttributeAndModes(0,_pointSprite,
                    osg::StateAttribute::ON);
            if(_spriteTexture)
            {
                getOrCreateStateSet()->setTextureAttributeAndModes(0,
                        _spriteTexture,osg::StateAttribute::ON);
            }
            break;
        }
        default:
            break;
    }
    calcSize();
}

void PointsNode::init(PointsMode mode, int startingNumPoints,
        float defaultPointSize, float defaultRadius, osg::Vec4ub & defaultColor,
        PointsBinding sizeBinding, PointsBinding radiusBinding,
        PointsBinding colorBinding)
{
    _size = startingNumPoints;
    _colorBinding = colorBinding;
    _sizeBinding = sizeBinding;
    _radiusBinding = radiusBinding;
    _mode = mode;

    _pointSize = defaultPointSize;
    _pointColor = defaultColor;
    _pointRadius = defaultRadius;

    _geometry = new osg::Geometry();
    _geode = new osg::Geode();

    _geode->addDrawable(_geometry);
    addChild(_geode);

    _geometry->setUseDisplayList(false);
    _geometry->setUseVertexBufferObjects(true);

    _vertArray = new osg::Vec3Array(_size);
    _geometry->setVertexArray(_vertArray);

    if(_colorBinding == POINTS_OVERALL)
    {
        _colorArray = new osg::Vec4ubArray(1);
        _colorArray->at(0) = _pointColor;
        _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    }
    else
    {
        _colorArray = new osg::Vec4ubArray(_size);
        for(int i = 0; i < _size; i++)
        {
            _colorArray->at(i) = _pointColor;
        }
        _geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    _geometry->setColorArray(_colorArray);

    if(_sizeBinding == POINTS_OVERALL)
    {
        _sizeArray = new osg::FloatArray(1);
        _sizeArray->at(0) = _pointSize;
    }
    else
    {
        _sizeArray = new osg::FloatArray(_size);
        for(int i = 0; i < _size; i++)
        {
            _sizeArray->at(i) = _pointSize;
        }
    }

    if(_radiusBinding == POINTS_OVERALL)
    {
        _radiusArray = new osg::FloatArray(1);
        _radiusArray->at(0) = _pointRadius;
    }
    else
    {
        _radiusArray = new osg::FloatArray(_size);
        for(int i = 0; i < _size; i++)
        {
            _radiusArray->at(i) = _pointRadius;
        }
    }

    _primitive = new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,_size);
    _geometry->addPrimitiveSet(_primitive);

    _scaleUni = new osg::Uniform(osg::Uniform::FLOAT,"objectScale");
    _scaleUni->set(1.0f);
    getOrCreateStateSet()->addUniform(_scaleUni);

    _updateCallback = new PointsUpdateCallback(this);
    setUpdateCallback(_updateCallback);

    _point = new osg::Point();
    _point->setSize(_pointSize);

    _pointSprite = new osg::PointSprite();
    _blendFunc = new osg::BlendFunc();

    //makeTexture();
    //getOrCreateStateSet()->setTextureAttributeAndModes(0, _sphereTexture, osg::StateAttribute::ON);

    setPointsMode(_mode);
}

void PointsNode::update()
{
    osg::Matrix m = getLocalToWorldMatrix(this);
    osg::Vec3 scale = m.getScale();
    float currentScale;
    _scaleUni->get(currentScale);

    if(currentScale != scale.x())
    {
        _scaleUni->set(scale.x());
    }
}

void PointsNode::calcSize()
{
    size_t newSize = _vertArray->size();
    if(_colorBinding == PointsNode::POINTS_PER_POINT)
    {
        newSize = std::min(newSize,_colorArray->size());
    }
    if(_sizeBinding == PointsNode::POINTS_PER_POINT)
    {
        newSize = std::min(newSize,_sizeArray->size());
    }

    _primitive->setCount(newSize);
    _size = newSize;
}

void PointsNode::refreshGeometry()
{
    _geometry->setVertexArray(_vertArray);
    _geometry->setColorArray(_colorArray);

    switch(_mode)
    {
        case POINTS_GL_POINTS:
        {
            _geometry->setVertexAttribArray(4,_sizeArray);
            break;
        }
        case POINTS_SHADED_SPHERES:
        {
            _geometry->setVertexAttribArray(4,_radiusArray);
            break;
        }
        case POINTS_POINT_SPRITES:
        {
            if(_sizeArray->size())
            {
                _point->setSize(_sizeArray->at(0));
            }
        }
            ;
        default:
            break;
    }

    if(_colorBinding == POINTS_OVERALL && !_colorArray->size())
    {
        _colorArray->push_back(_pointColor);
    }
    if(_radiusBinding == POINTS_OVERALL && !_radiusArray->size())
    {
        _radiusArray->push_back(_pointRadius);
    }
    if(_sizeBinding == POINTS_OVERALL && !_sizeArray->size())
    {
        _sizeArray->push_back(_pointSize);
    }
}

/*void PointsNode::makeTexture()
 {
 _textureCreationLock.lock();
 if(_sphereTexture)
 {
 _textureCreationLock.unlock();
 return;
 }

 osg::Image * image = new osg::Image();
 int size = 1024;
 image->allocateImage(size,size,1,GL_RGB,GL_BYTE);
 image->setInternalTextureFormat(3);

 char * textureData = (char *)image->data();
 int tindex = 0;

 for(int i = 0; i < size; i++)
 {
 for(int j = 0; j < size; j++)
 {
 char x,y,z;
 float xf, yf, zf;
 xf = ((float)j) /((float)(size-1));
 xf *= 2.0;
 xf -= 1.0;

 yf = ((float)i) /((float)(size-1));
 yf *= 2.0;
 yf -= 1.0;
 //yf *= -1.0;

 zf = 1.0 - yf*yf - xf*xf;
 if(zf < 0.0)
 {
 z = -128;
 }
 else
 {
 zf = sqrt(zf);
 z = (char)(zf * 128.0);
 }
 x = (char)(xf * 128.0);
 y = (char)(yf * 128.0);

 textureData[tindex] = x;
 textureData[tindex+1] = y;
 textureData[tindex+2] = z;
 tindex += 3;
 }
 }

 _sphereTexture = new osg::Texture2D(image);

 _textureCreationLock.unlock();
 }*/
