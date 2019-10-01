/**
 * @file PluginHelper.h
 */

#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

#include <cvrKernel/Export.h>
#include <cvrKernel/SceneManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrMenu/MenuSystem.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/InteractionManager.h>

#include <iostream>

#include <osg/MatrixTransform>
#include <osg/ClipNode>
#include <osg/Matrix>

namespace cvr
{

/**
 * @addtogroup kernel
 * @{
 */

/**
 * @brief Provides centralized access to various framework functions
 *
 * This class is designed to simplify plugin programing by grouping key
 * functions into a single place.  The functions themselves simply pass the 
 * call along to the correct class.
 */
class CVRKERNEL_EXPORT PluginHelper
{
    public:
        PluginHelper();
        ~PluginHelper();

        // scene
        /**
         * @brief Get the root scene transform (world space)
         */
        static osg::MatrixTransform * getScene();

        /**
         * @brief Get the root object node (navigation space)
         */
        static osg::ClipNode * getObjectsRoot();

        /**
         * @brief Get node containing the current object space
         *        position/orientation
         */
        static const osg::MatrixTransform * getObjectTransform();

        /**
         * @brief Get matrix containing the current object space
         *        position/orientation
         */
        static const osg::Matrix & getObjectMatrix();

        /**
         * @brief Set the object space position/orientation
         * @param mat Matrix that represents the new object space 
         *            transform
         */
        static void setObjectMatrix(osg::Matrix & mat);

        /**
         * @brief Get the scale transform value for object space
         */
        static float getObjectScale();

        /**
         * @brief Set the scale transform value for object space
         * @param scale The new scale for object space
         */
        static void setObjectScale(float scale);

        /**
         * @brief Gets the matrix transform from object space to
         *      world space
         */
        static const osg::Matrix & getWorldToObjectTransform();

        /**
         * @brief Gets the matrix transform from world space to
         *      object space
         */
        static const osg::Matrix & getObjectToWorldTransform();

        // tracking
        /**
         * @brief Get the number of hand targets being tracked
         */
        static int getNumHands();

        /**
         * @brief Get the hand to world space transform
         * @param hand Hand to get transform for
         */
        static osg::Matrix & getHandMat(int hand = 0);

		/**
		 * @brief Get the hand object so that things may be attached/detached
		 */
		static osg::MatrixTransform * getHand(unsigned int hand = 0);

        /**
         * @brief Get the number of head targets being tracked
         */
        static int getNumHeads();

        /**
         * @brief Get the head to world space transform
         * @param head Head to get transform for
         */
        static osg::Matrix & getHeadMat(int head = 0);

        /**
         * @brief get the number of loaded tracking systems
         */
        static int getNumTrackingSystems();

        /**
         * @brief Get the number of buttons in a given tracking system
         */
        static int getNumButtons(int system = 0);

        /**
         * @brief Get the mask repesenting the current button state
         * @param system Tracking system to get mask for
         *
         * Each bit represents a button state with the least significant 
         * being button 0. ie a value of 5 means buttons 0 and 2 are down
         */
        static unsigned int getRawButtonMask(int system = 0);

        /**
         * @brief Get the mask repesenting the current button state for each
         *        hand
         * @param hand Hand to get mask for
         *
         * A mask can be specified in the config file to specify which buttons 
         * are assigned to which hands.  Each hand then has a set of logical 
         * buttons 0 through n with states represented by the returned mask
         */
        static unsigned int getHandButtonMask(int hand = 0);

        /**
         * @brief Get the number of valuator values in a given tracking system
         */
        static int getNumValuators(int system = 0);

        /**
         * @brief Get the current value of a valuator
         * @param system Tracking system containing valuator
         * @param index Index of valuator within the station
         */
        static float getValuator(int system, int index);

        //mouse
        /**
         * @brief Get the current viewport X value for the mouse
         */
        static int getMouseX();

        /**
         * @brief Get the current viewport Y value for the mouse
         */
        static int getMouseY();

        /**
         * @brief Get the current mouse to world space transform
         */
        static osg::Matrix & getMouseMat();

        //menu
        /**
         * @brief Add an item to the main menu
         * @param item Menu item to add
         */
        static void addRootMenuItem(MenuItem * item);

        //time
        /**
         * @brief Get the time taken for the last frame, in seconds
         */
        static double getLastFrameDuration();

        /**
         * @brief Get the time between the start of the program and the start
         *        of the current frame
         */
        static double getProgramDuration();

        /**
         * @brief Get the time for the start of the current frame
         * @return The number of second between Jan 1 1970 and the frame start
         */
        static double getFrameStartTime();

        /**
         * @brief Get the time for the start of the program
         * @return The number of second between Jan 1 1970 and the program start
         */
        static double getProgramStartTime();

        //screen
        /**
         * @brief Set the opengl background clear color
         * @param color RGBA color to set
         */
        static void setClearColor(osg::Vec4 color);

        /**
         * @brief Get the number of screens(viewports) created on this node
         */
        static int getNumScreens();

        /**
         * @brief Get information for a given screen
         */
        static ScreenInfo * getScreenInfo(int screen);

        /**
         * @brief Register a SceneObject with the SceneManager
         * @param object SceneObject to register
         * @param plugin optional plugin name to associate with the object
         *
         * A SceneObject must be registered before it can be attached to the scene
         */
        static void registerSceneObject(SceneObject * object,
                std::string plugin = "");

        /**
         * @brief Unregister a SceneObject with the SceneManager
         */
        static void unregisterSceneObject(SceneObject * object);

        /**
         * @brief Sends a message to a plugin with the given name through the message() callback in the plugin interface
         * @param plugin name of plugin to send message to
         * @param type value the plugin gets as the message type
         * @param data pointer to any message data
         *
         * If the plugin is not on, nothing happens.  This is a local call and no data copies or movement is involved.
         */
        static void sendMessageByName(std::string plugin, int type,
                char * data);

        /**
         * @brief Sends a message to a plugin in a collaborative session (asynchronous)
         * @param plugin plugin to send message to
         * @param type value the plugin gets as the message type
         * @param data data to send for this message
         * @param size size of data to send
         * @param sendLocal if true, this message is also sent to the local instance of this plugin using the standard
         *        message interface
         *
         * This call is asynchronous and the message will not likely be sent for at least a frame.  The data is assumed to be
         * disposable and the buffer is deleted automatically after the message is sent.  If sendLocal is true, it happens before
         * the function returns.
         */
        static void sendCollaborativeMessageAsync(std::string plugin, int type,
                char * data, int size, bool sendLocal = false);

        /**
         * @brief Sends a message to a plugin in a collaborative session (synchronous)
         * @param plugin plugin to send message to
         * @param type value the plugin gets as the message type
         * @param data data to send for this message
         * @param size size of data to send
         * @param sendLocal if true, this message is also sent to the local instance of this plugin using the standard
         *        message interface
         *
         * This is a synchronous call.  The function does not return until the message is sent.  The time taken depends on the 
         * event in the collaborative queue and the network latency to the server.
         */
        static void sendCollaborativeMessageSync(std::string plugin, int type,
                char * data, int size, bool sendLocal = false);

    protected:

};

/**
 * @}
 */

}

#endif
