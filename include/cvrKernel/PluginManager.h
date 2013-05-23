/**
 * @file PluginManager.h
 */

#ifndef CALVR_PLUGIN_MANAGER_H
#define CALVR_PLUGIN_MANAGER_H

#include <cvrKernel/Export.h>
#include <cvrKernel/CVRPlugin.h>

#include <string>
#include <vector>
//#include <queue>
#include <map>

namespace cvr
{

/*template< typename FirstType, typename SecondType >
 struct PrioritySort
 {
 bool operator() (std::pair<FirstType,SecondType> const& first, std::pair<FirstType,SecondType> const& second)
 {
 return first.first > second.first;
 }
 };*/

class InteractionEvent;

/**
 * @addtogroup kernel
 * @{
 */

/**
 * @brief Loads and manager CalVR plugins
 */
class CVRKERNEL_EXPORT PluginManager
{
    public:
        ~PluginManager();

        /**
         * @brief Get static pointer to class instance
         */
        static PluginManager * instance();

        /**
         * @brief Load plugins specified in config file
         */
        bool init();

        /**
         * @brief Do preFrame callback on all loaded plugins
         */
        void preFrame();

        /**
         * @brief Do postFrame callback on all loaded plugins
         */
        void postFrame();

        /**
         * @brief Forward interaction event to loaded plugins
         */
        bool processEvent(InteractionEvent * event);

        /**
         * @brief Sends a message to a plugin with the given name through the message() callback in the plugin interface
         * @param plugin name of plugin to send message to
         * @param type value the plugin gets as the message type
         * @param data pointer to any message data
         *
         * If the plugin is not on, nothing happens.  This is a local call and no data copies or movement is involved.
         */
        void sendMessageByName(std::string plugin, int type, char * data);

        /**
         * @brief Get the pointer to a plugin with the given name
         * @return returns NULL if plugin is off
         */
        CVRPlugin * getPlugin(std::string plugin);

        std::string getPluginName(CVRPlugin * plugin);

        /**
         * @brief Returns if the plugin with the given name is on
         */
        bool getPluginLoaded(std::string plugin);

        /**
         * @brief Get a list of the names of all loaded plugins
         */
        std::vector<std::string> getLoadedPluginList();

    protected:
        PluginManager();

        static PluginManager * _myPtr; ///< static self pointer

        /**
         * @brief Open a plugin from a shared library by name
         */
        bool loadPlugin(std::string plugin);

        /**
         * @brief Structure with info for a loaded plugin
         */
        struct PluginInfo
        {
                int priority;       ///< plugin priority
                CVRPlugin * ptr;    ///< pointer to instance of loaded plugin
                std::string name;   ///< name of plugin
        };

        /**
         * @brief Contains function used to sort vector of plugins based on priority
         */
        struct PrioritySort
        {
                bool operator()(PluginInfo* const & first,
                        PluginInfo* const & second)
                {
                    return first->priority > second->priority;
                }
        };

        std::vector<std::string> _pluginLibDirs; ///< directories containing plugin shared libraries
        //std::priority_queue<std::pair<int,CVRPlugin*>,std::vector<std::pair<int,CVRPlugin*> >,PrioritySort<int,CVRPlugin*> > _pluginList;
        std::vector<PluginInfo *> _loadedPluginList; ///< list of loaded plugins
        std::map<std::string,bool> _pluginMap; ///< map containing all plugin names in the config file and if they are on

};

/**
 * @}
 */

}

#endif
