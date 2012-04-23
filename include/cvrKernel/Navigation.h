/**
 * @file Navigation.h
 */

#ifndef CALVR_NAVIGATION_H
#define CALVR_NAVIGATION_H

#include <cvrKernel/Export.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/CalVR.h>

#include <osg/Matrix>
#include <osg/Vec3>

#include <map>
#include <vector>
#include <string>

namespace cvr
{

/**
 * @brief List of navigation/interaction modes
 */
enum NavMode
{
    NONE = 0,
    WALK,
    DRIVE,
    FLY,
    MOVE_WORLD,
    SCALE
};

class NavImplementationBase;
class NavDeviceBase;

/**
 * @brief Uses tracking events to interact with object space
 */
class CVRKERNEL_EXPORT Navigation
{
        friend class CalVR;
    public:

        /**
         * @brief Get static self pointer
         */
        static Navigation * instance();

        /**
         * @brief Do class startup operations
         */
        bool init();

        /**
         * @brief Do per frame operations
         */
        void update();

        /**
         * @brief Process a tracker event into a scale/navigation operation
         */
        void processEvent(InteractionEvent * iEvent);

        /**
         * @brief Set the navigation mode for the main button
         * @param nm New navigation mode to set
         */
        void setPrimaryButtonMode(NavMode nm);

        /**
         * @brief Get the navigation mode for the primary button
         */
        NavMode getPrimaryButtonMode();

        /**
         * @brief Set the navigation mode for the primary button
         */
        void setButtonMode(int button, NavMode nm);

        /**
         * @brief Get the navigation mode for a given button
         */
        NavMode getButtonMode(int button);

        /**
         * @brief Set a scale value for the translational movement in the nav modes
         */
        void setScale(float scale);

        /**
         * @brief Get the scale value for the translational movement in the nav modes
         */
        float getScale();

        /**
         * @brief Set if there is currently active navigation
         * @param value if navigation is active
         * @param hand hand set as active
         */
        void setEventActive(bool value, int hand);

        /**
         * @brief Get is there is currently active navigation
         */
        bool getEventActive() { return _eventActive; }

        /**
         * @brief Navigation Implementation types
         */
        enum NavImplementation
        {
            MOUSE_NAV = 0,
            MOUSEKEYBOARD_NAV,
            TRACKER_NAV,
            NONE_NAV
        };

    protected:
        Navigation();
        virtual ~Navigation();

        bool _eventActive; ///< if there is currenly active navigation
        int _activeHand;    ///< hand for the active event

        float _scale;           ///< nav movement scale

        std::map<int,NavImplementationBase*> _navImpMap; ///< map of hand id to navigation implementation
        
        static Navigation * _myPtr;     ///< static self pointer

        std::map<int,NavMode> _buttonMap; ///< map of what navigation mode is set for each button

        std::vector<NavDeviceBase*> _navDeviceList; ///< list of additional navigation devices to process
};

/**
 * @brief Interface class for a navigation device
 *
 * A navigation device provides a way of navigating that does not use a tracked hand
 */
class NavDeviceBase
{
    public:
        /**
         * @brief Called once, can be used to setup device, etc
         * @param tagBase Config file tag path to the base NavDevice tag
         * @return false indicates init failure
         */
        virtual bool init(std::string tagBase) = 0;

        /**
         * @brief Called once every frame, can be used navigate
         */
        virtual void update() {}
};

/**
 * @brief Interface class for a navigation method that uses CalVR events
 */
class NavImplementationBase
{
    friend class Navigation;
    public:
        /**
         * @brief Allows use of CalVR InteractionEvents for navigation
         */
        virtual void processEvent(InteractionEvent * ie) {}

        /**
         * @brief Called every frame if navigation is active and this hand is set
         *  as the active hand in Navigation
         */
        virtual void update() {}

    protected:
        int _hand; ///< hand id for this instance
};

/**
 * @brief Navigation implementation that uses mouse events 
 */
class NavMouse : public NavImplementationBase
{
    public:
        virtual void processEvent(InteractionEvent * ie);
        virtual void update();
    protected:
        /**
         * @brief Use mouse event for navigation
         */
        void processMouseNav(NavMode nm, MouseInteractionEvent * mie);

        int _eventButton; ///< button for active event
        NavMode _eventMode; ///< mode for active event
        float _startScale; ///< scale at start of event
        osg::Matrix _startXForm; ///< object space transform at start of event

        int _eventX; ///< x viewport value at start of event
        int _eventY; ///< y viewport value at start of event
};

/**
 * @brief Navigation implementation that uses a tracked hand device
 */
class NavTracker : public NavImplementationBase
{
    public:
        virtual void processEvent(InteractionEvent * ie);
    protected:
        /**
         * @brief Use a hand orientation to create navigation
         */
        void processNav(NavMode nm, osg::Matrix & mat);

        int _eventButton; ///< hand button for event
        NavMode _eventMode; ///< mode for active event
        float _startScale; ///< scale at start of event
        osg::Matrix _startXForm; ///< object space transform at start of event
        osg::Vec3 _eventPos; ///< hand position at start of event
        osg::Quat _eventRot; ///< hand rotation at start of event
};

/**
 * @brief Navigation implementation that uses a mouse and keyboard
 */
class NavMouseKeyboard : public NavImplementationBase
{
    public:
        NavMouseKeyboard();
        virtual ~NavMouseKeyboard();
        virtual void processEvent(InteractionEvent * ie);
        virtual void update();
    protected:
        /**
         * @brief Process a mouse movement into navigation
         */
        void mouseMove(MouseInteractionEvent * mie);

        bool _ctrlDown; ///< is the control key down
        bool _forwardDown; ///< is the 'w' key down
        bool _backDown; ///< is the 's' key down
        bool _leftDown; ///< is the 'a' key down
        bool _rightDown; ///< is the 'd' key down
        bool _scaleUpDown; ///< is the scale up key down
        bool _scaleDownDown; ///< is the scale down key down

        bool _mouseMove; ///< is there a mouse movement active
        osg::Vec3d _lastDir; ///< last mouse direction
        int _lastScreenNum; ///< master screen id for last mouse movement event
};

}

#endif
