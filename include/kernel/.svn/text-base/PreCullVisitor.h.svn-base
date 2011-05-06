#ifndef CVR_PRE_CULL_VISITOR_H
#define CVR_PRE_CULL_VISITOR_H

#include <osg/NodeVisitor>

namespace cvr
{

class PreCullVisitor : public osg::NodeVisitor
{
    public:
        PreCullVisitor();
        
        virtual void apply(osg::Node& node);
        virtual void apply(osg::Group& group);

    protected:
        bool _setMask;
};

}
#endif
