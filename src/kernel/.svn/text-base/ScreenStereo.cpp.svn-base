#include <kernel/ScreenStereo.h>
#include <kernel/CVRViewer.h>

#include <osgViewer/Renderer>

#include <iostream>

using namespace cvr;

ScreenStereo::ScreenStereo() : ScreenBase()
{
}

ScreenStereo::~ScreenStereo()
{
}

void ScreenStereo::init(int mode)
{
    _stereoMode = (osg::DisplaySettings::StereoMode)mode;

    _camera = new osg::Camera();

    CVRViewer::instance()->addSlave(_camera.get(), osg::Matrixd(), osg::Matrixd());
    defaultCameraInit(_camera.get());

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*> (_camera->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {
        osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
        ds->setStereo(true);
        ds->setStereoMode(_stereoMode);
        StereoCallback * sc = new StereoCallback;
        sc->screen = this;
        renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
        renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);
    }
}

void ScreenStereo::computeViewProj()
{
    osg::Vec3d eyeLeft = defaultLeftEye();
    osg::Vec3d eyeRight = defaultRightEye();

    computeDefaultViewProj(eyeLeft,_viewLeft,_projLeft);
    computeDefaultViewProj(eyeRight,_viewRight,_projRight);
}

void ScreenStereo::updateCamera()
{
    // not needed for this mode
}

osg::Matrixd ScreenStereo::StereoCallback::computeLeftEyeProjection(
                                                                    const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projLeft;
}

osg::Matrixd ScreenStereo::StereoCallback::computeLeftEyeView(
                                                              const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewLeft;
}

osg::Matrixd ScreenStereo::StereoCallback::computeRightEyeProjection(
                                                                     const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projRight;
}

osg::Matrixd ScreenStereo::StereoCallback::computeRightEyeView(
                                                               const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewRight;
}

void ScreenStereo::setClearColor(osg::Vec4 color)
{
    _camera->setClearColor(color);
}

ScreenInfo * ScreenStereo::findScreenInfo(osg::Camera * c)
{
    if(c == _camera.get())
    {
        return _myInfo;
    }
    return NULL;
}
