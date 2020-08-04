#include <cvrKernel/ScreenMVStencil.h>
#include <cvrKernel/CVRCullVisitor.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrUtil/OsgPrint.h>
#include <cvrMenu/MenuSystem.h>

#include <osgViewer/Renderer>
#include <osg/Stencil>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstring>

#ifdef WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

using namespace cvr;

const float ScreenMVStencil::EPSILON = 0.00001;
const float ScreenMVStencil::T_MAX = 1.0;
const float ScreenMVStencil::T_MIN = -1.0;

cvr::MenuCheckbox *ScreenMVStencil::_debug_mode = NULL;
cvr::MenuCheckbox *ScreenMVStencil::_align_head = NULL;
cvr::SubMenu *ScreenMVStencil::_mvs_menu = NULL;
cvr::MenuRangeValue *ScreenMVStencil::_fov_dial = NULL;

ScreenMVStencil::ScreenMVStencil() :
        ScreenBase()
{
}

ScreenMVStencil::~ScreenMVStencil()
{
}

void ScreenMVStencil::init(int mode)
{
    //this is passed in from ScreenConfig
    //creates ScreenMVStencil() for each eye in StarCave
    _stereoMode = (osg::DisplaySettings::StereoMode)mode;
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

    computeScreenInfoXZ();

    if(_debug_mode == NULL)
        _debug_mode = new MenuCheckbox("Debug Mode",true);

    if(_align_head == NULL)
        _align_head = new MenuCheckbox("Align Head",false);

    if(_fov_dial == NULL)
        _fov_dial = new MenuRangeValue("Field of View",1,180,120,1.0);

    if(_mvs_menu == NULL)
    {
        _mvs_menu = new SubMenu("ScreenMVStencil","ScreenMVStencil");
        _mvs_menu->addItem(_debug_mode);
        _mvs_menu->addItem(_align_head);
        _mvs_menu->addItem(_fov_dial);
    }

    //number of times to render per eye
    _num_render = 3;
    createCameras(_num_render);
}

void ScreenMVStencil::computeScreenInfoXZ()
{
    _screenInfoXZ = (ScreenInfoXZ *)malloc(sizeof(ScreenInfoXZ));

    //X-axis is pointing right
    //Y-axis is pointing into the screen
    //Z-axis is pointing up
    //screen is initially aligned to origin in XZ plane
    _screenInfoXZ->normal = osg::Vec3f(0,-1,0);

    //this is the actual screen's width and height measurements
    double h2 = _myInfo->height / 2, w2 = _myInfo->width / 2;

    //initially center is at (0,0,0) in XZ plane
    _screenInfoXZ->top_left = osg::Vec3f(-w2,0,h2);
    _screenInfoXZ->top_right = osg::Vec3f(w2,0,h2);
    _screenInfoXZ->bottom_left = osg::Vec3f(-w2,0,-h2);
    _screenInfoXZ->bottom_right = osg::Vec3f(w2,0,-h2);
    _screenInfoXZ->inv_transform = osg::Matrixd::inverse(_myInfo->transform);
}

void ScreenMVStencil::createCameras(unsigned int quantity)
{
    std::string shaderDir;
    char * cvrHome = getenv("CALVR_HOME");
    if(cvrHome)
    {
        shaderDir = cvrHome;
        shaderDir = shaderDir + "/";
    }
    shaderDir = shaderDir + "shaders/";

    for(unsigned int i = 0; i < quantity; ++i)
    {
        osg::Camera * cam = new osg::Camera;
        _cameras.push_back(cam);
        CVRViewer::instance()->addSlave(cam,osg::Matrix(),osg::Matrix());
        defaultCameraInit(cam);

        //There are 2 eyes.  Each is in a different thread.
        //Each thread has "quantity" cameras.
        //This will be called before each camera gets to render
        PreDrawCallback * pdc = new PreDrawCallback;
        pdc->screen = this;
        pdc->render_state = i;
        pdc->vertShader = shaderDir + "interleaver.vert";
        pdc->fragShader = shaderDir + "interleaver.frag";

        pdc->cameraList = &_cameras;
        pdc->scList = &_stereoCallbacks;
        _preCallbacks.push_back(pdc);

        cam->setPreDrawCallback(pdc);

        osgViewer::Renderer * renderer =
                dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
        //both scene 0 and 1 will be the same function
        osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
        //set to false for StarCave.  When false, the StereoBallback does
        //not matter.  True is for NexCave.
        //ds->setStereo(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE);
        ds->setStereo(true);
        ds->setStereoMode(_stereoMode);

        StereoCallback * sc = new StereoCallback;
        _stereoCallbacks.push_back(sc);

        renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
        renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);
    }
}

void ScreenMVStencil::computeViewProj()
{
    std::vector<osg::Vec3> iLeft;
    std::vector<osg::Vec3> iRight;
    iLeft.push_back(defaultLeftEye(0));
    iLeft.push_back(defaultLeftEye(1));
    iLeft.push_back((iLeft.at(0) + iLeft.at(1)) * 0.5);
    iRight.push_back(defaultRightEye(0));
    iRight.push_back(defaultRightEye(1));
    iRight.push_back((iRight.at(0) + iRight.at(1)) * 0.5);

    for(unsigned int i = 0; i < _cameras.size(); ++i)
    {
        StereoCallback * sc = _stereoCallbacks.at(i);
        computeDefaultViewProj(iLeft.at(i),sc->_viewLeft,sc->_projLeft);
        computeDefaultViewProj(iRight.at(i),sc->_viewRight,sc->_projRight);
    }
}

void ScreenMVStencil::updateCamera()
{
    if(osg::DisplaySettings::HORIZONTAL_INTERLACE == _stereoMode)
        return;

    for(unsigned int i = 0; i < _cameras.size(); ++i)
    {
        if(osg::DisplaySettings::LEFT_EYE == _stereoMode)
        {
            _cameras[i]->setViewMatrix(_stereoCallbacks[i]->_viewLeft);
            _cameras[i]->setProjectionMatrix(_stereoCallbacks[i]->_projLeft);
        }
        else if(osg::DisplaySettings::RIGHT_EYE == _stereoMode)
        {
            _cameras[i]->setViewMatrix(_stereoCallbacks[i]->_viewRight);
            _cameras[i]->setProjectionMatrix(_stereoCallbacks[i]->_projRight);
        }
        else
        {
            std::cerr << "Only LEFT_EYE and RIGHT_EYE are supported.\n";
            exit(0);
        }
    }
}

osg::Matrixd ScreenMVStencil::StereoCallback::computeLeftEyeProjection(
        const osg::Matrixd &projection) const
{
    return _projLeft;
}

osg::Matrixd ScreenMVStencil::StereoCallback::computeLeftEyeView(
        const osg::Matrixd &view) const
{
    return _viewLeft;
}

osg::Matrixd ScreenMVStencil::StereoCallback::computeRightEyeProjection(
        const osg::Matrixd &projection) const
{
    return _projRight;
}

osg::Matrixd ScreenMVStencil::StereoCallback::computeRightEyeView(
        const osg::Matrixd &view) const
{
    return _viewRight;
}

void ScreenMVStencil::setClearColor(osg::Vec4 color)
{
    for(int i = 0; i < _cameras.size(); i++)
        _cameras[i]->setClearColor(color);
}

ScreenInfo * ScreenMVStencil::findScreenInfo(osg::Camera * c)
{
    for(unsigned int i = 0; i < _cameras.size(); ++i)
    {
        if(c == _cameras[i])
        {
            return _myInfo;
        }
    }
    return NULL;
}

bool ScreenMVStencil::handleLineScreenIntersection(
        IntersectionPlane & plane) const
{
    osg::Vec3f xz_point = plane.point;
    osg::Vec3f xz_dir = plane.dir;
    xz_dir.normalize();

    plane.hit_top = lineSegmentIntersection2D_XZ(xz_point,xz_dir,
            _screenInfoXZ->top_left,_screenInfoXZ->top_right,plane.t_top);

    plane.hit_bottom = lineSegmentIntersection2D_XZ(xz_point,xz_dir,
            _screenInfoXZ->bottom_left,_screenInfoXZ->bottom_right,
            plane.t_bottom);

    plane.hit_left = lineSegmentIntersection2D_XZ(xz_point,xz_dir,
            _screenInfoXZ->bottom_left,_screenInfoXZ->top_left,plane.t_left);

    plane.hit_right = lineSegmentIntersection2D_XZ(xz_point,xz_dir,
            _screenInfoXZ->bottom_right,_screenInfoXZ->top_right,plane.t_right);

    plane.t_top = plane.t_top * 2.0 - 1.0;
    plane.t_bottom = plane.t_bottom * 2.0 - 1.0;
    plane.t_left = plane.t_left * 2.0 - 1.0;
    plane.t_right = plane.t_right * 2.0 - 1.0;

    return plane.hit_top || plane.hit_bottom || plane.hit_right
            || plane.hit_left;
}

void ScreenMVStencil::stencilOutView(const CameraOrient &cam,
        IntersectionPlane & l_p, IntersectionPlane & r_p, GLint ref,
        GLuint mask, GLuint write_mask) const
{
    std::vector<osg::Vec3f> list_of_points;
    //add all hit points into a list
    if(l_p.hit_left)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(_myInfo->width / -2.0f,0,
                        l_p.t_left * _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(T_MIN,l_p.t_left,T_MIN));
        else
            l_p.hit_left = false;
    }

    if(l_p.hit_right)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(_myInfo->width / 2.0f,0,
                        l_p.t_right * _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(T_MAX,l_p.t_right,T_MIN));
        else
            l_p.hit_right = false;
    }

    if(l_p.hit_top)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(l_p.t_top * _myInfo->width / 2.0f,0,
                        _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(l_p.t_top,T_MAX,T_MIN));
        else
            l_p.hit_top = false;
    }

    if(l_p.hit_bottom)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(l_p.t_bottom * _myInfo->width / 2.0f,0,
                        _myInfo->height / -2.0f)))
            list_of_points.push_back(osg::Vec3f(l_p.t_bottom,T_MIN,T_MIN));
        else
            l_p.hit_bottom = false;
    }

    if(r_p.hit_left)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(_myInfo->width / -2.0f,0,
                        r_p.t_left * _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(T_MIN,r_p.t_left,T_MIN));
        else
            r_p.hit_left = false;
    }

    if(r_p.hit_right)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(_myInfo->width / 2.0f,0,
                        r_p.t_right * _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(T_MAX,r_p.t_right,T_MIN));
        else
            r_p.hit_right = false;
    }

    if(r_p.hit_top)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(r_p.t_top * _myInfo->width / 2.0f,0,
                        _myInfo->height / 2.0f)))
            list_of_points.push_back(osg::Vec3f(r_p.t_top,T_MAX,T_MIN));
        else
            r_p.hit_top = false;
    }

    if(r_p.hit_bottom)
    {
        if(IsInsideOfPlane(cam.eye,cam.viewDir,
                osg::Vec3f(r_p.t_bottom * _myInfo->width / 2.0f,0,
                        _myInfo->height / -2.0f)))
            list_of_points.push_back(osg::Vec3f(r_p.t_bottom,T_MIN,T_MIN));
        else
            r_p.hit_bottom = false;
    }

    if(list_of_points.empty() || list_of_points.size() < 2)
        return;

    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glStencilFunc(GL_EQUAL,ref,mask);
    glStencilMask(write_mask);

    /* super corner case: This will fix it but cause second row of starCave
     * to have issues.
     if(
     list_of_points.size() == 2 && l_p.hit_left && l_p.hit_bottom)) {
     list_of_points.push_back(osg::Vec3f(T_MIN, T_MIN, T_MIN));
     ScreenMVStencil::sortXYPolarStartRight(list_of_points);
     fullscreenTriangle(list_of_points.at(0), list_of_points.at(1),
     list_of_points.at(2));
     return;
     } else if(
     list_of_points.size() == 2 && r_p.hit_right &&
     r_p.hit_bottom) {
     list_of_points.push_back(osg::Vec3f(T_MAX, T_MIN, T_MIN));
     ScreenMVStencil::sortXYPolarStartLeft(list_of_points);
     fullscreenTriangle(list_of_points.at(0), list_of_points.at(1),
     list_of_points.at(2));
     return;
     }
     */

    /*
     * Three ways to intersect XZ plane
     * Case 1: left and right plane hits only the top and bottom XZ.
     *         This case has only 4 hit points.  Already taken care of
     *         through adding all points of left and right plane.
     * Case 2: One of the frustum plane is offscreen.
     *         This case has 3/4/5 hit points depending on whether the
     *         frustum that hits the plane hits a side of the screen or
     *         only the top and bottom of the screen.
     * Case 3: Both frustum planes intersect the XZ plane, but one or
     *         both hit the side of the plane.
     *         This case has 5/6 hit points depending on whether one of the
     *         intersection is TOP|BOTTOM + TOP|BOTTOM, or
     *         SIDE + TOP/BOTTOM respectively.
     */
    std::vector<int> lp_list;
    std::vector<int> rp_list;

    IsInLeftView(l_p,lp_list);
    IsInRightView(r_p,rp_list);

    if(!lp_list.empty() && !rp_list.empty())
    {
        std::vector<int> final_list;
        std::sort(lp_list.begin(),lp_list.end(),ScreenMVStencil::sort_by_int);
        std::sort(rp_list.begin(),rp_list.end(),ScreenMVStencil::sort_by_int);

        std::set_intersection(lp_list.begin(),lp_list.end(),rp_list.begin(),
                rp_list.end(),back_inserter(final_list));
        AddScreenCorners(final_list,list_of_points);
    }
    else if(lp_list.empty())
    {
        std::sort(rp_list.begin(),rp_list.end(),ScreenMVStencil::sort_by_int);

        AddScreenCorners(rp_list,list_of_points);
    }
    else if(rp_list.empty())
    {
        std::sort(lp_list.begin(),lp_list.end(),ScreenMVStencil::sort_by_int);

        AddScreenCorners(lp_list,list_of_points);
    }

    if(list_of_points.empty())
        return;

    ScreenMVStencil::sortXYPolarStartRight(list_of_points);

    std::cerr << l_p.hit_left << " " << l_p.hit_right << " " << l_p.hit_top
            << " " << l_p.hit_bottom << " " << list_of_points.size() << " "
            << r_p.hit_left << " " << r_p.hit_right << " " << r_p.hit_top << " "
            << r_p.hit_bottom << std::endl;

    if(list_of_points.size() == 2)
    {
        /* 1) Both left and right plane hits only top or bottom.
         * 2) Left and right only hit one side respectively to their name.
         * 3) One hits the top/bottom while the other hits the left/right.
         */
        double t_hit = 0;
        osg::Vec3f dist = cam.up * 10000;
        osg::Vec3f result_point(0,0,0);

        if(LinePlaneIntersection(cam.eye - dist,cam.eye + dist,
                osg::Vec3f(0,0,0),osg::Vec3f(0,-1,0),result_point,t_hit))
        {
            result_point.x() = result_point.x() / (_myInfo->width / 2.0);
            result_point.y() = result_point.z() / (_myInfo->height / 2.0);
            result_point.z() = T_MIN;

            if((l_p.hit_top && r_p.hit_top)
                    || (l_p.hit_bottom && r_p.hit_bottom)
                    || (l_p.hit_left && r_p.hit_left)
                    || (l_p.hit_right && r_p.hit_right))
            {
                list_of_points.push_back(result_point);

                ScreenMVStencil::sortXYPolarStartRight(list_of_points);
                fullscreenTriangle(list_of_points.at(0),list_of_points.at(1),
                        list_of_points.at(2));
            }
            else if(l_p.hit_left && r_p.hit_right)
            {
                list_of_points.push_back(result_point);

                if((_screenInfoXZ->normal * cam.up) >= 0.0)
                {
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MAX,T_MIN));
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MAX,T_MIN));
                }
                else
                {
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MIN,T_MIN));
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MIN,T_MIN));
                }
                ScreenMVStencil::sortXYPolarStartRight(list_of_points);

                fullscreenTriangle(list_of_points.at(0),list_of_points.at(1),
                        list_of_points.at(2));
                fullscreenQuad(list_of_points.at(2),list_of_points.at(3),
                        list_of_points.at(4),list_of_points.at(0));
            }
            else
            {
                if(l_p.hit_left && r_p.hit_top)
                {
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MAX,T_MIN));
                    ScreenMVStencil::sortXYPolarStartRight(list_of_points);
                }
                else if(l_p.hit_top && r_p.hit_right)
                {
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MAX,T_MIN));
                    ScreenMVStencil::sortXYPolarStartLeft(list_of_points);
                }
                else if(l_p.hit_left && r_p.hit_bottom)
                {
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MIN,T_MIN));
                    ScreenMVStencil::sortXYPolarStartRight(list_of_points);
                }
                else if(l_p.hit_bottom && r_p.hit_right)
                {
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MIN,T_MIN));
                    ScreenMVStencil::sortXYPolarStartLeft(list_of_points);
                }
                else if(l_p.hit_top && r_p.hit_bottom)
                {
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MAX,T_MIN));
                    list_of_points.push_back(osg::Vec3f(T_MIN,T_MIN,T_MIN));
                    ScreenMVStencil::sortXYPolarStartRight(list_of_points);
                }
                else if(l_p.hit_bottom && r_p.hit_top)
                {
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MAX,T_MIN));
                    list_of_points.push_back(osg::Vec3f(T_MAX,T_MIN,T_MIN));
                    ScreenMVStencil::sortXYPolarStartLeft(list_of_points);
                }

                for(unsigned idx = 0; idx < list_of_points.size() - 1; ++idx)
                {
                    fullscreenTriangle(result_point,list_of_points.at(idx),
                            list_of_points.at(idx + 1));
                }
            }
        }
    }
    else if(list_of_points.size() == 3)
    {
        fullscreenTriangle(list_of_points.at(0),list_of_points.at(1),
                list_of_points.at(2));
    }
    else if(list_of_points.size() == 4)
    {
        fullscreenQuad(list_of_points.at(0),list_of_points.at(1),
                list_of_points.at(2),list_of_points.at(3));
    }
    else if(list_of_points.size() == 5)
    {
        fullscreenTriangle(list_of_points.at(0),list_of_points.at(1),
                list_of_points.at(2));
        fullscreenQuad(list_of_points.at(2),list_of_points.at(3),
                list_of_points.at(4),list_of_points.at(0));
    }
    else if(list_of_points.size() == 6)
    {
        fullscreenQuad(list_of_points.at(0),list_of_points.at(1),
                list_of_points.at(2),list_of_points.at(3));
        fullscreenQuad(list_of_points.at(3),list_of_points.at(4),
                list_of_points.at(5),list_of_points.at(0));
    }
}

bool ScreenMVStencil::IsInsideOfFrustum(const CameraOrient &cam,
        const osg::Vec3f &p) const
{
    return IsInsideOfPlane(cam.leftPlanePoint,cam.leftPlaneNormal,p)
            && IsInsideOfPlane(cam.rightPlanePoint,cam.rightPlaneNormal,p)
            && IsInsideOfPlane(cam.eye,cam.viewDir,p);
}

bool ScreenMVStencil::handleCameraScreenIntersection(const CameraOrient & cam,
        GLint ref, GLuint mask, GLuint write_mask) const
{
    bool top_left_in = IsInsideOfFrustum(cam,_screenInfoXZ->top_left);
    bool bottom_left_in = IsInsideOfFrustum(cam,_screenInfoXZ->bottom_left);
    bool top_right_in = IsInsideOfFrustum(cam,_screenInfoXZ->top_right);
    bool bottom_right_in = IsInsideOfFrustum(cam,_screenInfoXZ->bottom_right);
    bool do_intersection = false;

    //need to take care of case where screen is completely inside view
    if(top_left_in && bottom_left_in && top_right_in && bottom_right_in)
    {
        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
        glStencilFunc(GL_EQUAL,ref,mask);
        glStencilMask(write_mask);

        fullscreenQuad(osg::Vec3f(-1,1,-1),osg::Vec3f(-1,-1,-1),
                osg::Vec3f(1,-1,-1),osg::Vec3f(1,1,-1));
        do_intersection = true;
    }
    else
    {
        IntersectionPlane left, right;

        //optimization to avoid else-statements
        memset(&left,0,sizeof(left));
        memset(&right,0,sizeof(right));

        //find the intersection line of two planes
        left.hasIntersected = planePlaneIntersection(cam.leftPlanePoint,
                cam.leftPlaneNormal,_screenInfoXZ->top_left,
                _screenInfoXZ->normal,left.point,left.dir);

        right.hasIntersected = planePlaneIntersection(cam.rightPlanePoint,
                cam.rightPlaneNormal,_screenInfoXZ->top_left,
                _screenInfoXZ->normal,right.point,right.dir);

        if(left.hasIntersected && right.hasIntersected)
        {
            do_intersection = handleLineScreenIntersection(left)
                    && handleLineScreenIntersection(right);
        }
        if(left.hasIntersected && !do_intersection)
        {
            do_intersection = handleLineScreenIntersection(left);
        }
        if(right.hasIntersected && !do_intersection)
        {
            do_intersection = handleLineScreenIntersection(right);
        }

        if(do_intersection)
            stencilOutView(cam,left,right,ref,mask,write_mask);
    }
    return do_intersection;
}

// set up stencil buffer into 3 zones (due to 2 views)
// 0 - no one is looking there
// 1 - one person is looking there
// 2 - other person is looking there
// 3 - both of them have views overlapping
void ScreenMVStencil::setupZones(CameraOrient & cam0, CameraOrient & cam1) const
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if(!_debug_mode->getValue())
    {
        glColorMask(0,0,0,0);

        handleCameraScreenIntersection(cam0,1,0,~0);
        glClear(GL_DEPTH_BUFFER_BIT);

        handleCameraScreenIntersection(cam1,2,0,0x2);
        glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(1,1,1,1);
    }
    else
    {
        glColor3f(1,0,0);

        handleCameraScreenIntersection(cam0,1,0,~0);
        glClear(GL_DEPTH_BUFFER_BIT);

        glColor3f(0,1,0);
        handleCameraScreenIntersection(cam1,2,0,0x2);
        glClear(GL_DEPTH_BUFFER_BIT);

        glColor3f(0,0,1);
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
        glStencilFunc(GL_EQUAL,3,~0);
        glStencilMask(0);

        fullscreenQuad(osg::Vec3f(-1,1,-1),osg::Vec3f(-1,-1,-1),
                osg::Vec3f(1,-1,-1),osg::Vec3f(1,1,-1));
    }
    glPopAttrib();

    static bool init_menu = true;

    if(init_menu)
    {
        MenuSystem::instance()->addMenuItem(_mvs_menu);
        init_menu = false;
    }
}

OpenThreads::Mutex ScreenMVStencil::PreDrawCallback::mutex;

void ScreenMVStencil::debugStencilBuffer(GLint x, GLint y, GLsizei w, GLsizei h,
        char * file_name) const
{
    unsigned char * buf = (unsigned char *)malloc(w * h);
    glReadPixels(x,y,w,h,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,buf);

    std::ofstream myfile;
    myfile.open(file_name);
    for(unsigned int i = 0; i < w * h; ++i)
    {
        if(i % w == 0)
            myfile << std::endl;
        myfile << (unsigned int)buf[i] << " ";
    }
    free(buf);
    myfile.close();
}

void ScreenMVStencil::PreDrawCallback::operator()(osg::RenderInfo & ri) const
{
    static float fov = 120;	// * 0.5 * M_PI / 180.0;//screen->_fov_dial->getValue();
    //PreDrawCallbacks have no state
    glViewport((GLint)screen->_myInfo->myChannel->left,
            (GLint)screen->_myInfo->myChannel->bottom,
            (GLsizei)screen->_myInfo->myChannel->width,
            (GLsizei)screen->_myInfo->myChannel->height);
    if(render_state == 0)
    {
        /*
         if(osg::DisplaySettings::LEFT_EYE == screen->_stereoMode)
         fov = screen->_fov_dial->getValue();
         */
        CameraOrient cam0, cam1;
        glEnable(GL_STENCIL_TEST);

        ri.getCurrentCamera()->setClearMask(~0);

        //first screen to render, so set up stencil buffer
        glStencilMask(~0);
        glClear(
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //extract position, v_up, v_right, v_view, and left/right plane
        screen->extractLRPlanes(PluginHelper::getHeadMat(0),fov,
                screen->_myInfo->myChannel->width
                        / screen->_myInfo->myChannel->height,cam0);
        screen->cameraToScreenSpace(cam0);

        screen->extractLRPlanes(PluginHelper::getHeadMat(1),fov,
                screen->_myInfo->myChannel->width
                        / screen->_myInfo->myChannel->height,cam1);
        screen->cameraToScreenSpace(cam1);

        //set up stencil buffer and extract section that is shared
        screen->setupZones(cam0,cam1);

        //disable writing to stencil buffer
        glStencilMask(0);

        //each viewer has it's own stencil value
        glStencilFunc(GL_EQUAL,render_state + 1,STENCIL_MASK_ALL);

        //Do not change any stencil value
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
    }
    else if(render_state < screen->_num_render)
    {
        //each real viewer has it's own stencil value
        glStencilFunc(GL_EQUAL,render_state + 1,STENCIL_MASK_ALL);
    }
    else
    {
        //these are screens for interpolated viewers
        glStencilFunc(GL_EQUAL,render_state + 1,STENCIL_MASK_ALL);
    }
    ri.getCurrentCamera()->setClearMask(0);

}
