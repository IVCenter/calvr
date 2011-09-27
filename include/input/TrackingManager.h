/**
 * @file
 * @author Andrew Prudhomme (aprudhomme@ucsd.edu)
 */

#ifndef CALVR_TRACKING_MANAGER_H
#define CALVR_TRACKING_MANAGER_H

#include <input/Export.h>
#include <input/TrackerBase.h>
#include <kernel/CalVR.h>
#include <kernel/Navigation.h>
#include <kernel/SceneManager.h>

#include <osg/Matrix>
#include <osg/Vec3>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <vector>
#include <queue>
#include <list>
#include <map>

struct timeval;

/**
 * @brief Primary namespace of CalVR
 */
namespace cvr
{

struct InteractionEvent;
struct TrackingInteractionEvent;
class GenComplexTrackingEvents;

/**
 *  @brief Manages the body and button tracker objects and values.  
 *
 *  Provides access to head, hand and button information.
 *  Generates button interaction events.
 *  Can poll tracking values in a thread.
 */
class CVRINPUT_EXPORT TrackingManager : public OpenThreads::Thread
{
        friend class GenComplexTrackingEvents;
        friend class CalVR;
    public:

        /**
         * @brief Returns static pointer to class object
         */
        static TrackingManager * instance();

        /**
         * @brief Init config file specified tracking system for body and button tracking and
         * distributes tracking system information to render nodes.
         * @return false if error, true otherwise
         */
        bool init();

        /**
         * @brief Syncs tracker information with render nodes.
         * Generates/flushes button events. 
         */
        void update();

        /**
         * @brief Driving function for threaded operation.
         *
         * Polls the tracking systems and pushes all tracking events into a queue to be used in the next
         * update call.  Sleeps to hit a target updates per second.
         */
        virtual void run();

        /**
         * @brief Flag the thread to exit.
         */
        void quitThread();

        /**
         * @brief Returns if the tracker is being polled by a thread
         */
        bool isThreaded()
        {
            return _threaded;
        }

        /**
         * @brief Return which graphic type to use for a hand
         */
        SceneManager::PointerGraphicType getPointerGraphicType(int hand);

        /**
         * @brief Number of head bodies being tracked by the system
         */
        int getNumHands();

        /**
         * @brief Number of hand bodies being tracked by the system
         */
        int getNumHeads();

        /**
         * @brief Get the trasform for a given hand body
         * @param hand Hand number
         * @return Matrix trasform from world space to hand space
         */
        osg::Matrix & getHandMat(int hand = 0);

        /**
         * @brief Get the trasform for a given head body
         * @param head Head number
         * @return Matrix trasform from world space to head space
         */
        osg::Matrix & getHeadMat(int head = 0);

        /**
         * @brief Get the trasform for a given head body, continues to 
         * update even if head tracking is frozen
         * @param head Head number
         * @return Matrix trasform from world space to head space
         */
        osg::Matrix & getUnfrozenHeadMat(int head = 0);

        int getNumTrackingSystems();

        TrackerBase::TrackerType getHandTrackerType(int hand);

        /**
         * @brief Returns number of buttons present in a given button station
         */
        int getNumButtons(int system = 0);

        /**
         * @brief Returns the raw button mask for a given button station
         */
        unsigned int getRawButtonMask(int system = 0);

        /**
         * @brief Returns the button mask for a given hand, which is processed from the raw mask(s)
         */
        unsigned int getHandButtonMask(int hand = 0);

        /**
         * @brief Returns the number of valuators in a given valuator station
         */
        int getNumValuators(int system = 0);

        /**
         * @brief Returns the value of the valuator in a given station with a given index
         */
        float getValuator(int system, int index);

        /**
         * @brief Set if head matrix should be updated
         */
        void setUpdateHeadTracking(bool b);

        /**
         * @brief Get if head matrix is being updated
         */
        bool getUpdateHeadTracking();

        void cleanupCurrentEvents();

    protected:
        TrackingManager();
        virtual ~TrackingManager();

        /**
         * @brief Generate the hand button masks from the raw button mask(s)
         */
        void updateHandMask();

        /**
         * @brief Generate the thread's hand button masks from the thread's raw button mask(s)
         */
        void updateThreadHandMask();

        /**
         * @brief Generate button events based on button and tracking info
         */
        void generateButtonEvents();

        /**
         * @brief Generate thread button events based on thread tracking info, push to queue
         */
        void generateThreadButtonEvents();

        /**
         * @brief Update thread's local head/hand matrix transforms from tracker
         */
        void updateThreadMats();

        /**
         * @brief Sync and flush thread generated event queue into interaction pipeline
         */
        void flushEvents();

        void setGenHandDefaultButtonEvents();

        struct TrackingSystemInfo
        {
            int numBodies;
            int numButtons;
            int numVal;
            osg::Matrix systemTransform;
            std::vector<osg::Matrix> bodyRotations;
            std::vector<osg::Vec3> bodyTranslations;
            Navigation::NavImplementation navImp;
            TrackerBase::TrackerType trackerType;
            SceneManager::PointerGraphicType defaultPointerType;
            bool genDefaultButtonEvents;
            bool thread;
        };

        static TrackingManager * _myPtr; ///< Static self pointer

        std::vector<TrackerBase*> _systems; ///< List of all tracking systems
        std::vector<TrackingSystemInfo*> _systemInfo;

        bool _debugOutput;
        bool _updateHeadTracking;

        bool _threaded; ///< is there a thread polling the tracker
        float _threadFPS;
        bool _threadQuit;
        OpenThreads::Mutex _quitLock;
        OpenThreads::Mutex _updateLock; ///< lock to protect multi-threaded operations
        std::vector<unsigned int> _threadHandButtonMasks;
        std::vector<osg::Matrix> _threadHeadMatList;
        std::vector<osg::Matrix> _threadHandMatList;

        int _numHands; ///< number of hands in system
        int _numHeads; ///< number of heads in system
        std::vector<std::pair<int,int> > _handAddress;
        std::vector<std::pair<int,int> > _headAddress;

        int _totalBodies;
        int _totalButtons;
        int _totalValuators;

        std::vector<osg::Matrix> _headMatList; ///< list of current head matrix transforms
        std::vector<osg::Matrix> _lastUpdatedHeadMatList; ///< used to hold the frozen head positition when head tracking is stopped
        std::vector<osg::Matrix> _handMatList; ///< list of current hand matrix transforms
        std::vector<unsigned int> _handButtonMask; ///< list of current button mask for each hand
        std::vector<unsigned int> _lastHandButtonMask; ///< list of last sampled button mask for each hand
        std::vector<unsigned int> _rawButtonMask; ///< list of current raw button masks from tracker
        std::vector<std::vector<unsigned int> > _handStationFilterMask; ///< collection of masks used to assign buttons to hands
        std::vector<std::vector<float> > _valuatorList; ///< list of current valuator values

        std::vector<std::vector<bool> > _genHandDefaultButtonEvents;

        GenComplexTrackingEvents * genComTrackEvents; ///< class used to create complex interaction events by processing simple ones


        std::map<int,std::list<InteractionEvent*> > _eventMap;
        //std::queue<InteractionEvent *,std::list<InteractionEvent *> > _threadEvents; ///< queue of interaction events generated in threaded mode
        

        //trackedBody * _defaultHead; ///< default value for head orientation
        //trackedBody * _defaultHand; ///< default value for hand orientation

        TrackingInteractionEvent * _currentEvents;
};

/**
 * @brief Takes in tracking events and outputs complex events(ie doubleclick) as needed
 */
class CVRINPUT_EXPORT GenComplexTrackingEvents
{
    public:
        GenComplexTrackingEvents();
        ~GenComplexTrackingEvents();

        void processEvent(TrackingInteractionEvent * tie);
    protected:

        float _doubleClickTimeout;

        std::vector<std::map<int,timeval *> > _lastClick;
        std::vector<std::map<int,bool> > _doubleClicked;
};

}

#endif
