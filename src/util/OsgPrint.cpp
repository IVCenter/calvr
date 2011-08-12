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
    std::cout << "X: " << v.x() << "\n";
    std::cout << "Y: " << v.y() << "\n";
    std::cout << LINE_SEP;
}

void print(const osg::Vec3f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << "\n";
    std::cout << "Y: " << v.y() << "\n";
    std::cout << "Z: " << v.z() << "\n";
    std::cout << LINE_SEP;
}

void print(osg::Vec3f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << "\n";
    std::cout << "Y: " << v.y() << "\n";
    std::cout << "Z: " << v.z() << "\n";
    std::cout << LINE_SEP;
}

void print(osg::Vec4f & v) {
    std::cout << LINE_SEP;
    std::cout << "X: " << v.x() << "\n";
    std::cout << "Y: " << v.y() << "\n";
    std::cout << "Z: " << v.z() << "\n";
    std::cout << LINE_SEP;
}

void print(osg::Matrixd & m) {
    std::cout << LINE_SEP;
    for(unsigned int i = 0; i < 4; ++i) {
	std::cerr << m(i, 0) << " " << m(i, 1) << " " <<
	    m(i, 2) << " " << m(i,3) << "\n";
    }
    std::cerr << LINE_SEP;
}
}
