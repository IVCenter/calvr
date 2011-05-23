/**
 * @file
 * @brief Handle collection and access to tracking information
 * @ingroup input
 * @author Andrew Prudhomme (aprudhomme@ucsd.edu)
 */

#ifndef CALVR_TRACKING_MANAGER_H
#define CALVR_TRACKING_MANAGER_H

#include <input/Export.h>
#include <input/TrackerBase.h>

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
    public:
        virtual ~TrackingManager();

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
         * @brief Return if wand graphic should be visible
         */
        bool getShowWand();

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
         * @brief Returns number of button stations provided by the button tracker
         */
        int getNumButtonStations();

        /**
         * @brief Returns number of buttons present in a given button station
         */
        int getNumButtons(int station = 0);

        /**
         * @brief Returns the raw button mask for a given button station
         */
        unsigned int getRawButtonMask(int station = 0);

        /**
         * @brief Returns the button mask for a given hand, which is processed from the raw mask(s)
         */
        unsigned int getHandButtonMask(int hand = 0);

        /**
         * @brief Returns the number of valuator stations provided by the tracker
         */
        int getNumValuatorStations();

        /**
         * @brief Returns the number of valuators in a given valuator station
         */
        int getNumValuators(int station = 0);

        /**
         * @brief Returns the value of the valuator in a given station with a given index
         */
        float getValuator(int station, int index);

        bool getUsingMouseTracker();

    protected:
        TrackingManager();

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

        /**
         * @brief Infomation sent to render nodes during \c TrackingManager init
         */
        struct TrackingManInit
        {
                int numHands; ///< number of hands being tracked
                int numHeads; ///< number of heads being tracked
                int totalBodies; ///< total number of 6dof bodies being tracked
                int buttonStations; ///< number of tracker button stations
                int valStations; ///< number of tracker valuator stations
                bool showWand; ///< should wand graphics be visible
        };

        static TrackingManager * _myPtr; ///< Static self pointer

        bool _threaded; ///< is there a thread polling the tracker
        OpenThreads::Mutex _updateLock; ///< lock to protect multi-threaded operations
        std::queue<InteractionEvent *,std::list<InteractionEvent *> >
                _threadEvents; ///< queue of interaction events generated in threaded mode
        GenComplexTrackingEvents * genComTrackEvents; ///< class used to create complex interaction events by processing simple ones

        std::vector<osg::Matrix> _headMatList; ///< list of current head matrix transforms
        std::vector<int> _headStations; ///< list of tracker station numbers used for head bodies
        std::vector<osg::Matrix> _handMatList; ///< list of current hand matrix transforms
        std::vector<int> _handStations; ///< list of tracker station numbers used for hand bodies
        std::vector<int> _handButtonStations;
        std::vector<int> _handButtonOffsets;
        std::vector<unsigned int> _handButtonMask; ///< list of current button mask for each hand
        std::vector<unsigned int> _lastHandButtonMask; ///< list of last sampled button mask for each hand
        std::vector<unsigned int> _rawButtonMask; ///< list of current raw button masks from tracker
        std::vector<std::vector<float> > _valuatorList; ///< list of current valuator values
        std::vector<std::vector<unsigned int> > _handStationFilterMask; ///< collection of masks used to assign buttons to hands

        int _numHands; ///< number of hands in system
        int _numHeads; ///< number of heads in system
        bool _showWand; ///< should wand graphic be visible

        bool _mouseTracker;

        TrackerBase * _buttonTracker; ///< tracking system that provides the button information
        TrackerBase * _bodyTracker; ///< tracking system that provides the body tracking information

        trackedBody * _defaultHead; ///< default value for head orientation
        trackedBody * _defaultHand; ///< default value for hand orientation

        osg::Matrix _systemTransform; ///< transform for the entire tracking system
        std::vector<osg::Matrix> _handTransformsRot;
        std::vector<osg::Vec3> _handTransformsTrans;
        std::vector<osg::Matrix> _headTransformsRot;
        std::vector<osg::Vec3> _headTransformsTrans;

        bool _debugOutput;

        int _totalButtons;
        int _totalValuators;

        float _threadFPS;
        bool _threadQuit;
        OpenThreads::Mutex _quitLock;
        std::vector<unsigned int> _threadHandButtonMasks;
        std::vector<osg::Matrix> _threadHeadMatList;
        std::vector<osg::Matrix> _threadHandMatList;
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
