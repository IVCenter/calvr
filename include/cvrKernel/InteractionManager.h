/**
 * @file InteractionManager.h
 */
#ifndef CALVR_INTERACTION_MANAGER_H
#define CALVR_INTERACTION_MANAGER_H

#include <cvrKernel/Export.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/CalVR.h>
#include <cvrKernel/InteractionEvent.h>

#include <osg/Vec3>
#include <osg/Matrix>

#include <OpenThreads/Mutex>

#include <queue>
#include <list>

namespace cvr
{

class TrackerMouse;

/**
 * @brief Directs events through interaction pipeline, manages event queue
 */
class CVRKERNEL_EXPORT InteractionManager
{
        friend class CVRViewer;
        friend class CalVR;
        friend class TrackerMouse;
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

        /**
         * @brief Set the state of the mouse wheel
         *
         * Valid values are -1, 0, 1
         */
        void setMouseWheel(int w);

        /**
         * @brief Get the state of the mouse wheel
         *
         * Valid values are -1, 0, 1
         */
        int getMouseWheel() { return _mouseWheel; }

    protected:
        InteractionManager();
        virtual ~InteractionManager();

        /// queue for events, flushed every frame
        std::queue<InteractionEvent *,std::list<InteractionEvent *> > _eventQueue;

        /// queue for mouse events, read by TrackerMouse, flushed each frame
        std::queue<InteractionEvent *,std::list<InteractionEvent *> > _mouseQueue;
        OpenThreads::Mutex _queueLock; ///< lock for queue add/removes

        static InteractionManager * _myPtr; ///< static self pointer

        /**
         * @brief Create mouse button events from current and last button masks
         */
        void processMouse();

        /**
         * @brief Create mouse drag events from current button mask
         */
        void createMouseDragEvents(bool single);

        /**
         * @brief Create DOUBLE_CLICK MouseInteractionEvent for a given button
         */
        void createMouseDoubleClickEvent(int button);

        /**
         * @brief Check if the mouse wheel timeout has expired and the wheel state
         *  should be set back to zero
         */
        void checkWheelTimeout();

        unsigned int _lastMouseButtonMask; ///< last used mouse button mask
        unsigned int _mouseButtonMask; ///< current mouse button mask
        //MouseInfo * _mouseInfo; ///< current mouse state
        bool _mouseActive; ///< have we had a mouse event
        osg::Matrix _mouseMat; ///< mouse orientation
        int _mouseX; ///< current mouse x position
        int _mouseY; ///< current mouse y position
        int _mouseHand; ///< hand id to use for mouse events

        double _mouseWheelTimeout; ///< time the mouse wheel will stay active without an event
        double _mouseWheelTime; ///< time since last mouse wheel event
        int _mouseWheel; ///< current mouse wheel state

        double _dragEventTime; ///< time since last drag event
};

}

#endif
