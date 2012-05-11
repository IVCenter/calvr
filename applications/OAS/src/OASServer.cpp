#include "OASServer.h"

// static
oas::Server& oas::Server::getInstance()
{
    static oas::Server instance;

    return instance;
}

// private
void oas::Server::_readConfigFile(int argc, char **argv)
{
    const std::string defaultConfigFileLocation = "oas_config.xml";
    std::string configFile;
    bool fileExists = false;

    oas::FileHandler fileHandler;

    // If there are command line arguments
    if (argc > 1)
    {
        // Check the first argument as the config file location
        if (fileHandler.doesFileExist(argv[1]))
        {
            // File exists, so use it
            fileExists = true;
            configFile = argv[1];
        }
        else
        {
            oas::Logger::logf(  "The file at '%s' does not exist. Checking for a default configuration file at: ",
                                "'%s'\n", argv[1], defaultConfigFileLocation.c_str());
        }
    }
    else
    {
        oas::Logger::logf(  "Usage: \"%s [config file]\"\n", argv[0]);
        oas::Logger::logf(  "No config file parameter given. Checking for a default configuration file at: "
                            "'%s'\n", defaultConfigFileLocation.c_str());
    }

    // If no valid config file has been found
    if (!fileExists)
    {
        // Try the default config file location
        if (fileHandler.doesFileExist(defaultConfigFileLocation))
        {
            configFile = defaultConfigFileLocation;
            fileExists = true;
        }
        // Else, we have failed to load any configuration
        else
        {
            // @TODO: Use default settings, and generate default config file
            // For now, we say this is a warning, and load the hardcoded defaults.
            oas::Logger::warnf("Unable to load configuration file.");

            // Use defaults
            this->_serverInfo = new ServerInfo("/home/calvr/CalVR/applications/OAS/cache/", 31231);
           
            return;
        }
    }

    // At this point, we have guaranteed that a config file exists.
    
    // Load the XML file
    oas::FileHandler fh;
    if (!fh.loadXML(configFile, "OAS"))
    {
        this->_fatalError("Unable to load a valid configuration file.");
    }

    std::string cacheDirectory; 
    std::string port;

    // The config file MUST contain a cache directory and the port number the server will listen on
    // Find these two items in the XML file
    if (fh.findXML("cache_directory", NULL, NULL, cacheDirectory) &&
        fh.findXML("port", NULL, NULL, port))
    {
        this->_serverInfo = new ServerInfo(cacheDirectory, atol(port.c_str()));
    }
    else
    {
        if (!cacheDirectory.size())
            this->_fatalError("Could not retrieve cache directory information from configuration file.");
        else if (!port.size())
            this->_fatalError("Could not retrieve port number information from configuration file.");
    }

    // Optional sections of the config file
    std::string audioDevice;
    std::string gui;

    if (fh.findXML("audio_device", NULL, NULL, audioDevice) && audioDevice.size())
        this->_serverInfo->setAudioDeviceString(audioDevice);

    // GUI is enabled by default. We disable it only if explicitly specified
    this->_serverInfo->setGUI(true);

    if (fh.findXML("gui", NULL, NULL, gui) && gui.size())
        if (!gui.compare("off") 
            || !gui.compare("false")
            || !gui.compare("no")
            || !gui.compare("disable")
            || !gui.compare("disabled"))
        {
            oas::Logger::logf("The GUI has been disabled, as requested by the configuration file.");
            this->_serverInfo->setGUI(false);
        }
}

// private
void oas::Server::_processMessage(const Message &message)
{
    // If error, don't process message contents
    if (oas::Message::MERROR_NONE != message.getError())
    {
        return;
    }
    
    int newSource;
    unsigned int delay = 5;

    switch(message.getMessageType())
    {
        case oas::Message::MT_GHDL_FN:
            // Generate new audio source based on filename
            // 1) If the file is already loaded into a buffer, the buffer is reused in the new source
            // 2) Else look at filesystem to see if file exists
            //      2a) If exists, load audio file into a new buffer, then create new source
            //      2b) Else file does not exist, send  "-1" response
//            oas::Logger::logf("GHDL %s", message.getFilename());
            newSource = oas::AudioHandler::createSource(message.getFilename());
            if (-1 == newSource)
            {
                oas::Logger::logf("Server was unable to generate new audio source for file \"%s\".",
                                  message.getFilename().c_str());
                oas::SocketHandler::addOutgoingResponse(-1);
            }
            else
            {
                oas::Logger::logf("New sound source created for \"%s\". (Sound ID = %d)",
                                  message.getFilename().c_str(),
                                  newSource);
                oas::SocketHandler::addOutgoingResponse(newSource);
            }
            break;
        case oas::Message::MT_RHDL_HL:
            // Look through sources to see if handle exists. If exists, delete source. Else, do nothing
            oas::AudioHandler::deleteSource(message.getHandle());
            break;
        case oas::Message::MT_PTFI_FN_1I:
            // Shouldn't need to do anything!
            break;
        case oas::Message::MT_PLAY_HL:
            oas::Logger::logf("Playing sound #%d", message.getHandle());
            oas::AudioHandler::playSource(message.getHandle());
            break;
        case oas::Message::MT_STOP_HL:
            oas::Logger::logf("Stopping sound #%d", message.getHandle());
            oas::AudioHandler::stopSource(message.getHandle());
            break;
        case oas::Message::MT_SSPO_HL_3F:
            oas::AudioHandler::setSourcePosition( message.getHandle(), 
                                                  message.getFloatParam(0), 
                                                  message.getFloatParam(1),
                                                  message.getFloatParam(2));
            break;
        case oas::Message::MT_SSVO_HL_1F:
            oas::AudioHandler::setSourceGain( message.getHandle(),
                                              message.getFloatParam(0));
            break;
        case oas::Message::MT_SSLP_HL_1I:
            oas::AudioHandler::setSourceLoop( message.getHandle(),
                                              message.getIntegerParam());
            break;
        case oas::Message::MT_SSVE_HL_1F:
            oas::AudioHandler::setSourceSpeed( message.getHandle(),
                                               message.getFloatParam(0));
            break;
        case oas::Message::MT_SSVE_HL_3F:
            oas::AudioHandler::setSourceVelocity( message.getHandle(),
                                                  message.getFloatParam(0),
                                                  message.getFloatParam(1),
                                                  message.getFloatParam(2));
            break;
        case oas::Message::MT_SSDI_HL_1F:
            oas::AudioHandler::setSourceDirection( message.getHandle(),
                                                   message.getFloatParam(0));
            break;
        case oas::Message::MT_SSDI_HL_3F:
            oas::AudioHandler::setSourceDirection( message.getHandle(),
                                                   message.getFloatParam(0),
                                                   message.getFloatParam(1),
                                                   message.getFloatParam(2));
            break;
        case oas::Message::MT_SSDV_HL_1F_1F:
            oas::AudioHandler::setSourceDirection( message.getHandle(),
                                                   message.getFloatParam(0));
            oas::AudioHandler::setSourceGain( message.getHandle(),
                                              message.getFloatParam(1));
            break;
        case oas::Message::MT_SSDR_HL_1F:
            oas::Logger::warnf("SSDR is deprecated! Ignoring instruction.");
            break;
        case oas::Message::MT_SSRV_HL_1F_1F:
            oas::Logger::warnf("SSRV is deprecated! Ignoring instruction.");
            break;
        case Message::MT_SSRV_HL_3F_1F:
            oas::Logger::warnf("SSRV is deprecated! Ignoring instruction.");
            break;
        case Message::MT_TEST:
            break;
        case Message::MT_SYNC:
            // Send a simple "SYNC" response
            oas::SocketHandler::addOutgoingResponse("SYNC");
            break;
        case Message::MT_QUIT:
            // Will need to release all audio resources and then re-initialize them
            oas::AudioHandler::release();
            oas::ServerWindow::reset();
            // If for some reason initialization fails, try again
            while (!oas::AudioHandler::initialize(this->_serverInfo->getAudioDeviceString()))
            {
                oas::Logger::errorf("Failed to reset audio resources. Trying again in %d seconds.", delay);
                sleep(delay);
                delay += 5;
            }
            break;
        default:
            return;
    }
    gettimeofday(&((Message *) (&message))->processed, NULL);
}

// public
void oas::Server::initialize(int argc, char **argv)
{
    this->_readConfigFile(argc, argv);

    if (this->_serverInfo->useGUI() 
        && !oas::ServerWindow::initialize(argc, argv, &oas::Server::_atExit))
    {
        _fatalError("Could not initialize the windowed user interface!");
    }
    
    if (!oas::FileHandler::initialize(this->_serverInfo->getCacheDirectory()))
    {
        _fatalError("Could not initialize the File Handler!");
    }

    if (!oas::AudioHandler::initialize(this->_serverInfo->getAudioDeviceString()))
    {
        _fatalError("Could not initialize the Audio Handler!");
    }

    if (!oas::SocketHandler::initialize(this->_serverInfo->getPort()))
    {
        _fatalError("Could not initialize the Socket Handler!");
    }

    // Thread attribute variable
    pthread_attr_t threadAttr;

    // Initialize thread attribute
    pthread_attr_init(&threadAttr);

    // Set joinable thread attribute
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    // Spawn thread to run the core server loop
    int threadError = pthread_create(&this->_serverThread, &threadAttr, &this->_serverLoop, NULL);
    
    // Destroy thread attribute
    pthread_attr_destroy(&threadAttr);

    if (threadError)
    {
        _fatalError("Could not create server thread!");
    }

    // Fl::run() puts all of the FLTK window rendering on this current thread (main thread)
    Fl::run();
}

// private, static
void* oas::Server::_serverLoop(void *parameter)
{
    std::queue<Message*> messages;
    std::queue<const AudioUnit*> sources;
    const AudioSource *source;

    const long int k_timeoutMicroseconds = 100000; // Timeout in 0.1 seconds

    struct timespec timeOut;

    while (1)
    {
        _computeTimeout(timeOut, k_timeoutMicroseconds);

        // If there are no messages, populateQueueWithIncomingMessages() will block until timeout
        oas::SocketHandler::populateQueueWithIncomingMessages(messages, timeOut);

        // If messages is still empty, then we timed out. Check the audio state for updates before redoing the loop
        if (messages.empty())
        {
            oas::AudioHandler::populateQueueWithUpdatedSources(sources);

            if (!sources.empty())
                 oas::ServerWindow::audioUnitsWereModified(sources);

            continue;
        }

        while (!messages.empty())
        {
            Message *nextMessage = messages.front();
            oas::Server::getInstance()._processMessage(*nextMessage);
            oas::Logger::logf("Server processed message \"%s\"", nextMessage->getOriginalString().c_str());
            delete nextMessage;
            messages.pop();

            source = oas::AudioHandler::getRecentlyModifiedSource();
            if (NULL != source)
            {
                // Call ServerWindow method that will queue up the source
                oas::ServerWindow::audioUnitWasModified(source);
            }
        }
    }

    return NULL;
}

// private, static
void oas::Server::_computeTimeout(struct timespec &timeout, unsigned long int k_timeoutInMicroseconds)
{
    struct timeval currTime;

    gettimeofday(&currTime, NULL);

    timeout.tv_sec = currTime.tv_sec + (k_timeoutInMicroseconds / 1000000);
    timeout.tv_nsec = (currTime.tv_usec + ((k_timeoutInMicroseconds - (k_timeoutInMicroseconds / 1000000)))) * 1000;

}

double oas::Server::_computeElapsedTime(struct timeval start, struct timeval end)
{
    return ((end.tv_sec + ((double) end.tv_usec / 1000000.0)) - (start.tv_sec + ((double) start.tv_usec / 1000000.0))); 
}

// private, static
void oas::Server::_fatalError(const char *errorMessage)
{
    std::cerr << "\n\n" 
              << "OAS: Fatal Error occured!\n"
              << "     Error: " << errorMessage << "\n"
              << "Exiting OAS...\n\n";
    exit(1);
}

void oas::Server::_atExit()
{
    oas::SocketHandler::terminate();
    oas::AudioHandler::release();
}

// Main
int main(int argc, char **argv)
{
    // Initialize all of the components of the server
    oas::Server::getInstance().initialize(argc, argv);

    return 0;
}

