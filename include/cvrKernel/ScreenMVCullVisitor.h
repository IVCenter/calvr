#ifndef CALVR_SCREEN_MV_CULL_VISITOR_H
#define CALVR_SCREEN_MV_CULL_VISITOR_H

#include <cvrKernel/NodeMask.h>

#include <osgUtil/CullVisitor>
#include <osg/BoundingBox>
#include <osg/Polytope>
#include <osg/Matrix>

#include <iostream>
#include <stack>

namespace cvr
{

class ScreenMVCullVisitor : public osgUtil::CullVisitor
{
    public:
        ScreenMVCullVisitor();
        ScreenMVCullVisitor(const ScreenMVCullVisitor& cv);
        virtual CullVisitor* clone() const
        {
            return new ScreenMVCullVisitor(*this);
        }

        inline void printDebug(osg::Vec3 point)
        {
            osg::Polytope::PlaneList pl;
            pl = _currentNearFrustum.getPlaneList();
            std::cerr << "Point x: " << point.x() << " y: " << point.y()
                    << " z: " << point.z() << std::endl;
            for(int i = 0; i < pl.size(); i++)
            {
                std::cerr << "Near Dist: " << pl[i].distance(point)
                        << std::endl;
            }

            pl = _currentFarFrustum.getPlaneList();
            for(int i = 0; i < pl.size(); i++)
            {
                std::cerr << "Far Dist: " << pl[i].distance(point) << std::endl;
            }
        }

        inline bool isInCullArea(const osg::BoundingBox& bb)
        {
            //printDebug(bb.center());
            //std::cerr << "isInCullArea bb value: " << !(_currentNearFrustum.contains(bb) || _currentFarFrustum.contains(bb)) << std::endl;
            //return false;
            return !(_currentNearFrustum.contains(bb)
                    || _currentFarFrustum.contains(bb));
        }

        inline bool isInCullArea(const osg::BoundingSphere& bs)
        {
            //printDebug(bs.center());
            return !(_currentNearFrustum.contains(bs)
                    || _currentFarFrustum.contains(bs));
        }

        inline bool isInCullArea(const osg::Node& node)
        {
            //std::cerr << "isInCullArea node value: " << isInCullArea(node.getBound()) << std::endl;
            //return false;
            return isInCullArea(node.getBound());
        }

        inline bool isCulled(const osg::Node& node)
        {
            if(!_cullingStatus)
            {
                //std::cerr << "Not cull status" << std::endl;
                return false;
            }
            else if(!(node.getNodeMask() & CULL_ENABLE))
            {
                //std::cerr << "Cull Disabled." << std::endl;
                _cullingStatus = false;
                return false;
            }
            else if(_firstCullStatus && node.getNodeMask() & DISABLE_FIRST_CULL)
            {
                if(node.getNodeMask() & FIRST_CULL_STATUS)
                {
                    //std::cerr << "Disable first cull." << std::endl;
                    _skipCull = true;
                    return false;
                }
                else
                {
                    //std::cerr << "First cull done" << std::endl;
                    _skipCull = false;
                    /*bool b = isInCullArea(node);
                     if(b)
                     {
                     std::cerr << "Culling node" << std::endl;
                     }
                     return b;*/
                    return isInCullArea(node);
                }
            }
            else
            {
                _skipCull = false;
                _firstCullStatus = false;
                /*bool b = isInCullArea(node);
                 if(b)
                 {
                 std::cerr << "Culling node" << std::endl;
                 }
                 return b;*/
                return isInCullArea(node);
            }
        }

        inline bool isCulled(const osg::BoundingBox& bb)
        {
            if(!_cullingStatus || _skipCull)
            {
                return false;
            }
            /*bool b = isInCullArea(bb);
             if(b)
             {
             std::cerr << "Culling geometry" << std::endl;
             }
             return b;*/
            return isInCullArea(bb);
        }

        void pushModelViewMatrix(osg::RefMatrix* matrix,
                osg::Transform::ReferenceFrame referenceFrame);
        void popModelViewMatrix();
        void setFrustums(osg::Polytope & near, osg::Polytope & far);

    protected:
        bool _cullingStatus;
        bool _firstCullStatus;
        bool _skipCull;

        std::stack<osg::Matrix> _invMVStack;
        osg::Polytope _currentNearFrustum;
        osg::Polytope _currentFarFrustum;
        osg::Polytope _nearFrustum;
        osg::Polytope _farFrustum;

    public:
        // osgUtil::CullVisitor
        virtual void apply(osg::Node&);
        virtual void apply(osg::Geode& node);
        virtual void apply(osg::Billboard& node);
        virtual void apply(osg::LightSource& node);
        virtual void apply(osg::ClipNode& node);
        virtual void apply(osg::TexGenNode& node);

        virtual void apply(osg::Group& node);
        virtual void apply(osg::Transform& node);
        virtual void apply(osg::Projection& node);
        virtual void apply(osg::Switch& node);
        virtual void apply(osg::LOD& node);
        virtual void apply(osg::ClearNode& node);
        virtual void apply(osg::Camera& node);
        virtual void apply(osg::OccluderNode& node);
        virtual void apply(osg::OcclusionQueryNode& node);
};

}
#endif
