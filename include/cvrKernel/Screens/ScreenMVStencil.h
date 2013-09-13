/**
 * @file ScreenMVStencil.h
 */

#ifndef CALVR_SCREEN_MV_STENCIL_H
#define CALVR_SCREEN_MV_STENCIL_H

#include <cvrKernel/Screens/ScreenBase.h>

#include <osg/DisplaySettings>
#include <osgUtil/SceneView>
#include <cvrMenu/MenuItem.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/SubMenu.h>
#include <cvrMenu/MenuRangeValue.h>

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

namespace cvr
{

/**
 * @addtogroup screens
 * @{
 */

/**
 * @brief Creates a stereo screen using osg stereo modes
 */
class ScreenMVStencil : public ScreenBase
{
    public:
#define k_Top_Right 0
#define k_Top_Left 1
#define k_Bottom_Left 2
#define k_Bottom_Right 3

        static const float EPSILON;
        static const float T_MAX;
        static const float T_MIN;
        static const GLint MAX_STENCIL_BIT = 255;
        static const GLint DEFAULT_STENCIL_BIT = 0;
        static const GLuint STENCIL_MASK_ALL = ~0;

        inline double degToRad(double deg) const
        {
            return (deg / 180.0) * M_PI;
        }

        typedef struct ScreenInfoXZ_t
        {
                osg::Vec3f top_left, top_right;
                osg::Vec3f bottom_left, bottom_right;
                osg::Vec3f normal;
                osg::Matrix inv_transform; //world space to screen space
        } ScreenInfoXZ;

        typedef struct CameraOrient_t
        {
                osg::Vec3f eye, viewDir, up;
                osg::Vec3f leftPlaneNormal, rightPlaneNormal;
                osg::Vec3f leftPlanePoint, rightPlanePoint;
        } CameraOrient;

        typedef struct IntersectionPlane_t
        {
                osg::Vec3f point, dir;
                bool hasIntersected, hit_top, hit_bottom, hit_left, hit_right;
                float t_top,	// intersection at the top of XZ plane
                        t_bottom,	// intersection at the bottom of XZ plane
                        t_left,		// intersection at the left of the XZ plane
                        t_right;	// intersection at the right of the XZ plane
        } IntersectionPlane;

        bool LinePlaneIntersection(const osg::Vec3f &line_point_start,
                const osg::Vec3f &line_point_end,
                const osg::Vec3f &point_on_plane,
                const osg::Vec3f &normal_of_plane, osg::Vec3f &i_point,
                double &t_hit) const
        {
            float denom = normal_of_plane * (line_point_end - line_point_start);

            if(fabs(denom) < EPSILON)
                return false;

            t_hit = normal_of_plane * (point_on_plane - line_point_start)
                    / denom;

            i_point = line_point_start
                    + (line_point_end - line_point_start) * t_hit;

            return true;
        }

        bool planePlaneIntersection(const osg::Vec3f &p0, const osg::Vec3f &n0,
                const osg::Vec3f &p1, const osg::Vec3f &n1, osg::Vec3f &point,
                osg::Vec3f &direction) const
        {
            osg::Vec3f n0_cross_n1 = n0 ^ n1;

            if(fabs(n0_cross_n1.x()) < EPSILON
                    && fabs(n0_cross_n1.y()) < EPSILON
                    && fabs(n0_cross_n1.z()) < EPSILON)
            {
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

//r_d is the direction of the line.  Either direction is fine
//because the goal is to intersect against the segment.
        bool lineSegmentIntersection2D_XZ(const osg::Vec3f &r_o,
                const osg::Vec3f &r_d, const osg::Vec3f &s_p0,
                const osg::Vec3f &s_p1, float &t_hit) const
        {
            osg::Vec2f ray_o(r_o.x(),r_o.z());
            osg::Vec2f ray_d(r_d.x(),r_d.z());
            osg::Vec2f seg_p0(s_p0.x(),s_p0.z());
            osg::Vec2f seg_p1(s_p1.x(),s_p1.z());

            osg::Vec2f p0_to_p1 = seg_p1 - seg_p0;
            osg::Vec2f normal(-p0_to_p1.y(),p0_to_p1.x());

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

            //must be within segment length
            if(time > 1.0 || time < 0.0)
                return false;

            t_hit = time;
            return true;
        }

        static bool IsLeftOfLine(const osg::Vec3f & a, const osg::Vec3f & b,
                const osg::Vec3f & c)
        {
            //cross product allows us to check if a point is right/left of a line
            return ((b.x() - a.x()) * (c.y() - a.y())
                    - (b.y() - a.y()) * (c.x() - a.x())) > 0;
        }

        static bool IsRightOfLine(const osg::Vec3f & a, const osg::Vec3f & b,
                const osg::Vec3f & c)
        {
            return !IsLeftOfLine(a,b,c);
        }

//In general, a point p is inside a plane if
//n_x * x + n_y * y + n_z * z + d = 0 with (n_x, n_y, n_z) pointing inward
//n_x * p_x + n_y * p_y + n_z * p_z + d > 0
//Inside can be thought of inside the camera frustum
        static bool IsInsideOfPlane(const osg::Vec3f plane_position,
                const osg::Vec3f plane_normal, const osg::Vec3f point)
        {
            double d = -(plane_position * plane_normal);
            return ((plane_normal * point) + d) > 0.0;
        }

        static double toPolarStartRight(const osg::Vec3f &v)
        {
            //default is quadrant one
            double ret = atan(v.y() / v.x()) * 180.0 / osg::PI;

            if(v.x() < 0 && v.y() >= 0)
                ret += 180.0;
            else if(v.x() < 0 && v.y() < 0)
                ret += 180.0;
            else if(v.x() > 0 && v.y() < 0)
                ret += 360.0;

            return ret;
        }

        static double toPolarStartLeft(const osg::Vec3f & v)
        {
            double t0, t1, t3, t4;

            t3 = fabs(v.x());
            t1 = fabs(v.y());
            t0 = std::max(t3,t1);
            t1 = std::min(t3,t1);
            t3 = 1.0 / t0;
            t3 = t1 * t3;

            t4 = t3 * t3;
            t0 = -0.013480470;
            t0 = t0 * t4 + 0.057477314;
            t0 = t0 * t4 - 0.121239071;
            t0 = t0 * t4 + 0.195635925;
            t0 = t0 * t4 - 0.332994597;
            t0 = t0 * t4 + 0.999995630;
            t3 = t0 * t3;

            if(fabs(v.y()) > fabs(v.x()))
                t3 = 1.570796327 - t3;

            if(v.x() < 0)
                t3 = 3.141592654 - t3;

            if(v.y() < 0)
                t3 = -t3;

            return t3;
        }

        void IsInLeftView(const IntersectionPlane & p,
                std::vector<int> & list_p) const
        {
            if(p.hit_top)
            {
                if(p.hit_left)
                {
                    list_p.push_back(k_Top_Right);
                    list_p.push_back(k_Bottom_Left);
                    list_p.push_back(k_Bottom_Right);
                }
                else if(p.hit_bottom)
                {
                    list_p.push_back(k_Top_Right);
                    list_p.push_back(k_Bottom_Right);
                }
                else if(p.hit_right)
                {
                    list_p.push_back(k_Top_Right);
                }
            }
            else if(p.hit_bottom)
            {
                if(p.hit_left)
                {
                    list_p.push_back(k_Top_Right);
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Bottom_Right);
                }
                else if(p.hit_right)
                {
                    list_p.push_back(k_Bottom_Right);
                }
            }
            else if(p.hit_left && p.hit_right)
            {
                if(p.t_left < p.t_right)
                {
                    list_p.push_back(k_Bottom_Left);
                    list_p.push_back(k_Bottom_Right);
                }
                else
                {
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Top_Right);
                }
            }
        }

        void IsInRightView(const IntersectionPlane & p,
                std::vector<int> & list_p) const
        {
            if(p.hit_top)
            {
                if(p.hit_right)
                {
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Bottom_Left);
                    list_p.push_back(k_Bottom_Right);
                }
                else if(p.hit_bottom)
                {
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Bottom_Left);
                }
                else if(p.hit_left)
                {
                    list_p.push_back(k_Top_Left);
                }
            }
            else if(p.hit_bottom)
            {
                if(p.hit_right)
                {
                    list_p.push_back(k_Top_Right);
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Bottom_Left);
                }
                else if(p.hit_left)
                {
                    list_p.push_back(k_Bottom_Left);
                }
            }
            else if(p.hit_left && p.hit_right)
            {
                if(p.t_left > p.t_right)
                {
                    list_p.push_back(k_Bottom_Left);
                    list_p.push_back(k_Bottom_Right);
                }
                else
                {
                    list_p.push_back(k_Top_Left);
                    list_p.push_back(k_Top_Right);
                }
            }
        }

        void AddScreenCorners(std::vector<int> & list_p,
                std::vector<osg::Vec3f> & list_points) const
        {
            std::vector<int>::iterator it = list_p.begin();

            for(; it < list_p.end(); ++it)
            {
                switch(*it)
                {
                    case k_Top_Right:
                    {
                        list_points.push_back(osg::Vec3f(T_MAX,T_MAX,T_MIN));
                        break;
                    }
                    case k_Top_Left:
                    {
                        list_points.push_back(osg::Vec3f(T_MIN,T_MAX,T_MIN));
                        break;
                    }
                    case k_Bottom_Left:
                    {
                        list_points.push_back(osg::Vec3f(T_MIN,T_MIN,T_MIN));
                        break;
                    }
                    case k_Bottom_Right:
                    {
                        list_points.push_back(osg::Vec3f(T_MAX,T_MIN,T_MIN));
                        break;
                    }
                    default:
                    {
                        std::cout << "Error: addScreenCorners" << std::endl;
                        break;
                    }
                }
            }
        }

        static bool sort_by_int(const int & a, const int & b)
        {
            return a < b;
        }

        static bool sort_by_polar_start_left(const osg::Vec3f & a,
                const osg::Vec3f b)
        {
            return ScreenMVStencil::toPolarStartLeft(a)
                    < ScreenMVStencil::toPolarStartLeft(b);
        }

        static bool sort_by_polar_start_right(const osg::Vec3f & a,
                const osg::Vec3f b)
        {
            return ScreenMVStencil::toPolarStartRight(a)
                    < ScreenMVStencil::toPolarStartRight(b);
        }

        static void sortXYPolarStartLeft(std::vector<osg::Vec3f> & list)
        {
            sort(list.begin(),list.end(),
                    ScreenMVStencil::sort_by_polar_start_left);
        }

        static void sortXYPolarStartRight(std::vector<osg::Vec3f> & list)
        {
            sort(list.begin(),list.end(),
                    ScreenMVStencil::sort_by_polar_start_right);
        }

        void extractViewerMat(const osg::Matrixd &viewerMat, osg::Vec3f &eye,
                osg::Vec3f &viewDir, osg::Vec3f &up) const
        {
            //IMPORTANT
            //THE TRANSFORMATION PROVIDED TO THIS FUNCTION _myInfo->transform
            //IS NOT SCALE INVARIANT.
            //Multiply the transform by a osg::Vec3f, which the function will
            //fill in the fourth coordinate as ONE, to retrieve a position.
            //Use that position and the eye coordinate to get the view direction.
            eye = viewerMat.getTrans();

            if(!_align_head->getValue())
            {
                viewDir = osg::Vec3(0,1,0) * viewerMat;
                up = osg::Vec3(0,0,1) * viewerMat;
                viewDir = viewDir - eye;
                up = up - eye;
            }
            else
            {
                viewDir = (osg::Vec3(0,1,0) * viewerMat) - eye;
                viewDir.z() = 0;
                viewDir.normalize();
                up = osg::Vec3f(0,0,1);
            }

            viewDir.normalize();
            up.normalize();
        }

// extract the left and right plane of view frustum
// lighthouse 3d view frustum geometric approach
        void extractLRPlanes(const osg::Matrixd &viewerMat, float fov,
                float ratio, CameraOrient &cam) const
        {
            float farDist = 10000;

            extractViewerMat(viewerMat,cam.eye,cam.viewDir,cam.up);

            float Hfar = 2 * tan(fov) * farDist;
            float Wfar = Hfar * ratio;

            osg::Vec3f vRight = cam.viewDir ^ cam.up;

            osg::Vec3f fc = cam.eye + cam.viewDir * farDist;

            osg::Vec3f ftl = fc + (cam.up * Hfar / 2.0) - (vRight * Wfar / 2.0);
            osg::Vec3f fbl = fc - (cam.up * Hfar / 2.0) - (vRight * Wfar / 2.0);

            osg::Vec3f ftr = fc + (cam.up * Hfar / 2.0) + (vRight * Wfar / 2.0);
            osg::Vec3f fbr = fc - (cam.up * Hfar / 2.0) + (vRight * Wfar / 2.0);

            cam.leftPlaneNormal = (fbl - cam.eye) ^ (ftl - cam.eye);
            cam.leftPlaneNormal.normalize();
            cam.leftPlanePoint = ftl;

            cam.rightPlaneNormal = (ftr - cam.eye) ^ (fbr - cam.eye);
            cam.rightPlaneNormal.normalize();
            cam.rightPlanePoint = ftr;
        }

        void cameraToScreenSpace(CameraOrient & cam) const
        {
            //get camera position in screen space
            osg::Vec3f eye = cam.eye * _screenInfoXZ->inv_transform;

            //IMPORTANT: TRANSFORMATION MATRIX IS NOT SCALE INVARIANT
            //SO REPRESENT ALL VECTORS AS TWO POINTS
            //translate the view direction using points into screen space
            //to be scale invariant
            osg::Vec3f viewDir = (cam.viewDir + cam.eye)
                    * _screenInfoXZ->inv_transform;
            cam.viewDir = viewDir - eye;
            cam.viewDir.normalize();

            //do same thing with up direction
            osg::Vec3f up = (cam.up + cam.eye) * _screenInfoXZ->inv_transform;
            cam.up = up - eye;
            cam.up.normalize();

            //origin of left plane in screen space
            osg::Vec3f leftPlanePoint = cam.leftPlanePoint
                    * _screenInfoXZ->inv_transform;

            //normal vector of plane in screen space
            osg::Vec3f leftPlaneNormal = (cam.leftPlanePoint
                    + cam.leftPlaneNormal) * _screenInfoXZ->inv_transform;
            cam.leftPlaneNormal = leftPlaneNormal - leftPlanePoint;
            cam.leftPlaneNormal.normalize();

            //origin of right plane in screen space
            osg::Vec3f rightPlanePoint = cam.rightPlanePoint
                    * _screenInfoXZ->inv_transform;

            //normal vector of right plane in screen space
            osg::Vec3f rightPlaneNormal = (cam.rightPlanePoint
                    + cam.rightPlaneNormal) * _screenInfoXZ->inv_transform;
            cam.rightPlaneNormal = rightPlaneNormal - rightPlanePoint;
            cam.rightPlaneNormal.normalize();

            //update positions
            cam.eye = eye;
            cam.leftPlanePoint = leftPlanePoint;
            cam.rightPlanePoint = rightPlanePoint;
        }

        void fullscreenQuad(const osg::Vec3f &p0, const osg::Vec3f &p1,
                const osg::Vec3f &p2, const osg::Vec3f &p3) const
        {
            GLint currentMatrixMode;
            glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glBegin(GL_QUADS);
            glVertex3f(p0.x(),p0.y(),p0.z());
            glVertex3f(p1.x(),p1.y(),p1.z());
            glVertex3f(p2.x(),p2.y(),p2.z());
            glVertex3f(p3.x(),p3.y(),p3.z());
            glEnd();
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            glMatrixMode(currentMatrixMode);
        }

        void fullscreenTriangle(const osg::Vec3f &p0, const osg::Vec3f &p1,
                const osg::Vec3f &p2) const
        {
            GLint currentMatrixMode;
            glGetIntegerv(GL_MATRIX_MODE,&currentMatrixMode);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glBegin(GL_TRIANGLES);
            glVertex3f(p0.x(),p0.y(),p0.z());
            glVertex3f(p1.x(),p1.y(),p1.z());
            glVertex3f(p2.x(),p2.y(),p2.z());
            glEnd();
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            glMatrixMode(currentMatrixMode);
        }
        ScreenMVStencil();
        virtual ~ScreenMVStencil();

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
                computeRightEyeProjection(const osg::Matrixd &projection) const;
                virtual osg::Matrixd
                computeRightEyeView(const osg::Matrixd &view) const;
                osg::Matrix _viewLeft; ///< left eye view matrix
                osg::Matrix _viewRight; ///< right eye view matrix
                osg::Matrix _projLeft; ///< left eye projection matrix
                osg::Matrix _projRight; ///< right eye projection matrix
        };

        struct PreDrawCallback : public osg::Camera::DrawCallback
        {
                ScreenMVStencil * screen;
                unsigned int render_state;
                std::string vertShader;
                std::string fragShader;
                std::vector<osg::Camera *> * cameraList;
                std::vector<StereoCallback *> * scList;

                virtual void operator()(osg::RenderInfo & ri) const;
                static OpenThreads::Mutex mutex;
        };

        /**
         * @brief Create stereo screen for viewer
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
        bool handleLineScreenIntersection(IntersectionPlane & plane) const;
        void stencilOutView(const CameraOrient & cam, IntersectionPlane & l_p,
                IntersectionPlane & r_p, GLint ref, GLuint mask,
                GLuint write_mask) const;
        bool IsInsideOfFrustum(const CameraOrient &cam,
                const osg::Vec3f &p) const;
        bool handleCameraScreenIntersection(const CameraOrient & cam, GLint ref,
                GLuint mask, GLuint write_mask) const;
        void setupZones(CameraOrient & cam0, CameraOrient & cam1) const;
        void debugStencilBuffer(GLint, GLint, GLsizei, GLsizei, char *) const;

        osg::DisplaySettings::StereoMode _stereoMode;
        ScreenInfoXZ * _screenInfoXZ;
        unsigned int _num_render;
        std::vector<osg::Camera *> _cameras;
        std::vector<PreDrawCallback *> _preCallbacks;
        std::vector<StereoCallback *> _stereoCallbacks;

        static cvr::MenuCheckbox *_debug_mode, *_align_head;
        static cvr::SubMenu *_mvs_menu;
        static cvr::MenuRangeValue *_fov_dial;
};

/**
 * @}
 */

}

#endif
