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

    void quat2OSGEuler(const float *q, float &roll, float &pitch, float &yaw) {
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

        //Change to OSG Coordinates
        std::swap(pitch, yaw);
        pitch = -pitch;
    }
}