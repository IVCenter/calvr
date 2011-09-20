/**
 * @file Intersection.h
 */
#ifndef MENU_ISECT_H
#define MENU_ISECT_H

#include <util/Export.h>
#include <osgUtil/IntersectVisitor>

/**
 * @brief Contains intersection information
 *
 * Result of the getObjectIntersection() function
 */
class IsectInfo
{
  public:
      bool       found;              ///< false: no intersection found
      osg::Vec3  point;              ///< intersection point
      osg::Vec3  normal;             ///< intersection normal
      osg::Geode *geode;              ///< intersected Geode
};

//TODO: modify this call to take a vector reference to remove the return copies
/**
 * @brief Finds all the intersections between a line segment and the geometry under a node
 * @param root scenegraph to seach under
 * @param wPointerStart start point of the line segment
 * @param wPointerEnd end point of the line segment
 * @return list of all geometry intersections
 */
CVRUTIL_EXPORT std::vector<IsectInfo> getObjectIntersection(osg::Node *root,
                                             osg::Vec3& wPointerStart, osg::Vec3& wPointerEnd);

#endif
