/**
 * @file Navigation.h
 */

#ifndef CALVR_NAVIGATION_H
#define CALVR_NAVIGATION_H

#include <kernel/Export.h>
#include <kernel/InteractionManager.h>
#include <kernel/CalVR.h>

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

        void setButtonMode(int button, NavMode nm);

        NavMode getButtonMode(int button);

        /**
         * @brief Set a scale value for the translational movement in the nav modes
         */
        void setScale(float scale);

        /**
         * @brief Get the scale value for the translational movement in the nav modes
         */
        float getScale();

        void setEventActive(bool value, int hand);

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

        bool _eventActive;
        int _activeHand;    ///< hand for the active event

        float _scale;           ///< nav movement scale

        std::map<int,NavImplementationBase*> _navImpMap;
        
        static Navigation * _myPtr;     ///< static self pointer

        std::map<int,NavMode> _buttonMap; ///< map of what navigation mode is set for each button

        std::vector<NavDeviceBase*> _navDeviceList;
};

class NavDeviceBase
{
    public:
        virtual bool init(std::string tagBase) = 0;
        virtual void update() {}
};

class NavImplementationBase
{
    friend class Navigation;
    public:
        virtual void processEvent(InteractionEvent * ie) {}
        virtual void update() {}

    protected:
        int _hand;
};

class NavMouse : public NavImplementationBase
{
    public:
        virtual void processEvent(InteractionEvent * ie);
        virtual void update();
    protected:
        void processMouseNav(NavMode nm, MouseInteractionEvent * mie);

        int _eventButton;
        NavMode _eventMode;
        float _startScale;
        osg::Matrix _startXForm;

        int _eventX;
        int _eventY;
};

class NavTracker : public NavImplementationBase
{
    public:
        virtual void processEvent(InteractionEvent * ie);
    protected:
        void processNav(NavMode nm, osg::Matrix & mat);

        int _eventButton;
        NavMode _eventMode;
        float _startScale;
        osg::Matrix _startXForm;
        osg::Vec3 _eventPos;
        osg::Quat _eventRot;
};

class NavMouseKeyboard : public NavImplementationBase
{
    public:
        NavMouseKeyboard();
        virtual ~NavMouseKeyboard();
        virtual void processEvent(InteractionEvent * ie);
        virtual void update();
    protected:
        void mouseMove(MouseInteractionEvent * mie);

        bool _ctrlDown;
        bool _forwardDown;
        bool _backDown;
        bool _leftDown;
        bool _rightDown;
        bool _scaleUpDown;
        bool _scaleDownDown;

        bool _mouseMove;
        osg::Vec3d _lastDir;
        int _lastScreenNum;
};

}

#endif
