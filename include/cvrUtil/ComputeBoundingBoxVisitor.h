/**
 * @file ComputeBoundingBoxVisitor.h
 */
#ifndef CALVR_COMP_BB_VIS_H
#define CALVR_COMP_BB_VIS_H

#include <cvrUtil/Export.h>

#include <osg/BoundingBox>
#include <osg/MatrixTransform>

namespace cvr
{

/**
 * @addtogroup util cvrUtil
 * @{
 */

/**
 * @brief Vistor that computes the bounding box for a subgraph
 */
class CVRUTIL_EXPORT ComputeBoundingBoxVisitor : public osg::NodeVisitor
{
    public:
        ComputeBoundingBoxVisitor();

        /**
         * @brief Get the visitor's bounding box
         */
        const osg::BoundingBox & getBound()
        {
            return m_bb;
        }

        /**
         * @brief Set the visitor's bounding box
         */
        void setBound(osg::BoundingBox & bb)
        {
            m_bb = bb;
        }
        virtual void apply(osg::Transform&);
        virtual void apply(osg::Geode&);

    protected:
        osg::BoundingBox m_bb; ///< visitor's bounding box
        osg::Matrix m_curMatrix; ///< current matrix transform
};

/**
 * @}
 */

}
#endif
