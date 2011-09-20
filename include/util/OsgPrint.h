/**
 * @file OsgPrint.h
 */
#include <util/Export.h>
#include <osg/Matrix>

#include <string>

namespace cvr 
{

/**
 * @brief Utility function to print a vector
 * @param v vector to print
 * @param label label to use in the print output
 */
CVRUTIL_EXPORT void print(osg::Vec2 & v, std::string label = "Vec2");

/**
 * @brief Utility function to print a vector
 * @param v vector to print
 * @param label label to use in the print output
 */
CVRUTIL_EXPORT void print(osg::Vec3 & v, std::string label = "Vec3");

/**
 * @brief Utility function to print a vector
 * @param v vector to print
 * @param label label to use in the print output
 */
CVRUTIL_EXPORT void print(osg::Vec4 & v, std::string label = "Vec4");

/**
 * @brief Utility function to print a matrix
 * @param m matrix to print
 * @param label label to use in the print output
 * @param transpose should the output be transposed
 */
CVRUTIL_EXPORT void print(osg::Matrix & m, std::string label = "Matrix", bool transpose = false);

}
