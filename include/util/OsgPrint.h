#include <util/Export.h>
#include <osg/Matrix>

namespace cvr {
CVRUTIL_EXPORT void print(osg::Vec2f &);
CVRUTIL_EXPORT void print(osg::Vec3f &);
CVRUTIL_EXPORT void print(const osg::Vec3f &);
CVRUTIL_EXPORT void print(osg::Vec4f &);
CVRUTIL_EXPORT void print(osg::Matrixd &);
}
