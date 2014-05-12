/**
 * @file OsgMath.h
 */
#ifndef CALVR_OSG_MATH_H
#define CALVR_OSG_MATH_H

#include <cvrUtil\Export.h>

#include <osg/Vec3>

namespace cvr
{

/**
 * @addtogroup util
 * @{
 */

/**
 * @brief Find the distance from a point to a line
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param point point to find distance to
 * @return distance between the point and the line
 */
CVRUTIL_EXPORT float linePointDistance(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 point);

/**
 * @brief Find the distance from a point to a line
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param point point to find distance to
 * @return distance between the point and the line
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT float linePointDistanceRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & point);

/**
 * @brief Find the closest point on a line to a specified point
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param point point to find closest point to
 * @param closestPoint vector that is filled with the closest point
 * @return how far down the line segment this point is. i.e. closestPoint = linePoint1 + (linePoint2 - linePoint1) * (return value)
 */
CVRUTIL_EXPORT float linePointClosestPoint(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 point, osg::Vec3 & closestPoint);

/**
 * @brief Find the closest point on a line to a specified point
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param point point to find closest point to
 * @param closestPoint vector that is filled with the closest point
 * @return weight for this point down the line segment. i.e. closestPoint = linePoint1 + (linePoint2 - linePoint1) * (return value)
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT float linePointClosestPointRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & point,
        osg::Vec3 & closestPoint);

/**
 * @brief Find the intersection points between a line and a sphere
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param center center of the sphere
 * @param radius of the sphere
 * @param intersect1 filled with first intersection point
 * @param w1 filled with weight down line segment of first intersection
 * @param intersect2 filled with second intersection point
 * @param w2 filled with weight down line segment of second intersection
 * @return returns true if there is an intersection
 */
CVRUTIL_EXPORT bool lineSphereIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 center, float radius, osg::Vec3 & intersect1, float & w1,
        osg::Vec3 & intersect2, float & w2);

/**
 * @brief Find the intersection points between a line and a sphere
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param center center of the sphere
 * @param radius of the sphere
 * @param intersect1 filled with first intersection point
 * @param w1 filled with weight down line segment of first intersection
 * @param intersect2 filled with second intersection point
 * @param w2 filled with weight down line segment of second intersection
 * @return returns true if there is an intersection
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT bool lineSphereIntersectionRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & center, float radius,
        osg::Vec3 & intersect1, float & w1, osg::Vec3 & intersect2, float & w2);

/**
 * @brief Find the intersection point between a line and a plane
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param intersect filled with the point of intersection
 * @param w weight down the line segment for the intersection point
 * @return returns true if the line intersects the plane
 */
CVRUTIL_EXPORT bool linePlaneIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 planePoint, osg::Vec3 planeNormal, osg::Vec3 & intersect,
        float & w);

/**
 * @brief Find the intersection point between a line and a plane
 * @param linePoint1 first point on line
 * @param linePoint2 second point on line
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param intersect filled with the point of intersection
 * @param w weight down the line segment for the intersection point
 * @return returns true if the line intersects the plane
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT bool linePlaneIntersectionRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, osg::Vec3 & intersect, float & w);

/**
 * @brief Find the distance between a point and a plane
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param point point to find the distance to
 * @return distance between the point and the plane
 */
CVRUTIL_EXPORT float planePointDistance(osg::Vec3 planePoint, osg::Vec3 planeNormal,
        osg::Vec3 point);

/**
 * @brief Find the distance between a point and a plane
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param point point to find the distance to
 * @return distance between the point and the plane
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT float planePointDistanceRef(const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, const osg::Vec3 & point);

/**
 * @brief Find the closest point on a plane to the given point
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param point point to find closest point to
 * @param closestPoint filled with the closest point in the plane
 */
CVRUTIL_EXPORT void planePointClosestPoint(osg::Vec3 planePoint, osg::Vec3 planeNormal,
        osg::Vec3 point, osg::Vec3 & closestPoint);

/**
 * @brief Find the closest point on a plane to the given point
 * @param planePoint a point in the plane
 * @param planeNormal normal for the plane, need not be normalized
 * @param point point to find closest point to
 * @param closestPoint filled with the closest point in the plane
 *
 * Uses references to avoid param copies
 */
CVRUTIL_EXPORT void planePointClosestPointRef(const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, const osg::Vec3 & point,
        osg::Vec3 & closestPoint);

/**
 * @}
 */

}
#endif
