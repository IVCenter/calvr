/**
 * @file Gesture.h
 */
#pragma once

#include <string>

namespace cvr
{

class GestureDetector;

/**
 *  @brief  Represents a script running in a GestureDetector allowing for it
 *           to be reset, suspended, or removed from the detector.
 *  
 *  Each gesture to match is loaded from a Lua script and run as a coroutine
 *  in the context of a specific GestureDetector instance.
 */
class GestureScript
{
        friend class GestureDetector;
    public:

        /**
         *  Resets the gesture script; equivalent to canceling the script and
         *  reloading it into the gesture detector.
         */
        void reset();

        /**
         *  Suspends the execution of the script until resume() is called.
         */
        void suspend();

        /**
         *  If suspend() has been called, resumes the script; otherwise,
         *  has no effect.
         */
        void resume();

        /**
         *  Removes the script from execution, preventing it from being run
         *  again (unless it is reloaded).
         */
        void cancel();

    private:
        /**
         * Direct construction of GestureScript instances is not allowed because
         * they are not portable between GestureDetectors; to create a
         * GestureScript, load a script into a specific detector.
         */
        GestureScript();

        GestureDetector* detector;
        std::string name;
        std::string code;
};

/**
 *  @brief  Allows for scripting of custom Kinect gestures using Lua.
 */
class GestureDetector
{
        friend class GestureScript;

    public:
        /**
         *  @brief Create a gesture detector using skeleton input from a librk
         *  server.
         *
         *  @param host The network name or ip of the computer the server is on.
         *  @param port The port on which to stream skeleton data. (default: 9002)
         */
        GestureDetector(std::string host = "localhost", unsigned short port =
                9002);

        ~GestureDetector();

        /**
         *  @brief Loads a lua script from a C++ string.
         */
        GestureScript load_string(std::string script_name, std::string code);

        /**
         *  @brief Loads a lua script from a file.
         */
        GestureScript load_file(std::string filename);

        /// allow access to symbols in plugin_name's shared object
        void look_in_plugin(std::string plugin_name);

        /**
         *  @brief Advances all active gesture detection scripts to the next step.
         */
        void step();

    private:
        void* internal_state;
};

}
