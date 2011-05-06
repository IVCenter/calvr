/**
 * @file Navigation.h
 */

#ifndef CALVR_NAVIGATION_H
#define CALVR_NAVIGATION_H

#include <kernel/InteractionManager.h>

#include <osg/Matrix>
#include <osg/Vec3>

#include <map>

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

/**
 * @brief Uses tracking events to interact with object space
 */
class Navigation
{
    public:
        ~Navigation();

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
         * @brief Process a mouse event into a scale/navigation operation
         */
        void processMouseEvent(MouseInteractionEvent * event);

        /**
         * @brief Set the navigation mode for the main button
         * @param nm New navigation mode to set
         */
        void setPrimaryButtonMode(NavMode nm);

        /**
         * @brief Get the navigation mode for the primary button
         */
        NavMode getPrimaryButtonMode();

        void setScale(float scale);
        float getScale();

    protected:
        Navigation();
        void processNav(NavMode nm, osg::Matrix & mat);
        void processMouseNav(NavMode nm);

        static Navigation * _myPtr;     ///< static self pointer

        std::map<int,NavMode> _buttonMap;   ///< map of what navigation mode is set for each button
        bool _eventActive;  ///< is there a navigation event in progress
        int _activeHand;    ///< hand for the active event
        int _eventID;       ///< button for the active event
        NavMode _eventMode; ///< mode for the active event

        osg::Vec3 _eventPos;    ///< world space position at start of event
        osg::Quat _eventRot;    ///< rotation at start of event
        float _startScale;      ///< scale at start of event
        osg::Matrix _startXForm;    ///< object space transform at start of event

        bool _mouseEventActive; ///< is a mouse navigation event started
        int _eventX;            ///< mouse viewport x coord at start of event
        int _eventY;            ///< mouse viewport y coord at start of event

        float _scale;
};

}

#endif
