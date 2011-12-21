#include <kernel/ScreenHMD.h>
#include <kernel/CVRViewer.h>
#include <input/TrackingManager.h>

#include <osgViewer/Renderer>

#include <iostream>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

ScreenHMD::ScreenHMD() :
        ScreenBase()
{
}

ScreenHMD::~ScreenHMD()
{
}

void ScreenHMD::init(int mode)
{
    if(mode >= 0)
    {
        _stereo = true;
        _stereoMode = (osg::DisplaySettings::StereoMode)mode;
    }
    else
    {
        _stereo = false;
    }

    _camera = new osg::Camera();

    osg::DisplaySettings * ds = new osg::DisplaySettings();
    _camera->setDisplaySettings(ds);

    CVRViewer::instance()->addSlave(_camera.get(),osg::Matrixd(),
            osg::Matrixd());
    defaultCameraInit(_camera.get());

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*>(_camera->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {
        osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
        ds->setStereo(_stereo);

        if(_stereo)
        {
            ds->setStereoMode(_stereoMode);
            StereoCallback * sc = new StereoCallback;
            sc->screen = this;
            renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
            renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);
        }
    }
}

void ScreenHMD::computeViewProj()
{
    osg::Vec3d eyeLeft;
    osg::Vec3d eyeRight;

    if(_stereo)
    {
        eyeLeft = defaultLeftEye(_myInfo->myChannel->head);
        eyeRight = defaultRightEye(_myInfo->myChannel->head);
    }
    else
    {
        eyeLeft = TrackingManager::instance()->getHeadMat(
                _myInfo->myChannel->head).getTrans();
    }

    osg::Quat invViewerRot = TrackingManager::instance()->getHeadMat(
            _myInfo->myChannel->head).getRotate();
    invViewerRot = invViewerRot.inverse();

    //translate screen to origin
    osg::Matrix screenTrans;
    screenTrans.makeTranslate(
            -(_myInfo->xyz
                    * TrackingManager::instance()->getHeadMat(
                            _myInfo->myChannel->head)));

    //rotate screen to xz
    osg::Matrix screenRot;
    screenRot.makeRotate(-_myInfo->h * M_PI / 180.0,osg::Vec3(0,0,1),
            -_myInfo->p * M_PI / 180.0,osg::Vec3(1,0,0),
            -_myInfo->r * M_PI / 180.0,osg::Vec3(0,1,0));

    eyeLeft = eyeLeft * screenTrans * osg::Matrix::rotate(invViewerRot)
            * screenRot;

    //make frustum
    float top, bottom, left, right;
    float screenDist = -eyeLeft.y();

    top = _near * (_myInfo->height / 2.0 - eyeLeft.z()) / screenDist;
    bottom = _near * (-_myInfo->height / 2.0 - eyeLeft.z()) / screenDist;
    right = _near * (_myInfo->width / 2.0 - eyeLeft.x()) / screenDist;
    left = _near * (-_myInfo->width / 2.0 - eyeLeft.x()) / screenDist;

    _projLeft.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyeLeft);

    //make view
    _viewLeft = screenTrans * osg::Matrix::rotate(invViewerRot) * screenRot
            * cameraTrans
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));

    if(_stereo)
    {
        eyeRight = eyeRight * screenTrans * osg::Matrix::rotate(invViewerRot)
                * screenRot;

        //make frustum
        top, bottom, left, right;
        screenDist = -eyeRight.y();

        top = _near * (_myInfo->height / 2.0 - eyeRight.z()) / screenDist;
        bottom = _near * (-_myInfo->height / 2.0 - eyeRight.z()) / screenDist;
        right = _near * (_myInfo->width / 2.0 - eyeRight.x()) / screenDist;
        left = _near * (-_myInfo->width / 2.0 - eyeRight.x()) / screenDist;

        _projRight.makeFrustum(left,right,bottom,top,_near,_far);

        // move camera to origin
        osg::Matrix cameraTrans;
        cameraTrans.makeTranslate(-eyeRight);

        //make view
        _viewRight = screenTrans * osg::Matrix::rotate(invViewerRot) * screenRot
                * cameraTrans
                * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                        osg::Vec3(0,0,1));
    }
}

void ScreenHMD::updateCamera()
{
    if(!_stereo)
    {
        _camera->setViewMatrix(_viewLeft);
        _camera->setProjectionMatrix(_projLeft);
    }
}

osg::Matrixd ScreenHMD::StereoCallback::computeLeftEyeProjection(
        const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projLeft;
}

osg::Matrixd ScreenHMD::StereoCallback::computeLeftEyeView(
        const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewLeft;
}

osg::Matrixd ScreenHMD::StereoCallback::computeRightEyeProjection(
        const osg::Matrixd &projection) const
{
    (void)projection;
    return screen->_projRight;
}

osg::Matrixd ScreenHMD::StereoCallback::computeRightEyeView(
        const osg::Matrixd &view) const
{
    (void)view;
    return screen->_viewRight;
}

void ScreenHMD::setClearColor(osg::Vec4 color)
{
    _camera->setClearColor(color);
}

ScreenInfo * ScreenHMD::findScreenInfo(osg::Camera * c)
{
    if(c == _camera.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenHMD::adjustViewportCoords(int & x, int & y)
{
    if(_stereoMode == osg::DisplaySettings::HORIZONTAL_SPLIT)
    {
        if(x > (_myInfo->myChannel->width / 2.0))
        {
            x = (int)(((float)x) - (_myInfo->myChannel->width / 2.0));
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
