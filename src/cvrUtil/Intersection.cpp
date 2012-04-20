#include <cvrUtil/Intersection.h>
#include <cvrKernel/NodeMask.h>

std::vector<IsectInfo> getObjectIntersection(osg::Node *root,
        osg::Vec3& wPointerStart, osg::Vec3& wPointerEnd)
{
    // Compute intersections of viewing ray with objects:
    osgUtil::IntersectVisitor iv;
    osg::ref_ptr < osg::LineSegment > testSegment = new osg::LineSegment();
    testSegment->set(wPointerStart,wPointerEnd);
    iv.addLineSegment(testSegment.get());
    iv.setTraversalMask(cvr::INTERSECT_MASK);
    std::vector<IsectInfo> isecvec;

    // Traverse the whole scenegraph.
    // Non-Interactive objects must have been marked with setNodeMask(~2):
    root->accept(iv);
    if(iv.hits())
    {
        float minDist = FLT_MAX;
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(
                testSegment.get());
        if(!hitList.empty())
        {
            for(size_t i = 0; i < hitList.size(); i++)
            {
                IsectInfo isect;
                isect.point = hitList.at(i).getWorldIntersectPoint();
                isect.normal = hitList.at(i).getWorldIntersectNormal();
                isect.geode = hitList.at(i)._geode.get();
                isect.found = true;
                minDist = (isect.point - wPointerStart).length2();
                isecvec.push_back(isect);
            }
        }
    }
    return isecvec;
}
