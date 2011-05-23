/**
 * @file ScreenStereo.h
 */

#ifndef CALVR_MULTI_VIEW_SCREEN_H
#define CALVR_MULTI_VIEW_SCREEN_H

#include <kernel/ScreenBase.h>

#include <osg/DisplaySettings>
#include <osgUtil/SceneView>

#include <vector>
#include <algorithm>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

namespace cvr
{

/**
 * @brief Creates a stereo screen using osg stereo modes
 */
class MultiViewScreen : public ScreenBase
{
    public:

static float EPSILON;
static float T_MAX;
static float T_MIN;
static const GLint MAX_STENCIL_BIT = 255;
static const GLint DEFAULT_STENCIL_BIT = 0;
static const GLuint STENCIL_MASK_ALL = ~0;

#define IS_LEFT(x) ((x) >= 0)
#define IS_RIGHT(x) ((x) <= 0)

inline double degToRad(double deg) const {
    return (deg / 180.0) * M_PI;
}

typedef struct ScreenInfoXZ_t {
    osg::Vec3f top_left, top_right;
    osg::Vec3f bottom_left, bottom_right;
    osg::Vec3f ws_normal;
    osg::Matrix inv_transform; //world space to screen space
} ScreenInfoXZ;

typedef struct CameraOrient_t {
    osg::Vec3f eye, viewDir, up;
    osg::Vec3f leftPlaneNormal, rightPlaneNormal;
} CameraOrient;

typedef struct ScreenBoundary_t {
    osg::Vec3f bl, br, tl, tr;
    float bl_t, br_t, tl_t, tr_t;
} ScreenBoundary;

typedef struct IntersectionPlane_t {
    osg::Vec3f point, dir;
    bool hasIntersected, hit_top, hit_bottom, hit_left, hit_right;
    float t_top,	// intersection at the top of XZ plane
	t_bottom,	// intersection at the bottom of XZ plane
	t_left,		// intersection at the left of the XZ plane
	t_right;	// intersection at the right of the XZ plane
} IntersectionPlane;

bool planePlaneIntersection(const osg::Vec3f &p0, const osg::Vec3f &n0,
	const osg::Vec3f &p1, const osg::Vec3f &n1,
	osg::Vec3f &point, osg::Vec3f &direction) const {
    osg::Vec3f n0_cross_n1 = n0 ^ n1;

    if(fabs(n0_cross_n1.x()) < EPSILON && fabs(n0_cross_n1.y()) < EPSILON &&
	fabs(n0_cross_n1.z()) < EPSILON) {
	return false;
    }

    float d0 = n0 * p0;
    float d1 = n1 * p1;

    float n0_dot_n0 = n0 * n0;
    float n1_dot_n1 = n1 * n1;
    float n0_dot_n1 = n0 * n1;

    float determinant = n0_dot_n0 * n1_dot_n1 - n0_dot_n1 * n0_dot_n1;

    float c0 = (n1_dot_n1 * d0 - n0_dot_n1 * d1) / determinant;
    float c1 = (n0_dot_n0 * d1 - n0_dot_n1 * d0) / determinant;

    point = n0 * c0 + n1 * c1;
    direction = n0_cross_n1;
    direction.normalize();

    return true;
}

bool raySegmentIntersection2D_XZ(const osg::Vec3f &r_o,
	const osg::Vec3f &r_d, const osg::Vec3f &s_p0,
	const osg::Vec3f &s_p1, float &t_hit) const {
    osg::Vec2f ray_o(r_o.x(), r_o.z());
    osg::Vec2f ray_d(r_d.x(), r_d.z());
    osg::Vec2f seg_p0(s_p0.x(), s_p0.z());
    osg::Vec2f seg_p1(s_p1.x(), s_p1.z());

    osg::Vec2f p0_to_p1 = seg_p1 - seg_p0;
    osg::Vec2f normal(-p0_to_p1.y(), p0_to_p1.x());

    /* dot product is operator * */
    float dirDotN = ray_d * normal;

    if(fabs(dirDotN) < EPSILON)
	return false;

    float rODotN = ray_o * normal;
    float pDotN = seg_p0 * normal;

    float time = (-rODotN + pDotN) / dirDotN;

    osg::Vec2f hit = ray_o + ray_d * time;

    if(fabs(p0_to_p1.y()) < EPSILON)
	time = (hit - seg_p0).x() / p0_to_p1.x();
    else if(fabs(p0_to_p1.x()) < EPSILON)
	time = (hit - seg_p0).y() / p0_to_p1.y();
    else
	return false;

    if(time > 1.0 || time < 0.0)
	return false;

    t_hit = time;
    return true;
}

float checkRegion(const osg::Vec3f & a, const osg::Vec3f & b, const
	osg::Vec3f & c) const {
    //cross product allows us to check if a point is right/left of a line
    return (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
}

static bool sort_x_dir(const osg::Vec3f & a, const osg::Vec3f & b) {
    return a.x() < b.x();
}

static bool sort_y_dir(const osg::Vec3f & a, const osg::Vec3f & b) {
    return a.y() < b.y();
}

static void sortToXZ(std::vector<osg::Vec3f> & list) {
    std::vector<osg::Vec3f> left_list, right_list, top_list, bottom_list;
    osg::Vec3f tmp;

    for(int i = 0; i < list.size(); ++i) {
	tmp = list.at(i);
	if(fabs(tmp.x() - T_MIN) < EPSILON) {
	    left_list.push_back(tmp);
	} else if(fabs(tmp.y() - T_MIN) < EPSILON) {
	    bottom_list.push_back(tmp);
	} else if(fabs(tmp.x() - T_MAX) < EPSILON) {
	    right_list.push_back(tmp);
	} else if(fabs(tmp.y() - T_MAX) < EPSILON) {
	    top_list.push_back(tmp);
	}
    }

    sort(left_list.begin(), left_list.end(), MultiViewScreen::sort_y_dir);
    sort(right_list.begin(), right_list.end(), MultiViewScreen::sort_y_dir);
    sort(top_list.begin(), top_list.end(), MultiViewScreen::sort_x_dir);
    sort(bottom_list.begin(), bottom_list.end(), MultiViewScreen::sort_x_dir);

    list.clear();
    list.insert(list.end(), left_list.begin(), left_list.end());
    list.insert(list.end(), bottom_list.begin(), bottom_list.end());
    list.insert(list.end(), right_list.begin(), right_list.end());
    list.insert(list.end(), top_list.begin(), top_list.end());
}

void extractViewerMat(const osg::Matrixd &viewerMat, osg::Vec3f &eye,
	osg::Vec3f &viewDir, osg::Vec3f &up) const {
    eye = viewerMat.getTrans();

    osg::Vec4 viewDir4 = osg::Vec4(0, 1, 0, 0) * viewerMat;
    osg::Vec4 upDir4 = osg::Vec4(0, 0, 1, 0) * viewerMat;

    viewDir = osg::Vec3f(viewDir4.x(), viewDir4.y(), viewDir4.z());
    up = osg::Vec3f(upDir4.x(), upDir4.y(), upDir4.z());

    viewDir.normalize();
    up.normalize();
}

// extract the left and right plane of view frustum
// lighthouse 3d view frustum geometric approach
void extractLRPlanes(const osg::Matrixd &viewerMat, float fov,
	float ratio, CameraOrient &cam) const {
    float farDist = 10000;

    extractViewerMat(viewerMat, cam.eye, cam.viewDir, cam.up);

    float Hfar = 2 * tan(fov / 2.0) * farDist;
    float Wfar = Hfar * ratio;

    osg::Vec3f vRight = cam.viewDir ^ cam.up;

    osg::Vec3f fc = cam.eye + cam.viewDir * farDist;

    osg::Vec3f ftl = fc + (cam.up * Hfar / 2.0) - (vRight * Wfar / 2.0);
    osg::Vec3f fbl = fc - (cam.up * Hfar / 2.0) - (vRight * Wfar / 2.0);

    osg::Vec3f ftr = fc + (cam.up * Hfar / 2.0) + (vRight * Wfar / 2.0);
    osg::Vec3f fbr = fc - (cam.up * Hfar / 2.0) + (vRight * Wfar / 2.0);

    cam.leftPlaneNormal = (fbl - cam.eye) ^ (ftl - cam.eye);
    cam.leftPlaneNormal.normalize();

    cam.rightPlaneNormal = (ftr - cam.eye) ^ (fbr - cam.eye);
    cam.rightPlaneNormal.normalize();
}

void fullscreenQuad(const ScreenBoundary &sb) const {
    GLint currentMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &currentMatrixMode);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glVertex3f(sb.bl.x(), sb.bl.y(), sb.bl.z());
    glVertex3f(sb.br.x(), sb.br.y(), sb.br.z());
    glVertex3f(sb.tr.x(), sb.tr.y(), sb.tr.z());
    glVertex3f(sb.tl.x(), sb.tl.y(), sb.tl.z());
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(currentMatrixMode);
}

void fullscreenQuad(const osg::Vec3f &p0, const osg::Vec3f &p1,
	const osg::Vec3f &p2, const osg::Vec3f &p3) const {
    GLint currentMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &currentMatrixMode);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glVertex3f(p0.x(), p0.y(), p0.z());
    glVertex3f(p1.x(), p1.y(), p1.z());
    glVertex3f(p2.x(), p2.y(), p2.z());
    glVertex3f(p3.x(), p3.y(), p3.z());
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(currentMatrixMode);
}

void fullscreenTriangle(const osg::Vec3f &p0, const osg::Vec3f &p1,
	const osg::Vec3f &p2) const {
    GLint currentMatrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &currentMatrixMode);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glVertex3f(p0.x(), p0.y(), p0.z());
    glVertex3f(p1.x(), p1.y(), p1.z());
    glVertex3f(p2.x(), p2.y(), p2.z());
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(currentMatrixMode);
}
        MultiViewScreen();
        virtual ~MultiViewScreen();

        /**
         * @brief Receives callbacks from osg render for view and projection matrices
         */
        struct StereoCallback : public osgUtil::SceneView::ComputeStereoMatricesCallback
        {
                virtual osg::Matrixd
                        computeLeftEyeProjection(const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                        computeLeftEyeView(const osg::Matrixd &view) const;
                virtual osg::Matrixd
                        computeRightEyeProjection(
                                                  const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                        computeRightEyeView(const osg::Matrixd &view) const;
                osg::Matrix _viewLeft; ///< left eye view matrix
                osg::Matrix _viewRight; ///< right eye view matrix
                osg::Matrix _projLeft; ///< left eye projection matrix
                osg::Matrix _projRight; ///< right eye projection matrix
        };

        struct PreDrawCallback : public osg::Camera::DrawCallback {
            MultiViewScreen * screen;
	    unsigned int render_state;
            std::string vertShader;
            std::string fragShader;
            std::vector<osg::Camera * > * cameraList;
            std::vector<StereoCallback * > * scList;

            virtual void operator()(osg::RenderInfo & ri) const;
            static OpenThreads::Mutex mutex;
        };

        /**
         * @brief Create stereo screen for viewer
         * @param viewer viewer to create screen for
         * @param screenInfo parameters for screen
         * @param mode osg stereo mode to use from enum osg::DisplaySettings::StereoMode
         */
        virtual void init(int mode = 0);

        /**
         * @brief Recompute the view and projection matrices for screen
         */
        virtual void computeViewProj();

        /**
         * @brief update osg::Cameras with current view/proj matrices
         */
        virtual void updateCamera();

        /**
         * @brief set the opengl clear color
         */
        virtual void setClearColor(osg::Vec4 color);

        /**
         * @brief find if this screen contains this camera
         */
        virtual ScreenInfo * findScreenInfo(osg::Camera * c);

    protected:
	void computeScreenInfoXZ();
    	void createCameras(unsigned int quantity);
    	bool computeIntersections(IntersectionPlane & plane) const;
    	void computeBoundary(ScreenBoundary & sb) const;
    	bool checkIntersection(IntersectionPlane & c0_l,
		IntersectionPlane & c0_r, IntersectionPlane & c1_l,
		IntersectionPlane & c1_r) const;
	void stencilOutView(const IntersectionPlane & l_p,
		const IntersectionPlane & r_p, GLint ref, GLuint mask,
		GLuint write_mask) const;
	void setupZones(CameraOrient & cam0,
		CameraOrient & cam1, IntersectionPlane & c0_l,
		IntersectionPlane & c0_r, IntersectionPlane & c1_l,
		IntersectionPlane & c1_r, bool & hasIntersected) const;

        osg::DisplaySettings::StereoMode _stereoMode;
	ScreenInfoXZ * _screenInfoXZ;
	float _fov;
        unsigned int _num_render;
        std::vector< osg::Camera * > _cameras;
        std::vector<PreDrawCallback * > _preCallbacks;
        std::vector<StereoCallback * > _stereoCallbacks;
};
}

#endif
