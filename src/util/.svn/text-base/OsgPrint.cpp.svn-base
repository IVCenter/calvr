#include <iostream>

#include <osg/Matrixd>
#include <osg/Vec4f>
#include <osg/Vec3f>
#include <osg/Vec2f>

#include <util/OsgPrint.h>

namespace cvr {
#define LINE_SEP "==================================================\n"
void print(osg::Vec2f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << std::endl <<
	"Y: " << v.y() << std::endl;
    std::cout << LINE_SEP;
}
void print(osg::Vec3f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << std::endl <<
	"Y: " << v.y() << std::endl <<
	"Z: " << v.z() << std::endl;
    std::cout << LINE_SEP;
}
void print(osg::Vec4f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << std::endl <<
	"Y: " << v.y() << std::endl <<
	"Z: " << v.z() << std::endl <<
	"W: " << std::endl;
    std::cout << LINE_SEP;
}
void print(osg::Matrixd & m) {
    std::cout << LINE_SEP;
    for(unsigned int i = 0; i < 4; ++i) {
	std::cout << m(i, 0) << " " << m(i, 1) << " " <<
	    m(i, 2) << " " << m(i,3) << std::endl;
    }
    std::cout << LINE_SEP;
}
}
