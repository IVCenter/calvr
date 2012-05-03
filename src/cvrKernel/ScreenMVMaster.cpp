#include <cvrKernel/CVRCullVisitor.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/ScreenMVMaster.h>

#include <iostream>

using namespace cvr;

ScreenMVMaster::ScreenMVMaster() :
        ScreenMVSimulator()
{
}

ScreenMVMaster::~ScreenMVMaster()
{
}

void ScreenMVMaster::init(int mode)
{
    _cameraScene = new osg::Camera();
    CVRViewer::instance()->addSlave(_cameraScene.get(),osg::Matrixd(),
            osg::Matrixd());
    defaultCameraInit(_cameraScene.get());

    _cameraDiagram = new osg::Camera();
    CVRViewer::instance()->addSlave(_cameraDiagram.get(),osg::Matrixd(),
            osg::Matrixd());
    defaultCameraInit(_cameraDiagram.get());

    setupDiagramCam();
}

void ScreenMVMaster::computeViewProj()
{
    //get eye position
    osg::Vec3d eyePos;

    eyePos = eyePos * getCurrentHeadMatrix();

    computeDefaultViewProj(eyePos,_view,_proj);
}

void ScreenMVMaster::updateCamera()
{
    _cameraScene->setViewMatrix(_view);
    _cameraScene->setProjectionMatrix(_proj);
}

void ScreenMVMaster::setClearColor(osg::Vec4 color)
{
    _cameraScene->setClearColor(color);
    _cameraDiagram->setClearColor(color);
}

ScreenInfo * ScreenMVMaster::findScreenInfo(osg::Camera * c)
{
    if(c == _cameraScene.get())
    {
        return _myInfo;
    }
    else if(c == _cameraDiagram.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenMVMaster::adjustViewportCoords(int &x, int &y)
{
    if(_cameraDiagram->getNodeMask() != 0)
    {
        float w_2 = _myInfo->myChannel->width / 2;
        float h_2 = _myInfo->myChannel->height / 2;
        x = (int)((x - w_2) * viewProjRatio + w_2);
        y = (int)((y - h_2) * viewProjRatio + h_2);
    }
}

void ScreenMVMaster::setupDiagramCam()
{

    // setup camera to show objects in the x,y plane (z = 0)
    _cameraDiagram->setViewport(0,0,(int)_myInfo->myChannel->width,
            (int)_myInfo->myChannel->height);

    float width = _myInfo->width;
    float height = _myInfo->height;
    const int CAVE_RAD = 1468;

    if(width > height)
    {
        width = 2 * CAVE_RAD * width / height;
        height = 2 * CAVE_RAD;
    }
    else
    {
        width = 2 * CAVE_RAD;
        height = 2 * CAVE_RAD * height / width;
    }

    _cameraDiagram->setViewMatrixAsLookAt(osg::Vec3(0,-100,0),osg::Vec3(0,0,0),
            osg::Vec3(0,0,1));

    _cameraDiagram->setProjectionMatrixAsOrtho(-width / 2,width / 2,-height / 2,
            height / 2,1,10000);

    viewProjRatio = width / _myInfo->myChannel->width;
}

void ScreenMVMaster::showDiagram(bool show)
{
    static unsigned int sceneMask = _cameraScene->getNodeMask();
    static unsigned int diagramMask = _cameraDiagram->getNodeMask();

    if(show)
    {
        _cameraScene->setNodeMask(0);
        _cameraDiagram->setNodeMask(diagramMask);
    }
    else
    {
        _cameraScene->setNodeMask(sceneMask);
        _cameraDiagram->setNodeMask(0);
    }
}
