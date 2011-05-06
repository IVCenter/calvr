#include <kernel/MultiViewScreen.h>
#include <kernel/CVRCullVisitor.h>
#include <kernel/CVRViewer.h>
#include <kernel/NodeMask.h>
#include <kernel/PluginHelper.h>
#include <config/ConfigManager.h>
#include <input/TrackingManager.h>
#include <util/OsgPrint.h>

#include <osgViewer/Renderer>

#include <iostream>
using namespace cvr;

MultiViewScreen::MultiViewScreen() : ScreenBase()
{
}

MultiViewScreen::~MultiViewScreen()
{
}

void MultiViewScreen::init(int mode)
{
    //this is passed in from ScreenConfig
    //creates MultiViewScreen() for each eye in StarCave
    _stereoMode = (osg::DisplaySettings::StereoMode)mode;

    computeScreenInfoXZ();

    //field of view for all cameras
    _fov = 45;

    //number of times to render per eye
    _num_render = 3;
    createCameras(_num_render);
}

void MultiViewScreen::computeScreenInfoXZ() {
    _screenInfoXZ = (ScreenInfoXZ *) malloc(sizeof(ScreenInfoXZ));

    //center is at (0,0,0) initially
    _screenInfoXZ->ws_normal = osg::Vec3f(0,-1,0) * _myInfo->transform;

    //this is the actual screen's width and height measurements
    double h2 = _myInfo->height / 2,
	    w2 = _myInfo->width / 2;

    _screenInfoXZ->top_left = osg::Vec3f(-w2, 0, h2);
    _screenInfoXZ->top_right = osg::Vec3f(w2, 0, h2);
    _screenInfoXZ->bottom_left = osg::Vec3f(-w2, 0, -h2);
    _screenInfoXZ->bottom_right = osg::Vec3f(w2, 0, -h2);
    _screenInfoXZ->inv_transform =
	osg::Matrixd::inverse(_myInfo->transform);
}

void MultiViewScreen::createCameras(unsigned int quantity) {
    std::string shaderDir;
    char * cvrHome = getenv("CALVR_HOME");
    if(cvrHome) {
	shaderDir = cvrHome;
	shaderDir = shaderDir + "/";
    }
    shaderDir = shaderDir + "shaders/";
    for(unsigned int i = 0; i < quantity; ++i) {
	osg::Camera * cam = new osg::Camera;
	_cameras.push_back(cam);
        CVRViewer::instance()->addSlave(cam, osg::Matrix(), osg::Matrix());
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

        osgViewer::Renderer * renderer = dynamic_cast<osgViewer::Renderer*> (cam->getRenderer());
	//both scene 0 and 1 will be the same function
	osg::DisplaySettings * ds = renderer->getSceneView(0)->getDisplaySettings();
	//set to false for StarCave.  When false, the StereoBallback does
	//not matter.  True is for NexCave.
	ds->setStereo(_stereoMode == osg::DisplaySettings::HORIZONTAL_INTERLACE);
	ds->setStereoMode(_stereoMode);

	StereoCallback * sc = new StereoCallback;
	_stereoCallbacks.push_back(sc);

	renderer->getSceneView(0)->setComputeStereoMatricesCallback(sc);
	renderer->getSceneView(1)->setComputeStereoMatricesCallback(sc);
    }
}

void MultiViewScreen::computeViewProj() {
    std::vector<osg::Vec3> iLeft;
    std::vector<osg::Vec3> iRight;
    iLeft.push_back(defaultLeftEye(0));
    iLeft.push_back(defaultLeftEye(1));
    iLeft.push_back((iLeft.at(0) + iLeft.at(1)) * 0.5);
    iRight.push_back(defaultRightEye(0));
    iRight.push_back(defaultRightEye(1));
    iRight.push_back((iRight.at(0) + iRight.at(1)) * 0.5);

    for(unsigned int i = 0; i < _cameras.size(); ++i) {
	StereoCallback * sc = _stereoCallbacks.at(i);
        computeDefaultViewProj(iLeft.at(i), sc->_viewLeft, sc->_projLeft);
        computeDefaultViewProj(iRight.at(i), sc->_viewRight, sc->_projRight);
    }
}

void MultiViewScreen::updateCamera() {
    if(osg::DisplaySettings::HORIZONTAL_INTERLACE == _stereoMode)
	return;

    for(unsigned int i = 0; i < _cameras.size(); ++i) {
	if(osg::DisplaySettings::LEFT_EYE == _stereoMode) {
	    _cameras[i]->setViewMatrix(_stereoCallbacks[i]->_viewLeft);
	    _cameras[i]->setProjectionMatrix(_stereoCallbacks[i]->_projLeft);
	} else {
	    _cameras[i]->setViewMatrix(_stereoCallbacks[i]->_viewRight);
	    _cameras[i]->setProjectionMatrix(_stereoCallbacks[i]->_projRight);
	}
    }
}

osg::Matrixd MultiViewScreen::StereoCallback::computeLeftEyeProjection(
                                                                    const osg::Matrixd &projection) const
{
    return _projLeft;
}

osg::Matrixd MultiViewScreen::StereoCallback::computeLeftEyeView(
                                                              const osg::Matrixd &view) const
{
    return _viewLeft;
}

osg::Matrixd MultiViewScreen::StereoCallback::computeRightEyeProjection(
                                                                     const osg::Matrixd &projection) const
{
    return _projRight;
}

osg::Matrixd MultiViewScreen::StereoCallback::computeRightEyeView(
                                                               const osg::Matrixd &view) const
{
    return _viewRight;
}

void MultiViewScreen::setClearColor(osg::Vec4 color)
{
    for (int i = 0; i < _cameras.size(); i++)
        _cameras[i]->setClearColor(color);
}

ScreenInfo * MultiViewScreen::findScreenInfo(osg::Camera * c)
{
    for (unsigned int i = 0; i < _cameras.size(); ++i)
    {
        if(c == _cameras[i])
        {
            return _myInfo;
        }
    }
    return NULL;
}

bool MultiViewScreen::computeIntersections(IntersectionPlane & plane)
	const {
    osg::Vec3f xz_point = plane.point * _screenInfoXZ->inv_transform;
    osg::Vec3f xz_dir = plane.dir * _screenInfoXZ->inv_transform;
    xz_dir.normalize();

    plane.hit_top = raySegmentIntersection2D_XZ(xz_point, xz_dir,
	_screenInfoXZ->top_left, _screenInfoXZ->top_right,
	plane.t_top);
    plane.hit_bottom = raySegmentIntersection2D_XZ(xz_point, xz_dir,
	_screenInfoXZ->bottom_left, _screenInfoXZ->bottom_right,
	plane.t_bottom);

    plane.hit_left = raySegmentIntersection2D_XZ(xz_point, xz_dir,
	_screenInfoXZ->top_left, _screenInfoXZ->bottom_left,
	plane.t_left);

    plane.hit_right = raySegmentIntersection2D_XZ(xz_point, xz_dir,
	_screenInfoXZ->top_right, _screenInfoXZ->bottom_right,
	plane.t_right);

    // if hits corner of screen, make it as if it hits the top of the
    // screen for better performance
    if(plane.hit_left) {
	if(plane.hit_top && fabs(plane.t_top - plane.t_left) < EPSILON)
	    plane.hit_left = false;
	else if(plane.hit_bottom &&
		fabs(plane.t_bottom - plane.t_left) < EPSILON)
	    plane.hit_left = false;
    }

    if(plane.hit_right) {
	if(plane.hit_top && fabs(plane.t_top - plane.t_right) < EPSILON)
	    plane.hit_right = false;
	else if(plane.hit_bottom &&
		fabs(plane.t_bottom - plane.t_right) < EPSILON)
	    plane.hit_right = false;
    }

    plane.t_top = plane.t_top * 2.0 - 1.0;
    plane.t_bottom = plane.t_bottom * 2.0 - 1.0;
    plane.t_left = plane.t_left * 2.0 - 1.0;
    plane.t_right = plane.t_right * 2.0 - 1.0;

    return true;
}

//simply sets info.  Avoid copying this code a bunch of times
void MultiViewScreen::computeBoundary(ScreenBoundary & sb) const {
    sb.bl = osg::Vec3f(sb.bl_t, -1.0, -1.0);
    sb.br = osg::Vec3f(sb.br_t, -1.0, -1.0);
    sb.tl = osg::Vec3f(sb.tl_t, 1.0, -1.0);
    sb.tr = osg::Vec3f(sb.tr_t, 1.0, -1.0);
}

// may change intersection method later on.  This is used to check
// if view frustums are overlapping.
bool MultiViewScreen::checkIntersection(IntersectionPlane & c0_l,
	IntersectionPlane & c0_r, IntersectionPlane & c1_l,
	IntersectionPlane & c1_r) const {
    bool is_in_between =
	(c0_l.t_top <= c1_l.t_top && c1_r.t_top <= c0_r.t_top) ||
	(c1_l.t_top <= c0_l.t_top && c0_r.t_top <= c1_r.t_top);
    bool is_outside = c0_r.t_top <= c1_l.t_top || c1_r.t_top <= c0_l.t_top;
    return !(is_in_between || is_outside);
}

void MultiViewScreen::stencilOutView(const IntersectionPlane & l_p,
	const IntersectionPlane & r_p, GLint ref, GLuint mask,
	GLuint write_mask) const {
    std::vector<osg::Vec3f> list_of_points;
    bool left_is_offscreen = l_p.hit_top == false &&
	    l_p.hit_bottom == false &&
	    l_p.hit_left == false && l_p.hit_right == false;
    bool right_is_offscreen = r_p.hit_top == false &&
	    r_p.hit_bottom == false &&
	    r_p.hit_left == false && r_p.hit_right == false;
    std::vector<osg::Vec3f> list_xz_corners, lp_points, rp_points;
    list_xz_corners.push_back(osg::Vec3f(T_MIN, T_MIN, T_MIN));
    list_xz_corners.push_back(osg::Vec3f(T_MIN, T_MAX, T_MIN));
    list_xz_corners.push_back(osg::Vec3f(T_MAX, T_MIN, T_MIN));
    list_xz_corners.push_back(osg::Vec3f(T_MAX, T_MAX, T_MIN));

    //add all hit points into a list
    if(l_p.hit_left) {
	list_of_points.push_back(osg::Vec3f(T_MIN, l_p.t_left, T_MIN));
	lp_points.push_back(osg::Vec3f(T_MIN, l_p.t_left, T_MIN));
    }

    if(l_p.hit_right) {
	list_of_points.push_back(osg::Vec3f(T_MAX, l_p.t_right, T_MIN));
	lp_points.push_back(osg::Vec3f(T_MAX, l_p.t_right, T_MIN));
    }

    if(l_p.hit_top) {
	list_of_points.push_back(osg::Vec3f(l_p.t_top, T_MAX, T_MIN));
	lp_points.push_back(osg::Vec3f(l_p.t_top, T_MAX, T_MIN));
    }

    if(l_p.hit_bottom) {
	list_of_points.push_back(osg::Vec3f(l_p.t_bottom, T_MIN, T_MIN));
	lp_points.push_back(osg::Vec3f(l_p.t_bottom, T_MIN, T_MIN));
    }

    if(r_p.hit_left) {
	list_of_points.push_back(osg::Vec3f(T_MIN, r_p.t_left, T_MIN));
	rp_points.push_back(osg::Vec3f(T_MIN, r_p.t_left, T_MIN));
    }

    if(r_p.hit_right) {
	list_of_points.push_back(osg::Vec3f(T_MAX, r_p.t_right, T_MIN));
	rp_points.push_back(osg::Vec3f(T_MAX, r_p.t_right, T_MIN));
    }

    if(r_p.hit_top) {
	list_of_points.push_back(osg::Vec3f(r_p.t_top, T_MAX, T_MIN));
	rp_points.push_back(osg::Vec3f(r_p.t_top, T_MAX, T_MIN));
    }

    if(r_p.hit_bottom) {
	list_of_points.push_back(osg::Vec3f(r_p.t_bottom, T_MIN, T_MIN));
	rp_points.push_back(osg::Vec3f(r_p.t_bottom, T_MIN, T_MIN));
    }

    /*
     * Three ways to intersect XZ plane
     * Case 1: left and right plane hits only the top and bottom XZ.
     *         This case has only 4 hit points.  Already taken care of
     *         through adding all points.
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
    if(right_is_offscreen) {
	for(int i = 0; i < list_xz_corners.size(); ++i) {
	    if(IS_RIGHT(checkRegion(lp_points.at(0), lp_points.at(1),
		    list_xz_corners.at(i))))
		list_of_points.push_back(list_xz_corners.at(i));
	}
    } else if(left_is_offscreen) {
	for(int i = 0; i < list_xz_corners.size(); ++i) {
	    if(IS_LEFT(checkRegion(rp_points.at(0), rp_points.at(1),
		    list_xz_corners.at(i))))
		list_of_points.push_back(list_xz_corners.at(i));
	}
    } else {
	for(int i = 0; i < list_xz_corners.size(); ++i) {
	    if(IS_RIGHT(checkRegion(lp_points.at(0), lp_points.at(1),
		    list_xz_corners.at(i))) &&
		    IS_LEFT(checkRegion(rp_points.at(0), rp_points.at(1),
		    list_xz_corners.at(i))))
		list_of_points.push_back(list_xz_corners.at(i));
	}
    }
    MultiViewScreen::sortToXZ(list_of_points);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_EQUAL, ref, mask);
    glStencilMask(write_mask);

    if(list_of_points.size() == 3) {
	fullscreenTriangle(list_of_points.at(0), list_of_points.at(1),
	    list_of_points.at(2));
    } else if(list_of_points.size() == 4) {
	fullscreenQuad(list_of_points.at(0), list_of_points.at(1),
	    list_of_points.at(2), list_of_points.at(3));
    } else if(list_of_points.size() == 5) {
	fullscreenTriangle(list_of_points.at(0), list_of_points.at(1),
	    list_of_points.at(2));
	fullscreenQuad(list_of_points.at(2), list_of_points.at(3),
	    list_of_points.at(4), list_of_points.at(0));
    } else if(list_of_points.size() == 6) {
	fullscreenQuad(list_of_points.at(0), list_of_points.at(1),
	    list_of_points.at(2), list_of_points.at(3));
	fullscreenQuad(list_of_points.at(3), list_of_points.at(4),
	    list_of_points.at(5), list_of_points.at(0));
    }
}

// set up stencil buffer into 3 zones (due to 2 views)
// 0 - no one is looking there
// 1 - one person is looking there
// 2 - other person is looking there
// 3 - both of them have views overlapping
void MultiViewScreen::setupZones(CameraOrient & cam0,
	CameraOrient & cam1, IntersectionPlane & c0_l,
	IntersectionPlane & c0_r, IntersectionPlane & c1_l,
	IntersectionPlane & c1_r, bool & hasIntersected) const {
    bool do_intersection = false;
    c0_l.hasIntersected = planePlaneIntersection(cam0.eye,
	    cam0.leftPlaneNormal, _myInfo->xyz,
	    _screenInfoXZ->ws_normal, c0_l.point, c0_l.dir);

    c0_r.hasIntersected = planePlaneIntersection(cam0.eye,
	    cam0.rightPlaneNormal, _myInfo->xyz,
	    _screenInfoXZ->ws_normal, c0_r.point, c0_r.dir);

    c1_l.hasIntersected = planePlaneIntersection(cam1.eye,
	    cam1.leftPlaneNormal, _myInfo->xyz,
	    _screenInfoXZ->ws_normal, c1_l.point, c1_l.dir);

    c1_r.hasIntersected = planePlaneIntersection(cam1.eye,
	    cam1.rightPlaneNormal, _myInfo->xyz,
	    _screenInfoXZ->ws_normal, c1_r.point, c1_r.dir);

    if(c0_l.hasIntersected && c0_r.hasIntersected) {
	do_intersection = computeIntersections(c0_l) &&
	    computeIntersections(c0_r);
    } else if(c0_l.hasIntersected) {
	do_intersection = computeIntersections(c0_l);
	c0_r.t_top = c0_r.t_bottom = 1.0;
    } else if(c0_r.hasIntersected) {
	do_intersection = computeIntersections(c0_r);
	c0_l.t_top = c0_l.t_bottom = -1.0;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColorMask(0,0,0,0);

    if(do_intersection) {
	stencilOutView(c0_l, c0_r, 1, 0, ~0);
    }

    if(c1_l.hasIntersected && c1_r.hasIntersected) {
	do_intersection = computeIntersections(c1_l) &&
	    computeIntersections(c1_r);
    } else if(c1_l.hasIntersected) {
	do_intersection = computeIntersections(c1_l);
	c1_r.t_top = c1_r.t_bottom = 1.0;
    } else if(c1_r.hasIntersected) {
	do_intersection = computeIntersections(c1_r);
	c1_l.t_top = c1_l.t_bottom = -1.0;
    }

    if(do_intersection) {
	stencilOutView(c1_l, c1_r, 2, 0, 0x1);
    }
    hasIntersected = checkIntersection(c0_l, c0_r, c1_l, c1_r);
    glPopAttrib();
    glColorMask(1,1,1,1);
}

OpenThreads::Mutex MultiViewScreen::PreDrawCallback::mutex;

void MultiViewScreen::PreDrawCallback::operator()(osg::RenderInfo & ri)
	const {
    static IntersectionPlane c0_l, c0_r, c1_l, c1_r;
    static bool hasIntersected;

    if(render_state == 0) {
	CameraOrient cam0, cam1;

	ri.getCurrentCamera()->setClearMask(~0);

	//first screen to render, so set up stencil buffer
	glStencilMask(~0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	    GL_STENCIL_BUFFER_BIT);

	//extract position, v_up, v_right, v_view, and left/right plane
	if(screen->_stereoMode == osg::DisplaySettings::LEFT_EYE) {
	    screen->extractLRPlanes((*scList)[0]->_viewLeft, screen->_fov,
		screen->_myInfo->myChannel->width /
		screen->_myInfo->myChannel->height, cam0);
	    screen->extractLRPlanes((*scList)[1]->_viewLeft, screen->_fov,
		screen->_myInfo->myChannel->width /
		screen->_myInfo->myChannel->height, cam1);
	} else {
	    screen->extractLRPlanes((*scList)[0]->_viewRight, screen->_fov,
		screen->_myInfo->myChannel->width /
		screen->_myInfo->myChannel->height, cam0);
	    screen->extractLRPlanes((*scList)[1]->_viewRight, screen->_fov,
		screen->_myInfo->myChannel->width /
		screen->_myInfo->myChannel->height, cam1);
	}
	    print(cam0.eye);
	    print(cam1.eye);
	    print(cam0.viewDir);
	    print(cam1.viewDir);
	    print(cam0.up);
	    print(cam1.up);
	    print(cam0.leftPlaneNormal);
	    print(cam0.rightPlaneNormal);
	    print(cam1.leftPlaneNormal);
	    print(cam1.rightPlaneNormal);


exit(0);
	//set up stencil buffer and extract section that is shared
	screen->setupZones(cam0, cam1, c0_l, c0_r, c1_l, c1_r, hasIntersected);

	//disable writing to stencil buffer
	glStencilMask(0);

	//each viewer has it's own stencil value
	glStencilFunc(GL_EQUAL, render_state + 1, STENCIL_MASK_ALL);

	//Do not change any stencil value
	//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    } else if(render_state < screen->_num_render) {
	//these are screens for real viewers

	//each viewer has it's own stencil value
	glStencilFunc(GL_EQUAL, render_state + 1, STENCIL_MASK_ALL);
    } else {
	//these are screens for interpolated viewers
	glStencilFunc(GL_EQUAL, render_state + 1, STENCIL_MASK_ALL);
    }

    ri.getCurrentCamera()->setClearMask(0);
}
