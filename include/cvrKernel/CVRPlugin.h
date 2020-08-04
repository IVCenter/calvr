/**
 * @file CVRPlugin.h
 */

#ifndef CALVR_CVRPLUGIN
#define CALVR_CVRPLUGIN

#include <cvrKernel/InteractionEvent.h>

#include <osg/Matrix>

namespace cvr
{

/**
 * @addtogroup kernel
 * @{
 */

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
         * @brief Interface function for CalVR events
         * @param event InteractionEvent to process
         * @return if true, the event is consumed and does not continue to 
         *  be processed
         *
         * Use the InteractionEvent's as functions to determine the event type
         * and if you want to use it.  If you want to keep others from using 
         * this event after you, return true.
         */
        virtual bool processEvent(InteractionEvent * event)
        {
            return false;
        }

        /**
         * @brief Interface function that receives external messages
         *
         * @param type what this message is, up to the plugin
         * @param data pointer to some block of data consistent with the type
         * @param collaborative is this a message sent through a collaborative session
         *
         * If the message is collaborative, any data will be deleted after the call.  To stop this and take
         * charge of the data, set the data pointer to NULL.
         */
        virtual void message(int type, char * & data,
                bool collaborative = false)
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

/**
 * @}
 */

}

#ifdef WIN32
#define CVRPLUGIN_EXPORT   __declspec(dllexport)
#else
#define CVRPLUGIN_EXPORT
#endif

/**
 * @addtogroup kernel
 * @{
 */

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
 CVRPLUGIN_EXPORT cvr::CVRPlugin * createPlugin() \
 { \
  return new plugin(); \
 } \
}

/**
 * @}
 */

#endif
