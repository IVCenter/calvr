/**
 * @file CVRViewer.h
 */

#ifndef CALVR_VIEWER_H
#define CALVR_VIEWER_H

#include <kernel/Export.h>
#include <kernel/CalVR.h>

#include <osgViewer/Viewer>

#include <list>

namespace cvr
{

struct InteractionEvent;
struct DefaultUpdate;

/**
 * @brief Main viewer for the system, does rendering traversal, distributes window events
 */
class CVRKERNEL_EXPORT CVRViewer : public osgViewer::Viewer
{
    friend class CalVR;
    friend struct DefaultUpdate;
    public:
        CVRViewer();
        CVRViewer(osg::ArgumentParser& arguments);
        CVRViewer(const CVRViewer& viewer, const osg::CopyOp& copyop =
                osg::CopyOp::SHALLOW_COPY);

        /**
         * @brief Interface structure to add a custom function call to the
         *      viewer's update traversal
         */
        struct UpdateTraversal
        {
            public:
                /**
                 * @brief Function called during viewer update traversal
                 */
                virtual void update() = 0;
        };

        /**
         * @brief Get pointer to static instance of class
         */
        static CVRViewer * instance();

        /**
         * @brief Processes all update operations added to the update list
         */
        virtual void updateTraversal();

        /**
         * @brief Creates and distributes window events(mouse,keyboard)
         */
        virtual void eventTraversal();

        /**
         * @brief Rendering loop
         */
        virtual void renderingTraversals();

        /**
         * @brief Start threads for threaded rendering
         */
        virtual void startThreading();

        /**
         * @brief Do actions that should be done at the start of each frame
         */
        void frameStart();

        /**
         * @brief Add a custom update operation to the front of the update 
         *      traversal list
         */
        void addUpdateTraversalFront(UpdateTraversal * ut);

        /**
         * @brief Add a custom update operation to the end of the update 
         *      traversal list
         */
        void addUpdateTraversalBack(UpdateTraversal * ut);

        /**
         * @brief Remove a custom update operation from the update traversal
         */
        void removeUpdateTraversal(UpdateTraversal * ut);

        /**
         * @brief Set if the viewer information should be updated
         */
        void setStopHeadTracking(bool b);

        /**
         * @brief Process interaction events for viewer related actions
         */
        bool processEvent(InteractionEvent * event);

        /**
         * @brief Get if graphics are being rendered on the master node
         */
        bool getRenderOnMaster();

        /**
         * @brief Get if head tracking is being used
         */
        bool getStopHeadTracking();

        /**
         * @brief Get the time spent for the last frame loop
         * @return time in seconds
         */
        double getLastFrameDuration();

        /**
         * @brief Get the time between the program start and the start of
         *      the current frame
         * @return time in seconds
         */
        double getProgramDuration();

        /**
         * @brief Get the time for the start of the current frame
         * @return The number of second between Jan 1 1970 and the frame start
         */
        double getFrameStartTime();

        /**
         * @brief Get the time for the start of the program
         * @return The number of second between Jan 1 1970 and the program start
         */
        double getProgramStartTime();

        int getActiveMasterScreen() { return _activeMasterScreen; }
    protected:
        virtual ~CVRViewer();

        /**
         * @brief Default osg update traversal operation
         */
        void defaultUpdateTraversal();

        /**
         * @brief Culling mode used by the viewer
         */
        enum CullMode
        {
            DEFAULT,
            CALVR
        };

        struct eventInfo
        {
                int numEvents;
                int viewportX;
                int viewportY;
                float width;
                float height;
                float x, y, z;
        };

        struct event
        {
                int eventType;
                int param1;
                int param2;
        };

        enum CustomViewerEventType
        {
            UPDATE_ACTIVE_SCREEN = 1<<24,
            UPDATE_VIEWPORT = 1<<25
        };

        /**
         * @brief information synchronized between nodes at the start of a frame
         */
        struct FrameUpdate
        {
                osg::Timer_t currentTime; ///< frame start time
        };

        static CVRViewer * _myPtr; ///< static self pointer

        std::list<UpdateTraversal*> _updateList; ///< list of all update operations for viewer

        bool _renderOnMaster; ///< should the master render graphics
        bool _stopHeadTracking; ///< should we use head tracking

        osg::Timer_t _programStartTime; ///< time the program started running (distributed)
        osg::Timer_t _lastFrameStartTime; ///< time the last frame started (distributed)
        osg::Timer_t _frameStartTime; ///< time the current frame started (distributed)

        std::map<int,double> _lastClickTime; ///< used to create mouse double click events
        double _doubleClickTimeout; ///< interval, in seconds, for a second click to become a double click

        osg::ref_ptr<osg::BarrierOperation> _cullDrawBarrier; ///< used for threaded rendering

        CullMode _cullMode; ///< viewer culling mode

        int _activeMasterScreen;
};

/**
 * @brief Update operation for the default osg update traversal
 *
 * Added to the viewer by default
 */
struct CVRKERNEL_EXPORT DefaultUpdate : public CVRViewer::UpdateTraversal
{
    public:
        void update();
};

}

#endif
