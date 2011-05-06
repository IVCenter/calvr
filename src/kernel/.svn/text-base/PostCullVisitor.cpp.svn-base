#include <kernel/PostCullVisitor.h>
#include <kernel/NodeMask.h>

#include <osg/Group>

#include <iostream>

using namespace osg;
using namespace cvr;

PostCullVisitor::PostCullVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    setTraversalMask(DISABLE_FIRST_CULL);
}

void PostCullVisitor::apply(Node& node)
{
    node.setNodeMask(node.getNodeMask() & ~(FIRST_CULL_STATUS));
}

void PostCullVisitor::apply(Group& group)
{
    group.setNodeMask(group.getNodeMask() & ~(FIRST_CULL_STATUS));
    traverse(group);
}
