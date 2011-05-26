/**
 * @file ScreenMVSimulator.h
 * @author John Mangan
 */

#ifndef CALVR_SCREEN_MV_SIMULATOR_H
#define CALVR_SCREEN_MV_SIMULATOR_H

#include <vector>
#include <map>
#include <kernel/ScreenBase.h>
#include <osgViewer/Renderer>

namespace cvr
{

/**
 * @brief Creates a stereo screen for multiple viewers using osg stereo modes and position/orientation weighting
 */
class ScreenMVSimulator : public ScreenBase
{
    public:
        ScreenMVSimulator() : ScreenBase() {}
        virtual ~ScreenMVSimulator() {}

        /**
         * @brief Sets the corresponding head's matrix, to be overriden, or stop it from being simulated.
         * @param head integer corresponding to which head to set (will create a new "head" if it doesn't already exist)
         * @param matrix points to the osg::Matrix whose value will be copied into the headMatrix (NULL value will disable the head from simulation.)
         */
        static void setSimulatedHeadMatrix(int head, osg::Matrix * matrix);

        /**
         * @brief Gets the corresponding head's matrix if it is simulated.
         * @param head integer for which existence is in question
         * @return bool representing whether the head is simulated
         */
        static bool isSimulatedHeadMatrix(int head);

        /**
          * @overload ScreenBase::getCurrentHeadMatrix
          * @brief Overloaded function will return the corresponding headMat matrix if it exists, and defaults to the ScreenBase::getCurrentHeadMatrix function otherwise.
          * @param head integer representing which head's matrix to return
          * @return osg Matrix corresponding to the respective head
          */
        osg::Matrix getCurrentHeadMatrix(int head=0);

        /**
          * @overload ScreenBase::defaultLeftEye
          * @brief Overloaded function will return the corresponding head's left eye position, based on the simulated headMat matrix if it exists, and defaults to the ScreenBase::defaultLeftEye function otherwise.
          * @param head integer representing which head's left eye position to return
          * @return osg Vec3 corresponding to the respective head's left eye position
          */
        osg::Vec3 defaultLeftEye(int head=0);

        /**
          * @overload ScreenBase::defaultRightEye
          * @brief Overloaded function will return the corresponding head's right eye position, based on the simulated headMat matrix if it exists, and defaults to the ScreenBase::defaultRightEye function otherwise.
          * @param head integer representing which head's right eye position to return
          * @return osg Vec3 corresponding to the respective head's right eye position
          */
        osg::Vec3 defaultRightEye(int head=0);

    protected:

        static std::map<int,osg::Matrix *> headMat; ///< Stores simulated head matrices (used instead of TrackingManager values).
};
}

#endif
