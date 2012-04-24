#include <iostream>

#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>

#include <cvrUtil/OsgPrint.h>

namespace cvr
{

void print(osg::Vec2 & v, std::string label)
{
    std::cerr << label << " x: " << v.x() << " y: " << v.y() << std::endl;
}

void print(osg::Vec3 & v, std::string label)
{
    std::cerr << label << " x: " << v.x() << " y: " << v.y() << " z: " << v.z()
            << std::endl;
}

void print(osg::Vec4 & v, std::string label)
{
    std::cerr << label << " x: " << v.x() << " y: " << v.y() << " z: " << v.z()
            << " w: " << v.w() << std::endl;
}

void print(osg::Matrix & m, std::string label, bool transpose)
{
    std::cerr << label << ":" << std::endl;
    if(!transpose)
    {
        for(unsigned int i = 0; i < 4; ++i)
        {
            std::cerr << m(i,0) << " " << m(i,1) << " " << m(i,2) << " "
                    << m(i,3) << "\n";
        }
    }
    else
    {
        for(unsigned int i = 0; i < 4; ++i)
        {
            std::cerr << m(0,i) << " " << m(1,i) << " " << m(2,i) << " "
                    << m(3,i) << std::endl;
        }
    }
}

}
