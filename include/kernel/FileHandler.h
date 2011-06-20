/**
 * @file FileHandler.h
 */

#ifndef CALVR_FILE_HANDLER_H
#define CALVR_FILE_HANDLER_H

#include <kernel/Export.h>
#include <string>
#include <map>
#include <vector>

namespace cvr
{

class CVRKERNEL_EXPORT FileLoadCallback;

// TODO: add unload function call

/**
 * @brief Handles the loading of data file types
 */
class FileHandler
{
    public:
        ~FileHandler();

        /**
         * @brief get static pointer to class instance
         */
        static FileHandler * instance();

        /**
         * @brief try to load a file
         * @param file file to load
         * @return true if file is loaded
         *
         * Will determine if the extension is registered with a callback and attempt
         * to load with it.  If callback not present, or fails, osg readNodeFile is tried
         * and the result is attached to the object root.
         */
        bool loadFile(std::string file);

        /**
         * @brief register a callback for when a file with a certain extension is loaded
         * @param ext extension to register, not case sensitive
         * @param fl class object to receive the load call
         *
         * Note: If the extension is already registered, it is not replaced.
         */
        void registerExt(std::string ext, FileLoadCallback * fl);

        /**
         * @brief unregister a callback for when a file with a certain extension is loaded
         * @param ext extension to unregister, not case sensitive
         * @param fl callback that is registered to this extension
         *
         * Note: If this callback does not have the extension registered to it, the entry is
         * not removed.
         */
        void unregisterExt(std::string ext, FileLoadCallback * fl);

    protected:
        FileHandler();

        static FileHandler * _myPtr; ///< static self pointer

        std::map<std::string,FileLoadCallback *> _extMap; ///< map of file extension to callback
};

/**
 * @brief Interface class to receive a callback from the FileHandler when a file with a certain 
 * extension is being loaded
 */
class FileLoadCallback
{
    public:
        /**
         * @brief Constructor
         * @param exts comma separated list extensions to register to this callback class
         */
        FileLoadCallback(std::string exts);

        virtual ~FileLoadCallback();

        /**
         * @brief function called when a file with a registered extension is loaded
         */
        virtual bool loadFile(std::string file) = 0;
    private:
        std::vector<std::string> _exts; ///< list of extensions registed to this callback
};

}

#endif
