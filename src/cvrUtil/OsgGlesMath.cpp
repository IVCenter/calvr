#include <cvrUtil/OsgMath.h>

#include <cmath>
#include <osg/Matrixf>

using namespace cvr;
using namespace osg;
namespace cvr {
    osg::Matrixf rawRotation2OsgMatrix(const float *rotation) {
        osg::Quat quat = osg::Quat(rotation[0], -rotation[2], rotation[1], rotation[3]);
        osg::Matrixf mat;
        mat.makeRotate(quat);
        return mat;
    }

    osg::Matrixf rawTrans2OsgMatrix(const float *translation) {
        Vec3f pos = Vec3f(translation[0], -translation[2], translation[1]);
        osg::Matrixf mat;
        mat.makeTranslate(pos);
        return mat;
    }

    void quat2Euler(const float *q, float &roll, float &pitch, float &yaw) {
        // roll (x-axis rotation)
        double sinr_cosp = +2.0 * (q[3] * q[0] + q[1] * q[2]);
        double cosr_cosp = +1.0 - 2.0 * (q[0] * q[0] + q[1] * q[1]);
        roll = (float) atan2(sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        double sinp = +2.0 * (q[3] * q[1] - q[2] * q[0]);
        if (fabs(sinp) >= 1)
            pitch = (float) copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
            pitch = (float) asin(sinp);

        // yaw (z-axis rotation)
        double siny_cosp = +2.0 * (q[3] * q[2] + q[0] * q[1]);
        double cosy_cosp = +1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
        yaw = (float) atan2(siny_cosp, cosy_cosp);
    }

    float calculateDistanceToPlane(float* plane_pose, float* camera_pose) {

        osg::Vec3f plane_position(plane_pose[4], plane_pose[5], plane_pose[6]);
        osg::Vec3f camera_P_plane(camera_pose[4] - plane_position.x(),
                                  camera_pose[5] - plane_position.y(),
                                  camera_pose[6] - plane_position.z());

        osg::Quat plane_quaternion(plane_pose[0],plane_pose[1], plane_pose[2],plane_pose[3]);
        // Get normal vector, normal is defined to be positive Y-position in local
        // frame.
        osg::Vec3f normal_vec = plane_quaternion * osg::Vec3f(0,1.0f,0);

        return normal_vec * camera_P_plane;
    }
}