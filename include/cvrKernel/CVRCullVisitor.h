/**
 * @file CVRCullVisitor.h
 */

#ifndef CVR_CULL_VISITOR_H
#define CVR_CULL_VISITOR_H

#include <cvrKernel/NodeMask.h>

#include <osgUtil/CullVisitor>
#include <osg/BoundingBox>
#include <osg/NodeVisitor>

#include <iostream>

namespace cvr
{

/**
 * @addtogroup kernel
 * @{
 */

/**
 * @brief Modified osg cull visitor
 *
 * Uses CalVR defined node masks to force each object to draw the first
 *  time it is seen
 */
class CVRCullVisitor : public osgUtil::CullVisitor
{
    public:
        CVRCullVisitor();
        CVRCullVisitor(const CVRCullVisitor& cv);
        virtual CullVisitor* clone() const
        {
            return new CVRCullVisitor(*this);
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
                    return CullStack::isCulled(node);
                }
            }
            else
            {
                //std::cerr << "Disable first cull disabled." << std::endl;
                _skipCull = false;
                _firstCullStatus = false;
                return CullStack::isCulled(node);
            }
        }

        inline bool isCulled(const osg::BoundingBox& bb)
        {
            if(!_cullingStatus || _skipCull)
            {
                return false;
            }
            return CullStack::isCulled(bb);
        }

        class PreCullVisitor : public osg::NodeVisitor
        {
            public:
                PreCullVisitor();

                virtual void apply(osg::Node& node);
                virtual void apply(osg::Group& group);

            protected:
                bool _setMask;
        };

        class PostCullVisitor : public osg::NodeVisitor
        {
            public:
                PostCullVisitor();

                virtual void apply(osg::Node& node);
                virtual void apply(osg::Group& group);
        };

    protected:
        bool _cullingStatus;
        bool _firstCullStatus;
        bool _skipCull;

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

/**
 * @}
 */

}
#endif
