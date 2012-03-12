/**
 * @file InteractionManager.h
 */
#ifndef CALVR_INTERACTION_MANAGER_H
#define CALVR_INTERACTION_MANAGER_H

#include <kernel/Export.h>
#include <kernel/CVRViewer.h>
#include <kernel/CalVR.h>
#include <kernel/InteractionEvent.h>

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

        void setMouseWheel(int w);
        int getMouseWheel() { return _mouseWheel; }

    protected:
        InteractionManager();
        virtual ~InteractionManager();

        /// queue for events, flushed every frame
        std::queue<InteractionEvent *,std::list<InteractionEvent *> > _eventQueue;

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

        void createMouseDoubleClickEvent(int button);

        void checkWheelTimeout();

        bool _mouseEvents;
        bool _mouseTracker;

        unsigned int _lastMouseButtonMask; ///< last used mouse button mask
        unsigned int _mouseButtonMask; ///< current mouse button mask
        //MouseInfo * _mouseInfo; ///< current mouse state
        bool _mouseActive; ///< have we had a mouse event
        osg::Matrix _mouseMat; ///< mouse orientation
        int _mouseX; ///< current mouse x position
        int _mouseY; ///< current mouse y position
        int _mouseHand;

        double _mouseWheelTimeout;
        double _mouseWheelTime;
        int _mouseWheel;

        double _dragEventTime;
};

}

#endif
