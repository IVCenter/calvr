/**
 * @file
 * @author Andrew Prudhomme (aprudhomme@ucsd.edu)
 */

#ifndef CALVR_TRACKING_MANAGER_H
#define CALVR_TRACKING_MANAGER_H

#include <cvrInput/Export.h>
#include <cvrInput/TrackerBase.h>
#include <cvrKernel/CalVR.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/SceneManager.h>

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

class InteractionEvent;
class TrackedButtonInteractionEvent;
class GenComplexTrackingEvents;

/**
 * @addtogroup input
 * @{
 */

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

        /**
         * @brief Get the head id for a given hand
         */
        int getHeadForHand(int hand);

        /**
         * @brief Get all hands linked to a given head
         */
        const std::vector<int> & getHandsForHead(int head);

        /**
         * @brief Get the number of tracking systems that are being managed
         */
        int getNumTrackingSystems();

        /**
         * @brief Get the tracker implementation for a given tracking system number
         * @return NULL is returned if value is out of range or the system failed
         * during initialization
         */
        TrackerBase * getTrackingSystem(int system);

        /**
         * @brief Get the system and body for a given hand
         *
         * If the hand number is invalid, system and body are set to -1
         */
        void getHandAddress(int hand, int & system, int & body);

        /**
         * @brief Get the system and body for a given head
         *
         * If the head number is invalid, system and body are set to -1
         */
        void getHeadAddress(int head, int & system, int & body);

        /**
         * @brief Get the tracker type used by the tracking system driving
         * the given hand
         */
        TrackerBase::TrackerType getHandTrackerType(int hand);

        /**
         * @brief Get the navigation type to use for a given hand
         */
        Navigation::NavImplementation getHandNavType(int hand);

        int getNumBodies(int system = 0);

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

        unsigned int getButtonFilter(int hand, int system);

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

        osg::Matrix getHandTransformFromTrackedBody(int hand,
                TrackerBase::TrackedBody * body);
        osg::Matrix getHeadTransformFromTrackedBody(int head,
                TrackerBase::TrackedBody * body);
        void getHandButtonFromSystemButton(int system, int systemButton,
                int & hand, int & handButton);
        void setIntersectPoint(osg::Vec3f point){_isPoint = point;isIntersect = true;}
        bool getIsPoint(osg::Vec3f & isPoint);
        void setWindowsShape(int width, int height){_screenWidth = width; _screenHeight= height;}
        void getScreenToClientPos(osg::Vec2f & pos){pos = osg::Vec2f(2*(pos.x() - _screenWidth/2) / _screenWidth,
                                                                        2*(_screenHeight/2 - pos.y()) / _screenHeight);}
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
         * @brief Generate valuator events for non-threaded hands
         */
        void generateValuatorEvents();

        /**
         * @brief Generate valuator events for threaded hands
         */
        void generateThreadValuatorEvents();

        /**
         * @brief Generate position events for non-threaded hands
         */
        void generatePositionEvents();

        /**
         * @brief Generate position events for threaded hands
         */
        void generateThreadPositionEvents();

        /**
         * @brief Update thread's local head/hand matrix transforms from tracker
         */
        void updateThreadMats();

        /**
         * @brief Sync and flush thread generated event queue into interaction pipeline
         */
        void flushEvents();

        /**
         * @brief Init which hand buttons should have default button events
         * generated
         */
        void setGenHandDefaultButtonEvents();

        void setHandButtonMaps();

        /**
         * @brief Get if a given hand is being updated by the tracking thread
         */
        bool getIsHandThreaded(int hand);

        void printInitDebug();

        /**
         * @brief The information associated with a tracking system
         */
        struct TrackingSystemInfo
        {
                int numBodies; ///< number of tracked bodies
                int numButtons; ///< number of buttons
                int numVal; ///< number of valuators
                osg::Matrix systemTransform; ///< transform on the entire tracking system
                std::vector<osg::Matrix> bodyRotations; ///< rotation adjustment for each tracked body
                std::vector<osg::Vec3> bodyTranslations; ///< position adjustment for each tracked body
                Navigation::NavImplementation navImp; ///< navigation type for the system
                TrackerBase::TrackerType trackerType; ///< system type
                SceneManager::PointerGraphicType defaultPointerType; ///< type of graphic to use for pointer
                bool genDefaultButtonEvents; ///< should default buttons events be generated
                bool thread; ///< should this system be polled in a thread
        };

        enum ValuatorType
        {
            NON_ZERO, CHANGE
        };

        static TrackingManager * _myPtr; ///< Static self pointer

        std::vector<TrackerBase*> _systems; ///< List of all tracking systems
        std::vector<TrackingSystemInfo*> _systemInfo; ///< List of information for each system

        bool _debugOutput; ///< should debug output be printed
        bool _updateHeadTracking; ///< is head tracking being updated

        bool _threaded; ///< is there a thread polling the tracker
        float _threadFPS; ///< target frames per second of thread polling a tracking system
        bool _threadQuit; ///< quit flag for thread
        OpenThreads::Mutex _quitLock; ///< lock to protect quit flag
        OpenThreads::Mutex _updateLock; ///< lock to protect multi-threaded operations
        std::vector<unsigned int> _threadHandButtonMasks; ///< list of button mask for each hand in threaded update
        std::vector<unsigned int> _threadLastHandButtonMask; ///< list of last sampled button mask for each hand in threaded update
        std::vector<osg::Matrix> _threadHandMatList; ///< list of hand transforms for each hand in threaded update

        int _numHands; ///< number of hands in system
        int _numHeads; ///< number of heads in system
        std::vector<std::pair<int,int> > _handAddress; ///< address of each hand - system, index pair
        std::vector<std::pair<int,int> > _headAddress; ///< address of each head - system, index pair
        std::map<int,std::vector<std::pair<int,int> > > _handButtonAddressMap;
        std::vector<std::map<int,int> > _handButtonMapping;

        int _totalBodies; ///< total number of all tracked bodies in all systems
        int _totalButtons; ///< total number of buttons in all systems
        int _totalValuators; ///< total number of valuators in all systems

        std::vector<osg::Matrix> _headMatList; ///< list of current head matrix transforms
        std::vector<osg::Matrix> _lastUpdatedHeadMatList; ///< used to hold the frozen head positition when head tracking is stopped
        std::vector<osg::Matrix> _handMatList; ///< list of current hand matrix transforms
        std::vector<unsigned int> _handButtonMask; ///< list of current button mask for each hand
        std::vector<unsigned int> _lastHandButtonMask; ///< list of last sampled button mask for each hand
        std::vector<unsigned int> _rawButtonMask; ///< list of current raw button masks from tracker
        std::vector<std::vector<unsigned int> > _handStationFilterMask; ///< collection of masks used to assign buttons to hands
        std::vector<SceneManager::PointerGraphicType> _handGraphicType; ///< the graphic to use for display of a hand
        std::vector<Navigation::NavImplementation> _handNavImplementation; ///< navigation type used for each hand
        std::vector<bool> _handGenPositionEvents; ///< if the tracking system should create position events for a hand
        std::vector<std::vector<float> > _valuatorList; ///< list of current valuator values

        std::vector<int> _numEventValuators; ///< number of valuator for each hand
        std::vector<std::vector<std::pair<int,int> > > _eventValuatorAddress; ///< system/index pair address for each hand valuator
        std::vector<std::vector<ValuatorType> > _eventValuatorType; ///< type of each valuator
        std::vector<std::vector<float> > _eventValuators; ///< current value of each valuator
        std::vector<std::vector<double> > _eventValuatorTime; ///< time since last valuator event was created
        std::vector<std::map<int,float> > _lastEventValuators; ///< previous value of valuator, used for CHANGE type

        std::vector<std::vector<bool> > _genHandDefaultButtonEvents; ///< lookup to see if a hand button should have default events generated

        GenComplexTrackingEvents * genComTrackEvents; ///< class used to create complex interaction events by processing simple ones

        std::map<int,std::list<InteractionEvent*> > _eventMap; ///< map of events generated by the tracking system, used for cluster distribution

        std::vector<int> _handToHeadMap; ///< map of hand number to head number
        std::vector<std::vector<int> > _headToHandsMap; ///< map of head number to hand numbers
        osg::Matrix _touchEventMat;
        osg::Matrix _cameraRotMat;
        double cam_eu_x, cam_eu_y, cam_eu_z;
        osg::Vec3f _isPoint;
        bool isIntersect = false;
        int _screenWidth, _screenHeight;
        float _touchX, _touchY;
};

/**
 * @brief Takes in tracking events and outputs complex events(ie doubleclick) as needed
 */
class CVRINPUT_EXPORT GenComplexTrackingEvents
{
    public:
        GenComplexTrackingEvents();
        ~GenComplexTrackingEvents();

        /**
         * @brief Takes in tracking events and generated more complex events,
         * like double clicks
         */
        void processEvent(TrackedButtonInteractionEvent * tie);
    protected:

        float _doubleClickTimeout; ///< time to wait for a double click

        std::vector<std::map<int,timeval *> > _lastClick; ///< time of last click
        std::vector<std::map<int,bool> > _doubleClicked; ///< double click state
};

/**
 * @}
 */

}

#endif
