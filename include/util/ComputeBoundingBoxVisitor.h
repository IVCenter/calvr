#ifndef CALVR_COMP_BB_VIS_H
#define CALVR_COMP_BB_VIS_H

#include <osg/BoundingBox>
#include <osg/MatrixTransform>

namespace cvr
{

class ComputeBoundingBoxVisitor : public osg::NodeVisitor
{
    public:
        ComputeBoundingBoxVisitor();
        const osg::BoundingBox & getBound() { return m_bb; }
        void setBound(osg::BoundingBox & bb) { m_bb = bb; }
        virtual void apply(osg::Transform&);
        virtual void apply(osg::Geode&);

    protected:
        osg::BoundingBox m_bb;
        osg::Matrix m_curMatrix;
};

}
#endif
