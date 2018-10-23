/**
 * @file OsgMath.h
 */
#ifndef CALVR_OSG_GLES_MATH_H
#define CALVR_OSG_GLES_MATH_H

#include <cvrUtil/Export.h>

#include <osg/Vec3>

namespace cvr{

    CVRUTIL_EXPORT osg::Matrixf rawRotation2OsgMatrix(const float* rotation);

    CVRUTIL_EXPORT osg::Matrixf rawTrans2OsgMatrix(const float* translation);

    CVRUTIL_EXPORT void quat2OSGEuler(const float* q, float& roll, float& pitch, float& yaw);

}
#endif
