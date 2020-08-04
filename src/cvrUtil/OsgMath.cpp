#include <cvrUtil/OsgMath.h>

#include <cmath>

using namespace cvr;

float cvr::linePointDistance(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 point)
{
    return linePointDistanceRef(linePoint1,linePoint2,point);
}

float cvr::linePointDistanceRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & point)
{
    return ((point - linePoint1) ^ (point - linePoint2)).length()
            / (linePoint2 - linePoint1).length();
}

float cvr::linePointClosestPoint(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 point, osg::Vec3 & closestPoint)
{
    return linePointClosestPointRef(linePoint1,linePoint2,point,closestPoint);
}

float cvr::linePointClosestPointRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & point,
        osg::Vec3 & closestPoint)
{
    float u = ((point - linePoint1) * (linePoint2 - linePoint1))
            / (linePoint2 - linePoint1).length2();
    closestPoint = linePoint1 + ((linePoint2 - linePoint1) * u);
    return u;
}

bool cvr::lineSphereIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 center, float radius, osg::Vec3 & intersect1, float & w1,
        osg::Vec3 & intersect2, float & w2)
{
    return lineSphereIntersectionRef(linePoint1,linePoint2,center,radius,
            intersect1,w1,intersect2,w2);
}

bool cvr::lineSphereIntersectionRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & center, float radius,
        osg::Vec3 & intersect1, float & w1, osg::Vec3 & intersect2, float & w2)
{
    osg::Vec3 lineDir = linePoint2 - linePoint1;
    lineDir.normalize();

    osg::Vec3 c = center - linePoint1;
    float ldotc = lineDir * c;

    float determ = ldotc * ldotc - c * c + radius * radius;
    if(determ < 0)
    {
        return false;
    }

    w1 = ldotc - sqrt(determ);
    w2 = ldotc + sqrt(determ);

    intersect1 = lineDir * w1 + linePoint1;
    intersect2 = lineDir * w2 + linePoint1;

    return true;
}

bool cvr::linePlaneIntersection(osg::Vec3 linePoint1, osg::Vec3 linePoint2,
        osg::Vec3 planePoint, osg::Vec3 planeNormal, osg::Vec3 & intersect,
        float & w)
{
    return linePlaneIntersectionRef(linePoint1,linePoint2,planePoint,
            planeNormal,intersect,w);
}

bool cvr::linePlaneIntersectionRef(const osg::Vec3 & linePoint1,
        const osg::Vec3 & linePoint2, const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, osg::Vec3 & intersect, float & w)
{
    osg::Vec3 lineDir = linePoint2 - linePoint1;
    lineDir.normalize();

    osg::Vec3 pnorm = planeNormal;
    pnorm.normalize();

    float denom = (lineDir * pnorm);
    if(denom == 0.0)
    {
        return false;
    }

    w = ((planePoint - linePoint1) * pnorm) / denom;
    intersect = lineDir * w + linePoint1;
    return true;
}

void cvr::planePointClosestPoint(osg::Vec3 planePoint, osg::Vec3 planeNormal,
        osg::Vec3 point, osg::Vec3 & closestPoint)
{
    planePointClosestPointRef(planePoint,planeNormal,point,closestPoint);
}

void cvr::planePointClosestPointRef(const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, const osg::Vec3 & point,
        osg::Vec3 & closestPoint)
{
    float dist = planePointDistanceRef(planePoint,planeNormal,point);
    osg::Vec3 pnorm = planeNormal;
    pnorm.normalize();

    closestPoint = point - planeNormal * dist;
}

float cvr::planePointDistance(osg::Vec3 planePoint, osg::Vec3 planeNormal,
        osg::Vec3 point)
{
    return planePointDistanceRef(planePoint,planeNormal,point);
}

float cvr::planePointDistanceRef(const osg::Vec3 & planePoint,
        const osg::Vec3 & planeNormal, const osg::Vec3 & point)
{
    return ((point - planePoint) * planeNormal) / planeNormal.length();
}
