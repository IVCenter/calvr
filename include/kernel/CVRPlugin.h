/**
 * @file CVRPlugin.h
 */

#ifndef CALVR_CVRPLUGIN
#define CALVR_CVRPLUGIN

#include <osg/Matrix>

namespace cvr
{

/**
 * @brief Interface class for all CalVR plugins
 */
class CVRPlugin
{
    public:
        CVRPlugin()
        {
        }

        virtual ~CVRPlugin()
        {
        }

        /**
         * @brief Called right after the plugin is created
         * @return true on success, false otherwise
         *
         * A return value of false will unload the plugin
         */
        virtual bool init()
        {
            return true;
        }

        /**
         * @brief Called right before a frame is rendered
         */
        virtual void preFrame()
        {
        }

        /**
         * @brief Called right after a frame is rendered
         */
        virtual void postFrame()
        {
        }

        /**
         * @brief Called when a key is pressed or released
         * @param keyDown true if keydown event, false if keyup event
         * @param key id of key, ascii in most cases
         * @param mod modifier on keypress, ie ctrl,alt,etc.
         * @return a value of true will consume the event from the interaction pipeline
         */
        virtual bool keyEvent(bool keyDown, int key, int mod)
        {
            return false;
        }

        /**
         * @brief Called when there is a tracking system event
         * @param type event type from enum ::InteractionType
         * @param button the controller button this event relates to
         * @param hand the hand number for this button
         * @param mat the orientation matrix for the hand when this event occured
         * @return a value of true will consume the event from the interaction pipeline
         *
         * Note: button number is not unique, hand/button number pair is unique
         */
        virtual bool buttonEvent(int type, int button, int hand,
                                 const osg::Matrix & mat)
        {
            return false;
        }

        /**
         * @brief Called when there is a mouse event
         * @param type event type from enum ::InteractionType
         * @param button mouse button this event relates to
         * @param x viewport x coord for event
         * @param y viewport y coord for event
         * @param mat 3 space matrix transform for mouse, centered on viewer and going through mouse/viewport point
         */
        virtual bool mouseButtonEvent(int type, int button, int x, int y,
                                      const osg::Matrix & mat)
        {
            return false;
        }

        /**
         * @brief Interface function that receives external messages
         *
         * @param type what this message is, up to the plugin
         * @param data pointer to some block of data consistent with the type
         */
        virtual void message(int type, char * data)
        {
        }

        /**
         * @brief Return this plugin's priority
         *
         * The priority determines the order the list of active plugins receive all interface callbacks.  A plugin
         * with a higher number will get calls before one with a lower number.  This number should not really be
         * changed unless there is a good reason.
         */
        virtual int getPriority()
        {
            return 50;
        }
};

}

/**
 * @brief Defines the interface function for the plugin's library
 * @param plugin name of the plugin class
 *
 * The defined function is used to create a plugin object out of
 * the plugin's shared library.
 */
#define CVRPLUGIN(plugin) \
extern "C" \
{ \
 cvr::CVRPlugin * createPlugin() \
 { \
  return new plugin(); \
 } \
}

#endif
