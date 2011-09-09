/**
 * @file InteractionManager.h
 */
#ifndef CALVR_INTERACTION_MANAGER_H
#define CALVR_INTERACTION_MANAGER_H

#include <kernel/Export.h>
#include <kernel/CVRViewer.h>
#include <kernel/CalVR.h>

#include <osg/Vec3>
#include <osg/Matrix>

#include <OpenThreads/Mutex>

#include <queue>
#include <list>

namespace cvr
{

/** \public
 * @brief List off all different interaction events
 */
enum InteractionType
{
    BUTTON_DOWN = 0x10000000,
    BUTTON_UP = 0x10000001,
    BUTTON_DRAG = 0x10000002,
    BUTTON_DOUBLE_CLICK = 0x10000003,
    MOUSE_DRAG = 0x08000000,
    MOUSE_BUTTON_UP = 0x08000001,
    MOUSE_BUTTON_DOWN = 0x08000002,
    MOUSE_DOUBLE_CLICK = 0x08000003,
    KEY_UP = 0x04000000,
    KEY_DOWN = 0x04000001
};

enum EventType
{
    TRACKING_EVENT = 0x10000000,
    MOUSE_EVENT = 0x08000000,
    KEYBOARD_EVENT = 0x04000000
};

/**
 * @brief Base interaction event struct, all other inherit from this
 */
struct InteractionEvent
{
        InteractionType type;
};

/**
 * @brief Structure for a mouse event
 */
struct MouseInteractionEvent : public InteractionEvent
{
        int button; ///< button for this event
        int x; ///< mouse viewport x value
        int y; ///< mouse viewport y value
        osg::Matrix transform; ///< mouse orientation transform
};

/**
 * @brief Structure for a tracking system event
 */
struct TrackingInteractionEvent : public InteractionEvent
{
        int button; ///< button for event
        int hand; ///< hand for the button
        float xyz[3]; ///< hand translation
        float rot[4]; ///< hand rotation (quat)
};

/**
 * @brief Structure for a keyboard event
 */
struct KeyboardInteractionEvent : public InteractionEvent
{
        int key; ///< key for the event
        int mod; ///< modifier for keypress
};

CVRKERNEL_EXPORT osg::Matrix tie2mat(TrackingInteractionEvent * tie);

/**
 * @brief Directs events through interaction pipeline, manages event queue
 */
class CVRKERNEL_EXPORT InteractionManager
{
        friend class CVRViewer;
        friend class CalVR;
    public:

        /**
         * @brief Get a static pointer to the instance of the class
         */
        static InteractionManager * instance();

        /**
         * @brief Called after creation
         */
        bool init();

        /**
         * @brief Does per frame operations
         */
        void update();

        /**
         * @brief Flushes event queue
         */
        void handleEvents();

        /**
         * @brief Sends a single event through the event pipeline
         */
        void handleEvent(InteractionEvent * event);

        /**
         * @brief Adds an event to the event queue
         */
        void addEvent(InteractionEvent * event);

        /**
         * @brief Sets the current mouse state
         * @param x viewport x value
         * @param y viewport y value
         */
        void setMouse(int x, int y);

        /**
         * @brief Sets the current mouse button status
         * @param mask button state
         */
        void setMouseButtonMask(unsigned int mask);

        /**
         * @brief Get the current mouse button state
         */
        unsigned int getMouseButtonMask();

        /**
         * @brief returns false if there has not yet been a valid mouse event
         */
        bool mouseActive();

        /**
         * @brief get the current mouse orientation
         */
        osg::Matrix & getMouseMat();

        /**
         * @brief get the current mouse x viewport position
         */
        int getMouseX();

        /**
         * @brief get the current mouse y viewport position
         */
        int getMouseY();

    protected:
        InteractionManager();
        virtual ~InteractionManager();

        /// queue for events, flushed every frame
        std::queue<InteractionEvent *,std::list<InteractionEvent *> >
                _eventQueue;
        OpenThreads::Mutex _queueLock; ///< lock for queue add/removes

        static InteractionManager * _myPtr; ///< static self pointer

        /**
         * @brief Create mouse button events from current and last button masks
         */
        void processMouse();

        /**
         * @brief Create mouse drag events from current button mask
         */
        void createMouseDragEvents();

        void createMouseDoubleClickEvent(int button);

        bool _mouseEvents;
        bool _mouseTracker;

        unsigned int _lastMouseButtonMask; ///< last used mouse button mask
        unsigned int _mouseButtonMask; ///< current mouse button mask
        //MouseInfo * _mouseInfo; ///< current mouse state
        bool _mouseActive; ///< have we had a mouse event
        osg::Matrix _mouseMat; ///< mouse orientation
        int _mouseX; ///< current mouse x position
        int _mouseY; ///< current mouse y position

};

}

#endif
