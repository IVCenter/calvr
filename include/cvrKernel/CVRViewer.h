/**
 * @file CVRViewer.h
 */

#ifndef CALVR_VIEWER_H
#define CALVR_VIEWER_H

#include <cvrKernel/Export.h>
#include <cvrKernel/CalVR.h>

#include <osgViewer/Viewer>

#include <list>

namespace cvr
{

struct InteractionEvent;
struct DefaultUpdate;
struct PerContextCallback;

class CVRStatsHandler;

/**
 * @addtogroup kernel
 * @{
 */

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

        enum PreSwapOp
	{
	    PSO_FINISH = 0,
	    PSO_FLUSH,
	    PSO_NONE
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
         * @brief Process interaction events for viewer related actions
         */
        bool processEvent(InteractionEvent * event);

        /**
         * @brief Get if graphics are being rendered on the master node
         */
        bool getRenderOnMaster();

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

        /**
         * @brief Get the screen on the master node where the mouse is currently active
         */
        int getActiveMasterScreen()
        {
            return _activeMasterScreen;
        }

        /**
         * @brief Set if the y widow coord should be inverted for mouse events
         */
        void setInvertMouseY(bool inv)
        {
            _invertMouseY = inv;
        }

        /**
         * @brief Get if the y widow coord should be inverted for mouse events
         */
        bool getInvertMouseY()
        {
            return _invertMouseY;
        }

        PreSwapOp getPreSwapOperation()
        {
            return _preSwapOp;
        }

        void setPreSwapOperation(PreSwapOp pso)
        {
            _preSwapOp = pso;
        }

        /**
         * @brief Get a pointer to the CalVR custom stats handler
         */
        CVRStatsHandler * getStatsHandler()
        {
            return _statsHandler;
        }

        void addPerContextFrameStartCallback(PerContextCallback * pcc);
        int getNumPerContextFrameStartCallbacks();
        PerContextCallback * getPerContextFrameStartCallback(int callback);

        void addPerContextPreDrawCallback(PerContextCallback * pcc);
        int getNumPerContextPreDrawCallbacks();
        PerContextCallback * getPerContextPreDrawCallback(int callback);

        void addPerContextPostFinishCallback(PerContextCallback * pcc);
        int getNumPerContextPostFinishCallbacks();
        PerContextCallback * getPerContextPostFinishCallback(int callback);
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
            DEFAULT, CALVR
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

        /**
         * @brief Custom events to be handled during the viewer event traversal
         */
        enum CustomViewerEventType
        {
            UPDATE_ACTIVE_SCREEN = 1 << 24, UPDATE_VIEWPORT = 1 << 25
        };

        /**
         * @brief information synchronized between nodes at the start of a frame
         */
        struct FrameUpdate
        {
                osg::Timer_t currentTime; ///< frame start time
        };

        static CVRViewer * _myPtr; ///< static self pointer

        osg::ref_ptr<CVRStatsHandler> _statsHandler; ///< custom CalVR stats handler

        std::list<UpdateTraversal*> _updateList; ///< list of all update operations for viewer

        bool _renderOnMaster; ///< should the master render graphics

        osg::Timer_t _programStartTime; ///< time the program started running (distributed)
        osg::Timer_t _lastFrameStartTime; ///< time the last frame started (distributed)
        osg::Timer_t _frameStartTime; ///< time the current frame started (distributed)

        std::map<int,double> _lastClickTime; ///< used to create mouse double click events
        double _doubleClickTimeout; ///< interval, in seconds, for a second click to become a double click

        osg::ref_ptr<osg::BarrierOperation> _cullDrawBarrier; ///< used for threaded rendering
        osg::ref_ptr<osg::BarrierOperation> _swapReadyBarrier;

        CullMode _cullMode; ///< viewer culling mode

        int _activeMasterScreen; ///< screen the mouse is in on the master node

        PreSwapOp _preSwapOp;

        bool _invertMouseY;

        std::vector<PerContextCallback*> _frameStartCallbacks;
        std::vector<PerContextCallback*> _addFrameStartCallbacks;
        std::vector<PerContextCallback*> _preDrawCallbacks;
        std::vector<PerContextCallback*> _addPreDrawCallbacks;

        std::vector<PerContextCallback*> _postFinishCallbacks;
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

struct CVRKERNEL_EXPORT PerContextCallback
{
    public:
        virtual void perContextCallback(int contextid) const
        {
        }
};

/**
 * @}
 */

}

#endif
