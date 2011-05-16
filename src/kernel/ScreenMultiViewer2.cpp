#include <kernel/ScreenMultiViewer2.h>
#include <kernel/CVRCullVisitor.h>
#include <kernel/CVRViewer.h>
#include <kernel/NodeMask.h>
#include <kernel/PluginHelper.h>
#include <config/ConfigManager.h>
#include <input/TrackingManager.h>

#include <iostream>
#include <math.h>

using namespace cvr;

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MAX_ZONES_DEFAULT 16

// Statics
setContributionFunc ScreenMultiViewer2::setContribution;
std::vector<setContributionFunc> ScreenMultiViewer2::setContributionFuncs;
bool ScreenMultiViewer2::_orientation3d;
bool ScreenMultiViewer2::_autoAdjust;
bool ScreenMultiViewer2::_multipleUsers;
bool ScreenMultiViewer2::_zoneColoring;
float ScreenMultiViewer2::_autoAdjustTarget;
float ScreenMultiViewer2::_autoAdjustOffset;
int ScreenMultiViewer2::_setZoneColumns;
int ScreenMultiViewer2::_setZoneRows;
int ScreenMultiViewer2::_maxZoneColumns;
int ScreenMultiViewer2::_maxZoneRows;
osg::Vec4 ScreenMultiViewer2::_clearColor = osg::Vec4(0,0,0,0);

/*** Declarations for setContribution functions ***/
void linear(osg::Vec3 toZone0, osg::Vec3 orientation0, float &contribution0, osg::Vec3 toZone1, osg::Vec3 orientation1, float &contribution1);

void gaussian(osg::Vec3 toZone0, osg::Vec3 orientation0, float &contribution0, osg::Vec3 toZone1, osg::Vec3 orientation1, float &contribution1);
/**************************************************/

ScreenMultiViewer2::ScreenMultiViewer2() : ScreenMVSimulator()
{
}

ScreenMultiViewer2::~ScreenMultiViewer2()
{
}

void ScreenMultiViewer2::init(int mode)
{
    _stereoMode = (osg::DisplaySettings::StereoMode)mode;

    switch (_stereoMode)
    {
    default: // Shouldn't see this!
        std::cerr<<"Error! Invalid _stereoMode.\n";
    case osg::DisplaySettings::HORIZONTAL_INTERLACE:
        break; // NexCAVE
    case osg::DisplaySettings::LEFT_EYE:
        _viewPtr = &_viewLeftPtr;
        _projPtr = &_projLeftPtr;
        break;
    case osg::DisplaySettings::RIGHT_EYE:
        _viewPtr = &_viewRightPtr;
        _projPtr = &_projRightPtr;
        break;
    }

    _zonesChanged = true;
    _zones = 0;
    _zoneRows = 1;
    _zoneColumns = 1;
    _maxZoneColumns = ConfigManager::getInt("maxColumns","Zones",MAX_ZONES_DEFAULT);
    _maxZoneRows = ConfigManager::getInt("maxRows","Zones",4);
    _orientation3d = ConfigManager::getBool("Orientation3d",true);
    _autoAdjust = ConfigManager::getBool("FrameRate.autoAdjust",true);
    setAutoAdjustTarget(ConfigManager::getFloat("target","FrameRate",20));
    setAutoAdjustOffset(ConfigManager::getFloat("offset","FrameRate",4));
    
    _colorZones = false;

    /*** Setup setContributionFuncs Vector ***/
    setContribution = gaussian;
    setContributionFuncs.push_back(linear);
    setContributionFuncs.push_back(gaussian);
    /*** Done setting up setContributionFuncs Vector ***/

    _invScreenRotation = osg::Quat(-_myInfo->h*M_PI/180.0, osg::Vec3(0,0,1),
                                   -_myInfo->p*M_PI/180.0, osg::Vec3(1,0,0),
                                   -_myInfo->r*M_PI/180.0, osg::Vec3(0,1,0));

    // create cameras, one for each zone (based on maximum zone quantity)
    createCameras();

    // Only need a single zone for 1 user
    _multipleUsers = 1 < TrackingManager::instance()->getNumHeads();

    if (!_multipleUsers)
    {
        setZoneColumns(1);
        setZoneRows(1);
    }
    else
    {
        setZoneColumns(_maxZoneColumns);
        setZoneRows(_maxZoneRows);
    }
}

void ScreenMultiViewer2::computeViewProj()
{
    // Calculate zone quantity - sets _zonesChanged
    determineZoneQuantity();

    if (_zonesChanged)
    {
        _zones = _zoneRows * _zoneColumns;

        // Setup zones (zone centers)
        setupZones();

        // Setup cameras (recompute view matrices)
        setupCameras();        
    }

    // Handle zone coloring toggling
    if (_colorZones && !_zoneColoring)
    {
        // We just turned off zone coloring
        setClearColor(_clearColor);
    }
    _colorZones = _zoneColoring;

    // Find eye interpolated locations based on contributions from users
    std::vector<osg::Vec3> eyeLeft;
    std::vector<osg::Vec3> eyeRight;
    setEyeLocations(eyeLeft,eyeRight);

    //translate screen to origin
    osg::Matrix screenTrans;
    screenTrans.makeTranslate(-_myInfo->xyz);

    //rotate screen to xz
    osg::Matrix screenRot;
    screenRot.makeRotate(_invScreenRotation);

    // Move eyes via screen changes
    for (int i = 0; i < _zones; i++)
    {
        eyeLeft[i] = eyeLeft[i] * screenTrans * screenRot;
        eyeRight[i] = eyeRight[i] * screenTrans * screenRot;
    }

    float zHeight = _myInfo->height / _zoneRows;
    float zWidth = _myInfo->width / _zoneColumns;

    // Set up ComputeStereoMatricesCallback per camera
    for (int i = 0; i < _zones; i++)
    {
        int r = i / _zoneColumns;
        int c = i % _zoneColumns;

        osgViewer::Renderer * renderer = dynamic_cast<osgViewer::Renderer*> (_camera[i]->getRenderer());    
        StereoCallback * sc = dynamic_cast<StereoCallback*> (renderer->getSceneView(0)->getComputeStereoMatricesCallback());

        //make frustums
        float top, bottom, left, right;
        float screenDist;
        osg::Matrix cameraTrans;

        // NexCAVE or StarCAVE-Left-Eye
        if (_stereoMode != osg::DisplaySettings::RIGHT_EYE)
        {
            screenDist = -eyeLeft[i].y();

            top = _near * (-_myInfo->height / 2.0 + (r+1) * zHeight - eyeLeft[i].z()) / screenDist;
            bottom = _near * (-_myInfo->height / 2.0 + r * zHeight - eyeLeft[i].z()) / screenDist;
            left = _near * (-_myInfo->width / 2.0 + c * zWidth - eyeLeft[i].x()) / screenDist; 
            right = _near * (-_myInfo->width / 2.0 + (c+1) * zWidth - eyeLeft[i].x()) / screenDist;

            sc->_projLeft.makeFrustum(left, right, bottom, top, _near, _far);

            // move camera to origin
            cameraTrans.makeTranslate(-eyeLeft[i]);

            //make view
            sc->_viewLeft = screenTrans * screenRot * cameraTrans
                * osg::Matrix::lookAt(osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0),
                                  osg::Vec3(0, 0, 1));
        }

        // NexCAVE or StarCAVE-Right-Eye
        if (_stereoMode != osg::DisplaySettings::LEFT_EYE)
        {
            screenDist = -eyeRight[i].y();

            top = _near * (-_myInfo->height / 2.0 + (r+1) * zHeight - eyeRight[i].z()) / screenDist;
            bottom = _near * (-_myInfo->height / 2.0 + r * zHeight - eyeRight[i].z()) / screenDist;
            left = _near * (-_myInfo->width / 2.0 + c * zWidth - eyeRight[i].x()) / screenDist; 
            right = _near * (-_myInfo->width / 2.0 + (c+1) * zWidth - eyeRight[i].x()) / screenDist;

            sc->_projRight.makeFrustum(left, right, bottom, top, _near, _far);

            // move camera to origin
            cameraTrans.makeTranslate(-eyeRight[i]);

            //make view
            sc->_viewRight = screenTrans * screenRot * cameraTrans
                * osg::Matrix::lookAt(osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0),
                                  osg::Vec3(0, 0, 1));
        }

        _camera[i]->setCullMask(CULL_MASK);
        _camera[i]->setCullMaskLeft(CULL_MASK_LEFT);
        _camera[i]->setCullMaskRight(CULL_MASK_RIGHT);
    }
    for (int i = _zones; i < _camera.size(); i++)
    {
        _camera[i]->setCullMask(0);
        _camera[i]->setCullMaskLeft(0);
        _camera[i]->setCullMaskRight(0);
    }
}

void ScreenMultiViewer2::updateCamera()
{
    if (_viewPtr == NULL || _projPtr == NULL)
        return; // Not in StarCAVE

    for (int i=0; i < _camera.size(); i++)
    {
        _camera[i]->setViewMatrix( *(*_viewPtr)[i] );
        _camera[i]->setProjectionMatrix( *(*_projPtr)[i] );
    }
}

osg::Matrixd ScreenMultiViewer2::StereoCallback::computeLeftEyeProjection(const osg::Matrixd &) const
{
    return _projLeft;
}

osg::Matrixd ScreenMultiViewer2::StereoCallback::computeLeftEyeView(const osg::Matrixd &) const
{
    return _viewLeft;
}

osg::Matrixd ScreenMultiViewer2::StereoCallback::computeRightEyeProjection(const osg::Matrixd &) const
{
    return _projRight;
}

osg::Matrixd ScreenMultiViewer2::StereoCallback::computeRightEyeView(const osg::Matrixd &) const
{
    return _viewRight;
}

void ScreenMultiViewer2::setClearColor(osg::Vec4 color)
{
    _clearColor = color;

    for (int i = 0; i < _camera.size(); i++)
        _camera[i]->setClearColor(color);
}

ScreenInfo * ScreenMultiViewer2::findScreenInfo(osg::Camera * c)
{
    for (int i = 0; i < _camera.size(); i++)
    {
        if(c == _camera[i].get())
        {
            return _myInfo;
        }
    }
    return NULL;
}

void ScreenMultiViewer2::createCameras()
{
    osg::GraphicsContext* gc = _myInfo->myChannel->myWindow->gc;
    GLenum buffer = _myInfo->myChannel->myWindow->gc->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
    int quantity = _maxZoneRows * _maxZoneColumns;

    // make new cameras as necessary
    for (int i = 0; i < quantity; i++)
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        _camera.push_back(cam);
        cam->setGraphicsContext(gc);

        cam->setDrawBuffer(buffer);
        cam->setReadBuffer(buffer);

        cam->setClearColor(_clearColor);

        CVRViewer::instance()->addSlave(_camera[i].get(), osg::Matrixd(), osg::Matrixd());
        osgViewer::Renderer * renderer = dynamic_cast<osgViewer::Renderer*> (_camera[i]->getRenderer());

        if (!renderer)
        {
            std::cerr << "Error getting renderer pointer." << std::endl;
        }
        else
        {
            osg::DisplaySettings * ds = renderer->getSceneView(0)->getDisplaySettings();

            ds->setStereo(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE);
            ds->setStereoMode(_stereoMode);

            StereoCallback * sc = new StereoCallback;

            _projLeftPtr.push_back(&sc->_projLeft);
            _projRightPtr.push_back(&sc->_projRight);
            _viewLeftPtr.push_back(&sc->_viewLeft);
            _viewRightPtr.push_back(&sc->_viewRight);

            renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
            renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);
            renderer->getSceneView(0)->getDisplaySettings()->setSerializeDrawDispatch(false);
            renderer->getSceneView(1)->getDisplaySettings()->setSerializeDrawDispatch(false);

            if (ConfigManager::getEntry("value","CullingMode","CALVR") == "CALVR")
            {
                renderer->getSceneView(0)->setCullVisitor(new CVRCullVisitor());
                renderer->getSceneView(0)->setCullVisitorLeft(new CVRCullVisitor());
                renderer->getSceneView(0)->setCullVisitorRight(new CVRCullVisitor());
                renderer->getSceneView(1)->setCullVisitor(new CVRCullVisitor());
                renderer->getSceneView(1)->setCullVisitorLeft(new CVRCullVisitor());
                renderer->getSceneView(1)->setCullVisitorRight(new CVRCullVisitor());
            }
        }
    }
}

void ScreenMultiViewer2::determineZoneQuantity()
{
    _zonesChanged = true; // assume true, set false if wrong
    
    // set zone row/column quantity to 1 for a single user
    if (!_multipleUsers)
    {
        _zoneRows = _zoneColumns = 1;
        
        // Did we change since last frame?
        _zonesChanged = _zones != 1;
        return;
    }

    if (!_autoAdjust)
    {
        if (_setZoneColumns == _zoneColumns && _setZoneRows == _zoneRows)
        {
            _zonesChanged = false;
            return;
        }
            
        _zoneColumns = _setZoneColumns;
        _zoneRows = _setZoneRows;
        return;
    }

    double lastFrameDuration = CVRViewer::instance()->getLastFrameDuration();

    if (1.0 / lastFrameDuration < _autoAdjustTarget - _autoAdjustOffset)
    {
        if (_zoneColumns > 1)
            _zoneColumns--;
        else if (_zoneRows > 1)
            _zoneRows--;
        else
            _zonesChanged = false;
    }
    else if (1.0 / lastFrameDuration > _autoAdjustTarget + _autoAdjustOffset)
    {
        if (_zoneColumns < _maxZoneColumns)
            _zoneColumns++;
        else if (_zoneRows < _maxZoneRows)
            _zoneRows++;
        else
            _zonesChanged = false;
    }
    else
        _zonesChanged = false;

    // balance out rows/columns if we have changed them
    if (_zonesChanged)
    {
        const float THRESHOLD = 1.5;

        if (_zoneColumns*_zoneRows > THRESHOLD*_maxZoneColumns*_zoneRows && _zoneRows < _maxZoneRows)
        {
            _zoneColumns = _zoneColumns * _zoneRows / (_zoneRows + 1);
            _zoneRows++;
        }
        else if (THRESHOLD*_zoneColumns*_maxZoneRows < _maxZoneColumns*_zoneRows && _zoneRows > 1)
        {
            _zoneColumns = _zoneColumns * _zoneRows / (_zoneRows - 1);
            _zoneRows--;
        }
    }
}

void ScreenMultiViewer2::setupZones()
{
    for (int i = _zoneCenter.size(); i < _zones; i++)
    {
        _zoneCenter.push_back(osg::Vec3());
    }

    float left = _myInfo->myChannel->left;
    float bottom = _myInfo->myChannel->bottom;
    float zoneWidth = _myInfo->myChannel->width / _zoneColumns;
    float zoneHeight = _myInfo->myChannel->height / _zoneRows;

    for (int r = 0; r < _zoneRows; r++)
    {
        for (int c = 0; c < _zoneColumns; c++)
        {
            osg::Vec4 zc = osg::Vec4(left + (c+.5)*zoneWidth, bottom + (r+.5)*zoneHeight,0,1) * _myInfo->transform;
            _zoneCenter[r*_zoneColumns + c] = osg::Vec3(zc.x(),zc.y(),zc.z());
        }
    }
}

void ScreenMultiViewer2::setupCameras()
{
    float left = _myInfo->myChannel->left;
    float bottom = _myInfo->myChannel->bottom;
    float zoneWidth = _myInfo->myChannel->width / _zoneColumns;
    float zoneHeight = _myInfo->myChannel->height / _zoneRows;

    for (int i = 0; i < _camera.size(); i++)
    {
        // Camera needs its viewport setup
        if (i < _zones)
        {
            int r = i / _zoneColumns;
            int c = i % _zoneColumns;

            // "extra" calculation do to float->int rounding
            _camera[i]->setViewport((int)(left + c*zoneWidth),
                               (int)(bottom + r*zoneHeight),
                               (int)(left+(c+1)*zoneWidth)-(int)(left+c*zoneWidth),
                               (int)(bottom+(r+1)*zoneHeight)-(int)(bottom+r*zoneHeight));
        }
        else //*(i >= _zones)*/
        {
            _camera[i]->setViewport(0,0,0,0);
        }
    }
}

void ScreenMultiViewer2::setEyeLocations(std::vector<osg::Vec3> &eyeLeft,std::vector<osg::Vec3> &eyeRight)
{
    // For a single user, just use the default eye positions
    if (!_multipleUsers)
    {
        eyeLeft.push_back( defaultLeftEye(0) );
        eyeRight.push_back( defaultRightEye(0) );
        return;
    }

    // Get the head matrices and the eye positions
    osg::Matrix headMat0 = getCurrentHeadMatrix(0);
    osg::Matrix headMat1 = getCurrentHeadMatrix(1);
    osg::Vec3 pos0 = headMat0.getTrans();
    osg::Vec3 pos1 = headMat1.getTrans();

    osg::Vec3 eyeLeft0 = defaultLeftEye(0);
    osg::Vec3 eyeRight0 = defaultRightEye(0);
    osg::Vec3 eyeLeft1 = defaultLeftEye(1);
    osg::Vec3 eyeRight1 = defaultRightEye(1);

    // user orientations
    osg::Vec4 u0op = osg::Vec4(0,1,0,0) * headMat0;
    osg::Vec4 u1op = osg::Vec4(0,1,0,0) * headMat1;
    osg::Vec3 o0,o1;

    if (_orientation3d)
    {
       o0 = osg::Vec3(u0op.x(),u0op.y(),u0op.z());
       o1 = osg::Vec3(u1op.x(),u1op.y(),u1op.z());
    }
    else
    {
       o0 = osg::Vec3(u0op.x(),u0op.y(),0);
       o1 = osg::Vec3(u1op.x(),u1op.y(),0);
       o0.normalize();
       o1.normalize();
    }

    // compute contributions and set eye locations
    for (int i = 0; i < _zones; i++)
    {
        // user to zone center vectors
        osg::Vec3 u0toZC = _zoneCenter[i] - pos0;
        osg::Vec3 u1toZC = _zoneCenter[i] - pos1;

        // normalize all vectors
        u0toZC.normalize();
        u1toZC.normalize();

        // set the contribution level of each user to the zone
        float contribution0, contribution1;
        setContribution(u0toZC, o0, contribution0, u1toZC, o1, contribution1);

        // set default values for the eyes for this camera
        eyeLeft.push_back( eyeLeft0 * contribution0 + eyeLeft1 * contribution1 );
        eyeRight.push_back( eyeRight0 * contribution0 + eyeRight1 * contribution1 );

        // set this camera's "clear color" based on contributions as neccessary
        if (_colorZones)
        {
            _camera[i]->setClearColor(osg::Vec4(contribution0,0,contribution1,0));
        }
    }
}

void linear(osg::Vec3 toZone0, osg::Vec3 orientation0, float &contribution0, osg::Vec3 toZone1, osg::Vec3 orientation1, float &contribution1)
{
        // contribution factors for each user - in radians
        contribution0 = MAX(M_PI/2*0.01, acos(toZone0 * orientation0));
        contribution1 = MAX(M_PI/2*0.01, acos(toZone1 * orientation1));

        float cTotal = contribution0 + contribution1;

        contribution0 /= cTotal;
        contribution1 /= cTotal;
}

void gaussian(osg::Vec3 toZone0, osg::Vec3 orientation0, float &contribution0, osg::Vec3 toZone1, osg::Vec3 orientation1, float &contribution1)
{
        // contribution factors for each user
        contribution0 = MAX(0.01, toZone0 * orientation0);
        contribution1 = MAX(0.01, toZone1 * orientation1);

        float cTotal = contribution0 + contribution1;

        contribution0 /= cTotal;

        contribution1 /= cTotal;
}

bool ScreenMultiViewer2::setSetContributionFunc(int funcNum)
{
    if (funcNum < 0 || funcNum >= setContributionFuncs.size())
        return false;

    setContribution = setContributionFuncs[funcNum];
    return true;
}

void ScreenMultiViewer2::setOrientation3d(bool o3d)
{
    _orientation3d = o3d;
}

bool ScreenMultiViewer2::getOrientation3d()
{
    return _orientation3d;
}

void ScreenMultiViewer2::setZoneColumns(int columns)
{
    if (columns < 1 || columns > _maxZoneColumns)
        return;

    // in single user mode -> should not have multiple zones
    if (columns != 1 && !_multipleUsers)
        return;

    _setZoneColumns = columns;
}

void ScreenMultiViewer2::setZoneRows(int rows)
{
    if (rows < 1 || rows > _maxZoneRows)
        return;

    // in single user mode -> should not have multiple zones
    if (rows != 1 && !_multipleUsers)
        return;

    _setZoneRows = rows;
}

int ScreenMultiViewer2::getZoneColumns()
{
    return _setZoneColumns;
}

int ScreenMultiViewer2::getMaxZoneColumns()
{
    return _maxZoneColumns;
}
int ScreenMultiViewer2::getZoneRows()
{
    return _setZoneRows;
}

int ScreenMultiViewer2::getMaxZoneRows()
{
    return _maxZoneRows;
}

void ScreenMultiViewer2::setAutoAdjust(bool adjust)
{
    _autoAdjust = adjust;
}

bool ScreenMultiViewer2::getAutoAdjust()
{
    return _autoAdjust;
}

void ScreenMultiViewer2::setAutoAdjustTarget(float target)
{
    if (target < 0)
        return;

    _autoAdjustTarget = target;
}

float ScreenMultiViewer2::getAutoAdjustTarget()
{
    return _autoAdjustTarget;
}

void ScreenMultiViewer2::setAutoAdjustOffset(float offset)
{
    if (offset < 0)
        return;

    _autoAdjustOffset = offset;
}

float ScreenMultiViewer2::getAutoAdjustOffset()
{
    return _autoAdjustOffset;
}

void ScreenMultiViewer2::setMultipleUsers(bool multipleUsers)
{
    _multipleUsers = multipleUsers;
}

bool ScreenMultiViewer2::getMultipleUsers()
{
    return _multipleUsers;
}

void ScreenMultiViewer2::setZoneColoring(bool zoneColoring)
{
    _zoneColoring = zoneColoring;
}

bool ScreenMultiViewer2::getZoneColoring()
{
    return _zoneColoring;
}
