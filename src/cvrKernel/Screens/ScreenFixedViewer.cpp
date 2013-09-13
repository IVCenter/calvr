#include <cvrKernel/Screens/ScreenFixedViewer.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/CVRViewer.h>

#include <iostream>
#include <sstream>

using namespace cvr;

ScreenFixedViewer::ScreenFixedViewer() :
        ScreenBase()
{
}

ScreenFixedViewer::~ScreenFixedViewer()
{
}

void ScreenFixedViewer::init(int mode)
{
    _camera = new osg::Camera();

    osg::DisplaySettings * ds = new osg::DisplaySettings();
    _camera->setDisplaySettings(ds);

    CVRViewer::instance()->addSlave(_camera.get(),osg::Matrixd(),
            osg::Matrixd());
    defaultCameraInit(_camera.get());

    std::stringstream ss;
    ss << "ChannelConfig.Channel:" << _myInfo->channelIndex
            << ".ViewerPosition";

    float x, y, z;

    x = ConfigManager::getFloat("x",ss.str(),0);
    y = ConfigManager::getFloat("y",ss.str(),0);
    z = ConfigManager::getFloat("z",ss.str(),0);

    osg::Vec3 pos(x,y,z);

    _viewerMat.makeTranslate(pos);
}

void ScreenFixedViewer::computeViewProj()
{
    //get eye position
    osg::Vec3d eyePos;

    eyePos = eyePos * _viewerMat;

    computeDefaultViewProj(eyePos,_view,_proj);
}

void ScreenFixedViewer::updateCamera()
{
    _camera->setViewMatrix(_view);
    _camera->setProjectionMatrix(_proj);
}

void ScreenFixedViewer::setClearColor(osg::Vec4 color)
{
    _camera->setClearColor(color);
}

ScreenInfo * ScreenFixedViewer::findScreenInfo(osg::Camera * c)
{
    if(c == _camera.get())
    {
        return _myInfo;
    }
    return NULL;
}
