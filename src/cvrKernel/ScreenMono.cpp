#include <cvrKernel/ScreenMono.h>
#include <cvrKernel/CVRViewer.h>

#include <iostream>

using namespace cvr;

ScreenMono::ScreenMono() :
        ScreenBase()
{
}

ScreenMono::~ScreenMono()
{
}

void ScreenMono::init(int mode)
{
    _type = (monoType)mode;

    _camera = new osg::Camera();

    osg::DisplaySettings * ds = new osg::DisplaySettings();
    _camera->setDisplaySettings(ds);

    CVRViewer::instance()->addSlave(_camera.get(),osg::Matrixd(),
            osg::Matrixd());
    defaultCameraInit(_camera.get());
}

void ScreenMono::computeViewProj()
{
    //get eye position
    osg::Vec3d eyePos;

    switch(_type)
    {
        case CENTER:
            eyePos = eyePos * getCurrentHeadMatrix(_myInfo->myChannel->head);
            break;
        case LEFT:
            eyePos = defaultLeftEye(_myInfo->myChannel->head);
            break;
        case RIGHT:
            eyePos = defaultRightEye(_myInfo->myChannel->head);
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

void ScreenMono::viewportResized(int left, int bottom, int width, int height)
{
    if(_camera)
    {
	osg::Viewport * viewport = _camera->getViewport();
	if(viewport)
	{
	    viewport->x() = left;
	    viewport->y() = bottom;
	    viewport->width() = width;
	    viewport->height() = height;
	}
    }
    ScreenBase::viewportResized(left,bottom,width,height);
}
