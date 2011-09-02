#ifndef CALVR_OSG_MATH_H
#define CALVR_OSG_MATH_H

#include <osg/Vec3>

namespace cvr
{

float linePointDistance(osg::Vec3 linePoint1, osg::Vec3 linePoint2, osg::Vec3 point);
float linePointDistanceRef(const osg::Vec3 & linePoint1, const osg::Vec3 & linePoint2, const osg::Vec3 & point);

float linePointClosestPoint(osg::Vec3 linePoint1, osg::Vec3 linePoint2, osg::Vec3 point, osg::Vec3 & closestPoint);
float linePointClosestPointRef(const osg::Vec3 & linePoint1, const osg::Vec3 & linePoint2, const osg::Vec3 & point, osg::Vec3 & closestPoint);

bool lineSphereIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2, osg::Vec3 center, float radius, osg::Vec3 & intersect1, float & w1, osg::Vec3 & intersect2, float & w2);
bool lineSphereIntersectionRef(const osg::Vec3 & linePoint1, const osg::Vec3 & linePoint2, const osg::Vec3 & center, float radius, osg::Vec3 & intersect1, float & w1, osg::Vec3 & intersect2, float & w2);

bool linePlaneIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2, osg::Vec3 planePoint, osg::Vec3 planeNormal, osg::Vec3 & intersect, float & w);
bool linePlaneIntersectionRef(const osg::Vec3 & linePoint1, const osg::Vec3 & linePoint2, const osg::Vec3 & planePoint, const osg::Vec3 & planeNormal, osg::Vec3 & intersect, float & w);

float planePointDistance(osg::Vec3 planePoint, osg::Vec3 planeNormal, osg::Vec3 point);
float planePointDistanceRef(const osg::Vec3 & planePoint, const osg::Vec3 & planeNormal, const osg::Vec3 & point);

void planePointClosestPoint(osg::Vec3 planePoint, osg::Vec3 planeNormal, osg::Vec3 point, osg::Vec3 & closestPoint);
void planePointClosestPointRef(const osg::Vec3 & planePoint, const osg::Vec3 & planeNormal, const osg::Vec3 & point, osg::Vec3 & closestPoint);

}
#endif
