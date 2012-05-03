#include <cvrUtil/LocalToWorldVisitor.h>

namespace cvr
{

osg::Matrixd getLocalToWorldMatrix(osg::Node* node)
{
    getWorldCoordOfNodeVisitor* ncv = new getWorldCoordOfNodeVisitor();
    osg::Matrix retMat;
    if(node && ncv)
    {
        node->accept(*ncv);
        retMat = ncv->getMat();
    }
    delete ncv;
    return retMat;
}

}
