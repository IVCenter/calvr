#include <config/ConfigManager.h>
#include <kernel/ScreenInterlacedTopBottom.h>
#include <kernel/CVRViewer.h>
#include <kernel/CalVR.h>

#include <osgViewer/Renderer>
#include <osg/Shader>
#include <osg/GL2Extensions>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <iostream>
#include <string>

#include <GL/gl.h>

using namespace cvr;

OpenThreads::Mutex ScreenInterlacedTopBottom::InterlaceCallback::_initLock;

ScreenInterlacedTopBottom::ScreenInterlacedTopBottom() : ScreenBase()
{
}

ScreenInterlacedTopBottom::~ScreenInterlacedTopBottom()
{
}

void ScreenInterlacedTopBottom::init(int mode)
{
    _stereoMode = osg::DisplaySettings::VERTICAL_SPLIT;
    //_stereoMode = osg::DisplaySettings::LEFT_EYE;

    _cameraL = new osg::Camera();
    _cameraR = new osg::Camera();

    osg::DisplaySettings * ds = new osg::DisplaySettings();
    _cameraL->setDisplaySettings(ds);
    ds = new osg::DisplaySettings();
    _cameraR->setDisplaySettings(ds);

    CVRViewer::instance()->addSlave(_cameraL.get(), osg::Matrixd(), osg::Matrixd());
    defaultCameraInit(_cameraL.get());
    CVRViewer::instance()->addSlave(_cameraR.get(), osg::Matrixd(), osg::Matrixd());
    defaultCameraInit(_cameraR.get());

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*> (_cameraL->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {
        osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
        ds->setStereo(true);
        ds->setStereoMode(osg::DisplaySettings::LEFT_EYE);
        StereoCallback * sc = new StereoCallback;
        sc->screen = this;
        renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
        renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);

	osg::Viewport * vp = _cameraL->getViewport();
	vp->height() = vp->height() / 2.0;
	_cameraL->setViewport(vp);
    }

    renderer =
            dynamic_cast<osgViewer::Renderer*> (_cameraR->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {
        osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
        ds->setStereo(true);
        ds->setStereoMode(osg::DisplaySettings::RIGHT_EYE);
        StereoCallback * sc = new StereoCallback;
        sc->screen = this;
        renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
        renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);

	osg::Viewport * vp = _cameraR->getViewport();
	vp->height() = vp->height() / 2.0;
	_cameraR->setViewport(vp);
    }

    int samples = ConfigManager::getInt("MultiSample",0);

    InterlaceCallback * ic = new InterlaceCallback;
    ic->screen = this;
    ic->odd = true;
    ic->first = true;
    //ic->skip = true;
    _cameraL->setPostDrawCallback(ic);
    //_camera->setDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    //_camera->setRenderOrder(osg::Camera::PRE_RENDER);


    /*osg::Image * image = new osg::Image();
    image->allocateImage(1024,768,GL_RGBA,GL_RGBA,GL_FLOAT);
    image->setInternalTextureFormat(4);

    float * textureData = (float *)image->data();

    int index = 0;
    for(int i = 0; i < 1024 * 384; i++)
    {
	textureData[index] = 0.0;
	textureData[index+1] = 1.0;
	textureData[index+2] = 0.0;
	textureData[index+3] = 1.0;
	index += 4;
    }
     
    for(int i = 0; i < 1024 * 384; i++)
    {
	textureData[index] = 0.0;
	textureData[index+1] = 0.0;
	textureData[index+2] = 1.0;
	textureData[index+3] = 1.0;
	index += 4;
    }

    _colorTexture = new osg::Texture2D(image);
    _colorTexture->setResizeNonPowerOfTwoHint(false);
    _colorTexture->setUseHardwareMipMapGeneration(false);
    _colorTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
    _colorTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);*/

    _colorTextureL = new osg::Texture2D();
    _colorTextureL->setTextureSize((int)_myInfo->myChannel->width,(int)(_myInfo->myChannel->height / 2.0));
    _colorTextureL->setInternalFormat(GL_RGBA);
    _colorTextureL->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _colorTextureL->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    _colorTextureL->setResizeNonPowerOfTwoHint(false);
    _colorTextureL->setUseHardwareMipMapGeneration(false);

    /*_depthTexture = new osg::Texture2D();
    _depthTexture->setTextureSize((int)_myInfo->myChannel->width,(int)_myInfo->myChannel->height);
    _depthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
    _depthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _depthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    _depthTexture->setResizeNonPowerOfTwoHint(false);
    _depthTexture->setUseHardwareMipMapGeneration(false);*/


    //image = new osg::Image();
    //image->allocateImage(1024,768,GL_RGBA,GL_RGBA,GL_FLOAT);
    //image->setInternalTextureFormat(4);
    _cameraL->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    //_camera->attach(osg::Camera::COLOR_BUFFER0, image);
    _cameraL->attach(osg::Camera::COLOR_BUFFER0, _colorTextureL, 0, 0, false, samples, samples);
    //_cameraL->attach(osg::Camera::COLOR_BUFFER0, _colorTextureL);
    //_camera->attach(osg::Camera::DEPTH_BUFFER, _depthTexture);
    //_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //_colorTexture->setImage(image);
    ic->_texture = _colorTextureL.get();

    ic = new InterlaceCallback;
    ic->screen = this;
    ic->odd = false;
    ic->first = false;
    _cameraR->setPostDrawCallback(ic);

    _colorTextureR = new osg::Texture2D();
    _colorTextureR->setTextureSize((int)_myInfo->myChannel->width,(int)(_myInfo->myChannel->height / 2.0));
    _colorTextureR->setInternalFormat(GL_RGBA);
    _colorTextureR->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _colorTextureR->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    _colorTextureR->setResizeNonPowerOfTwoHint(false);
    _colorTextureR->setUseHardwareMipMapGeneration(false);

    _cameraR->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    //_camera->attach(osg::Camera::COLOR_BUFFER0, image);
    _cameraR->attach(osg::Camera::COLOR_BUFFER0, _colorTextureR, 0, 0, false, samples, samples);
    //_cameraR->attach(osg::Camera::COLOR_BUFFER0, _colorTextureR);
    //_camera->attach(osg::Camera::DEPTH_BUFFER, _depthTexture);
    //_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //_colorTexture->setImage(image);
    ic->_texture = _colorTextureR.get();
}

void ScreenInterlacedTopBottom::computeViewProj()
{
    osg::Vec3d eyeLeft = defaultLeftEye(_myInfo->myChannel->head);
    osg::Vec3d eyeRight = defaultRightEye(_myInfo->myChannel->head);

    computeDefaultViewProj(eyeLeft,_viewLeft,_projLeft);
    computeDefaultViewProj(eyeRight,_viewRight,_projRight);
}

void ScreenInterlacedTopBottom::updateCamera()
{
    _cameraL->setViewMatrix(_viewLeft);
    _cameraL->setProjectionMatrix(_projLeft);
    _cameraR->setViewMatrix(_viewRight);
    _cameraR->setProjectionMatrix(_projRight);
    // not needed for this mode
    //std::cerr << "Update." << std::endl;
}

osg::Matrixd ScreenInterlacedTopBottom::StereoCallback::computeLeftEyeProjection(
                                                                    const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projLeft;
}

osg::Matrixd ScreenInterlacedTopBottom::StereoCallback::computeLeftEyeView(
                                                              const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewLeft;
}

osg::Matrixd ScreenInterlacedTopBottom::StereoCallback::computeRightEyeProjection(
                                                                     const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projRight;
}

osg::Matrixd ScreenInterlacedTopBottom::StereoCallback::computeRightEyeView(
                                                               const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewRight;
}

void ScreenInterlacedTopBottom::setClearColor(osg::Vec4 color)
{
    _cameraL->setClearColor(color);
    _cameraR->setClearColor(color);
}

ScreenInfo * ScreenInterlacedTopBottom::findScreenInfo(osg::Camera * c)
{
    if(c == _cameraL.get() || c == _cameraR.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenInterlacedTopBottom::adjustViewportCoords(int & x, int & y)
{
    return;
    if(_stereoMode == osg::DisplaySettings::HORIZONTAL_SPLIT)
    {
	if(x > (_myInfo->myChannel->width / 2.0))
	{
	    x = (int)( ((float)x) - (_myInfo->myChannel->width / 2.0) );
	}
	x *= 2;
    }
    else if(_stereoMode == osg::DisplaySettings::VERTICAL_SPLIT)
    {
	if(y > (_myInfo->myChannel->height / 2.0))
	{
	    y = (int)(((float)y) - (_myInfo->myChannel->height / 2.0));
	}
	y *= 2;
    }

    return;
}

void ScreenInterlacedTopBottom::InterlaceCallback::operator() (osg::RenderInfo &renderInfo) const
{
    //osgDB::writeImageFile(*screen->image,"/home/aprudhom/testImage.tif");
    //exit(0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glFinish();
    //return;
    /*if(skip)
    {
	//screen->_camera->setClearColor(osg::Vec4(1.0,0,0,0));
	skip = false;
	return;
    }
    else
    {
	//screen->_camera->setClearColor(osg::Vec4(0,1.0,0,0));
	skip = true;
    }*/

    if(first)
    {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    //glPushAttrib(GL_ALL_ATTRIB_BITS);

    //std::cerr << "Callback." << std::endl;
    int context = renderInfo.getContextID();
    if(!_initMap[context])
    {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_initLock);
	std::string shaderdir = CalVR::instance()->getHomeDir() + "/shaders/";

	osg::Shader * vert, * frag;
	if(odd)
	{
	    vert = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shaderdir + "interlace.vert"));
	    frag = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shaderdir + "interlace-odd.frag"));	
	}
	else
	{
	    vert = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shaderdir + "interlace.vert"));
	    frag = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shaderdir + "interlace-even.frag"));
	}

	_programMap[context] = new osg::Program;
	_programMap[context]->addShader(vert);
	_programMap[context]->addShader(frag);

	_geometryMap[context] = new osg::Geometry();

	osg::DrawArrays * quad = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,0,0);
	osg::Vec2Array * verts = new osg::Vec2Array(0);
	_geometryMap[context]->setVertexArray(verts);
	_geometryMap[context]->addPrimitiveSet(quad);
	verts->push_back(osg::Vec2(-1.0,1.0));
	verts->push_back(osg::Vec2(-1.0,-1.0));
	verts->push_back(osg::Vec2(1.0,1.0));
	verts->push_back(osg::Vec2(1.0,-1.0));

	_geometryMap[context]->setUseDisplayList(false);

	quad->setCount(4);
	
	/*glGenBuffers(1,&_arrayMap[context]);
	  glBindBuffer(GL_ARRAY_BUFFER, arrayMap[context]);
	  float points[8] = {-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0};

	  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);
	  glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	
	_initMap[context] = true;
    }

    glViewport((int)screen->_myInfo->myChannel->left,(int)screen->_myInfo->myChannel->bottom,(int)screen->_myInfo->myChannel->width,(int)screen->_myInfo->myChannel->height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    _texture->apply(*renderInfo.getState());

    _programMap[context]->apply(*renderInfo.getState());
    _geometryMap[context]->drawImplementation(renderInfo);

    const osg::GL2Extensions* extensions = osg::GL2Extensions::Get(context,true);
    extensions->glUseProgram(0);
    renderInfo.getState()->setLastAppliedProgramObject(0);

    /*glBegin(GL_TRIANGLE_STRIP);

    glColor3f(1.0,0.0,0.0);

    glVertex2f(-1.0,1.0);
    glVertex2f(-1.0,-1.0);
    glVertex2f(1.0,1.0);
    glVertex2f(1.0,-1.0);

    glEnd();*/

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    //glPopAttrib();
}
