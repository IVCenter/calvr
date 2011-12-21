#include <util/ComputeBoundingBoxVisitor.h>

#include <osg/Geode>

#include <iostream>

using namespace cvr;

ComputeBoundingBoxVisitor::ComputeBoundingBoxVisitor() :
        osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_curMatrix = osg::Matrix();
    m_bb.init();
}

void ComputeBoundingBoxVisitor::apply(osg::Geode &geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); i++)
    {
        osg::BoundingBox bb = geode.getDrawable(i)->computeBound();
        m_bb.expandBy(bb.corner(0) * m_curMatrix);
        m_bb.expandBy(bb.corner(1) * m_curMatrix);
        m_bb.expandBy(bb.corner(2) * m_curMatrix);
        m_bb.expandBy(bb.corner(3) * m_curMatrix);
        m_bb.expandBy(bb.corner(4) * m_curMatrix);
        m_bb.expandBy(bb.corner(5) * m_curMatrix);
        m_bb.expandBy(bb.corner(6) * m_curMatrix);
        m_bb.expandBy(bb.corner(7) * m_curMatrix);
    }
}

void ComputeBoundingBoxVisitor::apply(osg::Transform& node)
{
    if(node.asMatrixTransform() || node.asPositionAttitudeTransform())
    {
        osg::Matrix prevMatrix = m_curMatrix;

        m_curMatrix.preMult(node.asMatrixTransform()->getMatrix());

        traverse(node);

        m_curMatrix = prevMatrix;
    }
}
