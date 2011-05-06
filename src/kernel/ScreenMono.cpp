#include <kernel/ScreenMono.h>
#include <kernel/CVRViewer.h>

#include <iostream>

using namespace cvr;

ScreenMono::ScreenMono() : ScreenBase()
{
}

ScreenMono::~ScreenMono()
{
}

void ScreenMono::init(int mode)
{
    _type = (monoType)mode;

    _camera = new osg::Camera();

    CVRViewer::instance()->addSlave(_camera.get(), osg::Matrixd(), osg::Matrixd());
    defaultCameraInit(_camera.get());
}

void ScreenMono::computeViewProj()
{
    //get eye position
    osg::Vec3d eyePos;

    switch(_type)
    {
        case CENTER:
	    eyePos = eyePos * getCurrentHeadMatrix();
            break;
        case LEFT:
            eyePos = defaultLeftEye();
            break;
        case RIGHT:
            eyePos = defaultRightEye();
            break;
        default:
            break;
    }

    computeDefaultViewProj(eyePos,_view,_proj);
}

void ScreenMono::updateCamera()
{
    _camera->setViewMatrix(_view);
    _camera->setProjectionMatrix(_proj);
}

void ScreenMono::setClearColor(osg::Vec4 color)
{
    _camera->setClearColor(color);
}

ScreenInfo * ScreenMono::findScreenInfo(osg::Camera * c)
{
    if(c == _camera.get())
    {
        return _myInfo;
    }
    return NULL;
}
