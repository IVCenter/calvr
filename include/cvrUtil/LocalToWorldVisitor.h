/**
 * @file LocalToWorldVisitor.h
 */
#ifndef CVR_LOCAL_TO_WORLD_VISITOR_H
#define CVR_LOCAL_TO_WORLD_VISITOR_H

#include <cvrUtil/Export.h>

#include <osg/Matrix>
#include <osg/NodeVisitor>
#include <osg/Transform>

namespace cvr
{

// Takes local coordinates and computes world coordinates.
class getWorldCoordOfNodeVisitor : public osg::NodeVisitor
{
    public:
        getWorldCoordOfNodeVisitor() :
                osg::NodeVisitor(NodeVisitor::TRAVERSE_PARENTS), _done(false)
        {
            _matrix = osg::Matrixd();
        }

        virtual void apply(osg::Node &node)
        {
            if(!_done)
            {
                if(0 == node.getNumParents()) // no parents
                {
                    _matrix.set(osg::computeLocalToWorld(this->getNodePath()));
                    _done = true;
                }
                traverse(node);
            }
        }

        osg::Matrixd & getMat()
        {
            return _matrix;
        }

    private:
        bool _done;
        osg::Matrix _matrix;
};

/**
 * @brief Computes the matrix to transform the local coords of a given node into world space
 * @param node local node
 * @return local to world matrix
 *
 * Note: is node is a transform type node, its transform will be contained in the resulting local to world
 * matrix
 */
osg::Matrixd CVRUTIL_EXPORT getLocalToWorldMatrix(osg::Node* node);

}

#endif
