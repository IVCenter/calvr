#include <cvrKernel/ScreenMVSimulator.h>

using namespace cvr;

std::map<int,osg::Matrix *> ScreenMVSimulator::headMat;

void ScreenMVSimulator::setSimulatedHeadMatrix(int head, osg::Matrix * matrix)
{
    if(matrix == NULL)
    {
        if(isSimulatedHeadMatrix(head))
        {
            delete headMat[head];
            headMat.erase(head);
        }
        return;
    }

    if(!isSimulatedHeadMatrix(head))
        headMat[head] = new osg::Matrix();

    *headMat[head] = *matrix;
}

bool ScreenMVSimulator::isSimulatedHeadMatrix(int head)
{
    return headMat.find(head) != headMat.end();
}

osg::Matrix & ScreenMVSimulator::getCurrentHeadMatrix(int head)
{
    return isSimulatedHeadMatrix(head) ?
            *headMat[head] : ScreenBase::getCurrentHeadMatrix(head);
}

osg::Vec3d ScreenMVSimulator::defaultLeftEye(int head)
{
    return isSimulatedHeadMatrix(head) ?
            osg::Vec3d(-_separation * _eyeSepMult / 2.0,0.0,0.0)
                    * getCurrentHeadMatrix(head) :
            ScreenBase::defaultLeftEye(head);
}

osg::Vec3d ScreenMVSimulator::defaultRightEye(int head)
{
    return isSimulatedHeadMatrix(head) ?
            osg::Vec3d(_separation * _eyeSepMult / 2.0,0.0,0.0)
                    * getCurrentHeadMatrix(head) :
            ScreenBase::defaultRightEye(head);
}
