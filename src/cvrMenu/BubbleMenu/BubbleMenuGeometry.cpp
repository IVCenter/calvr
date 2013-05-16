#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuButtonGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuCheckboxGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuListGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuRangeValueGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuRangeValueCompactGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuTextGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuTextButtonSetGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuImageGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuClosableGeometry.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuList.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrMenu/SubMenu.h>
#include <cvrMenu/BubbleMenu/Lerp.h>

#include <osgText/Text>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/PolygonMode>
#include <osg/LineWidth>

#include <iostream>
#include <cstdlib>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

osg::Vec4 BubbleMenuGeometry::_textColor;
osg::Vec4 BubbleMenuGeometry::_wireframeColor;
osg::Vec4 BubbleMenuGeometry::_textColorSelected;
osg::Vec4 BubbleMenuGeometry::_backgroundColor;
float BubbleMenuGeometry::_border;
float BubbleMenuGeometry::_iconHeight;
float BubbleMenuGeometry::_textSize;
float BubbleMenuGeometry::_radius;
int BubbleMenuGeometry::_tessellations;
osg::Vec4 BubbleMenuGeometry::_sphereColor;
std::string BubbleMenuGeometry::_iconDir;
osg::ref_ptr<osgText::Font> BubbleMenuGeometry::_font;
std::map<std::string,osg::ref_ptr<osg::Texture2D> > BubbleMenuGeometry::_iconCache;

BubbleMenuGeometry * cvr::createBubbleMenuGeometry(MenuItem * item, bool head)
{
    switch(item->getType())
    {
        case BUTTON:
        {
            BubbleMenuGeometry * mg = new BubbleMenuButtonGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case CHECKBOX:
        {
            BubbleMenuGeometry * mg = new BubbleMenuCheckboxGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case SUBMENU:
        {
            BubbleMenuGeometry * mg = new BubbleMenuSubMenuGeometry(head);
            mg->createGeometry(item);

            return mg;
            break;
        }
        case SUBMENU_CLOSABLE:
        {
            BubbleMenuGeometry * mg = new BubbleMenuSubMenuClosableGeometry(head);
            mg->createGeometry(item);

            return mg;
            break;
        }
        case RANGEVALUE:
        {
            BubbleMenuGeometry * mg = new BubbleMenuRangeValueGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case RANGEVALUECOMPACT:
        {
            BubbleMenuGeometry * mg = new BubbleMenuRangeValueCompactGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case TEXT:
        {
            BubbleMenuGeometry * mg = new BubbleMenuTextGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case TEXTBUTTONSET:
        {
            BubbleMenuGeometry * mg = new BubbleMenuTextButtonSetGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case IMAGE:
        {
            BubbleMenuGeometry * mg = new BubbleMenuImageGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case LIST:
        {
            BubbleMenuGeometry * mg = new BubbleMenuListGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case OTHER:
        default:
            break;
    }

    return NULL;
}

osg::Geometry * BubbleMenuGeometry::makeQuad(float width, float height,
        osg::Vec4 color, osg::Vec3 pos)
{
    osg::Geometry * geo = new osg::Geometry();
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(pos);
    verts->push_back(pos + osg::Vec3(width,0,0));
    verts->push_back(pos + osg::Vec3(width,0,height));
    verts->push_back(pos + osg::Vec3(0,0,height));

    geo->setVertexArray(verts);

    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
            osg::PrimitiveSet::QUADS,0);

    ele->push_back(0);
    ele->push_back(1);
    ele->push_back(2);
    ele->push_back(3);
    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);

    osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4> *colorIndexArray;
    colorIndexArray = new osg::TemplateIndexArray<unsigned int,
            osg::Array::UIntArrayType,4,4>;
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);

    geo->setColorArray(colors);
    geo->setColorIndices(colorIndexArray);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0,0));
    texcoords->push_back(osg::Vec2(1,0));
    texcoords->push_back(osg::Vec2(1,1));
    texcoords->push_back(osg::Vec2(0,1));
    geo->setTexCoordArray(0,texcoords);

    return geo;
}

osg::Geometry * BubbleMenuGeometry::makeLine(osg::Vec3 p1, osg::Vec3 p2,
        osg::Vec4 color)
{
    osg::Geometry * geo = new osg::Geometry();
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(p1);
    verts->push_back(p2);

    geo->setVertexArray(verts);

    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
            osg::PrimitiveSet::LINES,0);

    ele->push_back(0);
    ele->push_back(1);
    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);

    osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4> *colorIndexArray;
    colorIndexArray = new osg::TemplateIndexArray<unsigned int,
            osg::Array::UIntArrayType,4,4>;
    colorIndexArray->push_back(0);
    colorIndexArray->push_back(0);

    geo->setColorArray(colors);
    geo->setColorIndices(colorIndexArray);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    return geo;
}

osg::Geometry * BubbleMenuGeometry::makeSphere(osg::Vec3 point, float radius, 
    osg::Vec4 color)
{
    osg::Vec3Array * _verts      = new osg::Vec3Array(0);
    osg::Vec4Array * _colors     = new osg::Vec4Array(1);
    osg::Vec3Array * _normals    = new osg::Vec3Array(0);
    osg::DrawArrays * _primitive = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 0);
    osg::Geometry * _geometry    = new osg::Geometry();
   
//    (*_colors)[0] = color;
    (*_colors)[0] = _sphereColor;
    int _count = 0;

    float theta, cost, sint, thetaNext, costn, sintn, 
          gamma, sing, cosg, gammaNext, singn, cosgn, interval,
          x = point[0], y = point[1], z = point[2];

    _tessellations += _tessellations % 2; // force to be even for nice tessellating

    interval = (M_PI * 2) / (float)_tessellations;

    _verts->clear();
    _normals->clear();

    for (int i = 0; i <  _tessellations+1; ++i)
    {
        for (int j = 0; j <  _tessellations+1; ++j)
        {
            theta = i * interval;
            gamma = j * interval;

            cost = cos(theta);
            sint = sin(theta);

            cosg = cos(gamma);
            sing = sin(gamma);

            thetaNext = (i + 1) * interval;
            gammaNext = (j + 1) * interval;

            costn = cos(thetaNext);
            sintn = sin(thetaNext);

            cosgn = cos(gammaNext);
            singn = sin(gammaNext);

            osg::Vec3 topLeft     = osg::Vec3(x + radius * sint * cosg, 
                                              y + radius * sint * sing,
                                              z + radius * cost);
            osg::Vec3 topRight    = osg::Vec3(x + radius * sint * cosgn, 
                                              y + radius * sint * singn,
                                              z + radius * cost);
            osg::Vec3 bottomRight = osg::Vec3(x + radius * sintn * cosgn, 
                                              y + radius * sintn * singn,
                                              z + radius * costn);
            osg::Vec3 bottomLeft  = osg::Vec3(x + radius * sintn * cosg, 
                                              y + radius * sintn * sing,
                                              z + radius * costn);

            _verts->push_back(topLeft);
            _normals->push_back(topLeft);

            _verts->push_back(bottomLeft);
            _normals->push_back(bottomLeft);

            _verts->push_back(bottomRight);
            _normals->push_back(bottomRight);

            _verts->push_back(topRight);
            _normals->push_back(topRight);
        }
    }

    _count = 4 * _tessellations * _tessellations;

    _geometry->setVertexArray(_verts);
    _geometry->setColorArray(_colors);
    _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    _geometry->setNormalArray(_normals);
    _geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    _geometry->setUseDisplayList(false);
    _geometry->addPrimitiveSet(_primitive);

    _primitive->setCount(_count);
    return _geometry;
}

osg::Texture2D * BubbleMenuGeometry::loadIcon(std::string name)
{
    if(_iconCache.find(name) != _iconCache.end())
    {
        if(_iconCache[name])
        {
            return _iconCache[name].get();
        }
        else
        {
            return NULL;
        }
    }

    std::string file = _iconDir + "/icons/" + name;
    //std::cerr << "Trying to load icon: " << file << std::endl;
    osg::ref_ptr < osg::Image > image = osgDB::readImageFile(file);
    if(image)
    {
        osg::Texture2D* texture;
        texture = new osg::Texture2D;
        texture->setImage(image);

        // do not cache very large textures
        if(image->s() <= 512 && image->t() <= 512)
        {
            _iconCache[name] = texture;
        }

        return texture;
    }
    else
    {
        _iconCache[name] = NULL;
    }
    std::cerr << "Icon: " << file << " not found." << std::endl;
    return NULL;
}

osgText::Text * BubbleMenuGeometry::makeText(std::string text, float size,
        osg::Vec3 pos, osg::Vec4 color, osgText::Text::AlignmentType align)
{
    osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(size);
    textNode->setAlignment(align);
    textNode->setPosition(pos);
    textNode->setColor(color);
    textNode->setBackdropColor(osg::Vec4(0,0,0,0));
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    textNode->setMaximumWidth(200);
    if(_font.valid())
    {
        textNode->setFont(_font);
    }
    textNode->setText(text);
    return textNode;
}

osgText::Text3D * BubbleMenuGeometry::make3DText(std::string text, float size,
        osg::Vec3 pos, osg::Vec4 color, osgText::Text::AlignmentType align)
{
    osgText::Text3D * textNode = new osgText::Text3D();
    textNode->setCharacterSize(size);

    textNode->setCharacterDepth(15);
    textNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
    textNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    textNode->setDrawMode(osgText::Text3D::TEXT);
    textNode->setAlignment(align);
    textNode->setPosition(pos);
#if OPENSCENEGRAPH_MAJOR_VERSION >= 3
    textNode->setColor(color);
#endif
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    textNode->setMaximumWidth(6);

    if(_font.valid())
    {
        textNode->setFont(_font);
    }
    textNode->setText(text);
    return textNode;

}

void BubbleMenuGeometry::calibrateTextSize(float textSize)
{
    osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(textSize);
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    if(_font.valid())
    {
        textNode->setFont(_font);
    }
    textNode->setText("A");

    osg::BoundingBox bb;
    bb = textNode->getBound();

    float height = bb.zMax() - bb.zMin();

    float ratio = 24.375 / height;

    //_textSize = textSize * ratio;

    // hack for testing font sizes
    //_textSize = 35;

    textNode->unref();
}

BubbleMenuGeometry::BubbleMenuGeometry()
{
    _width = 0;
    _height = 0;
    _intersect = NULL;
    _item = NULL;
}

BubbleMenuGeometry::~BubbleMenuGeometry()
{
}

MenuItem * BubbleMenuGeometry::getMenuItem()
{
    return _item;
}

void BubbleMenuGeometry::resetIntersect(float width)
{
    _intersect->removeDrawables(0,_intersect->getNumDrawables());
/*    _intersect->addDrawable(
            makeQuad(width + 2.0 * _border,-(_height + _border),
                    osg::Vec4(0,.2,0,0),osg::Vec3(-_border,0,_border / 2.0)));*/
    _intersect->addDrawable(makeSphere(osg::Vec3(_radius/2,0,0), _radius, osg::Vec4(0,0,0,0)));
    _intersect->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

osg::Geode * BubbleMenuGeometry::getIntersect()
{
    return _intersect;
}

float BubbleMenuGeometry::getWidth()
{
    return _width;
}

float BubbleMenuGeometry::getHeight()
{
    return _height;
}

osg::MatrixTransform * BubbleMenuGeometry::getNode()
{
    return _node.get();
}

