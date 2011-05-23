#ifndef MENU_ISECT_H
#define MENU_ISECT_H

#include <util/Export.h>
#include <osgUtil/IntersectVisitor>

class IsectInfo     // this is an optional class to illustrate the return values of the accept() function
{
  public:
      bool       found;              ///< false: no intersection found
      osg::Vec3  point;              ///< intersection point
      osg::Vec3  normal;             ///< intersection normal
      osg::Geode *geode;              ///< intersected Geode
};

CVRUTIL_EXPORT std::vector<IsectInfo> getObjectIntersection(osg::Node *root,
                                             osg::Vec3& wPointerStart, osg::Vec3& wPointerEnd);

#endif
