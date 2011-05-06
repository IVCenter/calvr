#include <kernel/PreCullVisitor.h>
#include <kernel/NodeMask.h>

#include <osg/Group>

#include <iostream>

using namespace osg;
using namespace cvr;

PreCullVisitor::PreCullVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    setTraversalMask(DISABLE_FIRST_CULL);
    _setMask = false;
}

void PreCullVisitor::apply(Node& node)
{
    if(node.getNodeMask() & FIRST_CULL_STATUS)
    {
	_setMask = true;
    }
}

void PreCullVisitor::apply(Group& group)
{
    bool setMask = false;
    for(int i = 0; i < group.getNumChildren(); i++)
    {
	_setMask = false;
	group.getChild(i)->accept(*this);
	if(_setMask)
	{
	    setMask = true;
	}
    }

    if(group.getNodeMask() & FIRST_CULL_STATUS)
    {
	_setMask = true;
    }
    else if(setMask)
    {
	//std::cerr << "Pulling up node mask." << std::endl;
	_setMask = true;
	group.setNodeMask(group.getNodeMask() | FIRST_CULL_STATUS);
    }
    
}
