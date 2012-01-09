#include "OASServer.h"

// Statics
pthread_t       oas::Server::_serverThread;
std::string     oas::Server::_cacheDirectory;
unsigned short  oas::Server::_port;

// static, private
void oas::Server::_readConfigFile()
{
    oas::Server::_cacheDirectory = "/home/schowkwa/OAS";
    oas::Server::_port = 31231;
}

// static, private
void oas::Server::_processMessage(const Message &message)
{
    // If error, don't process message contents
    if (oas::Message::MERROR_NONE != message.getError())
    {
        return;
    }
    
    ALuint newSource;
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
            if (AL_NONE == newSource)
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
//            oas::Logger::logf("RHDL %d", message.getHandle());
            // Look through sources to see if handle exists. If exists, delete source. Else, do nothing
            oas::AudioHandler::deleteSource(message.getHandle());
            break;
        case oas::Message::MT_PTFI_FN_1I:
            // Shouldn't need to do anything!
//            oas::Logger::logf("PTFI");
            break;
        case oas::Message::MT_PLAY_HL:
//            oas::Logger::logf("PLAY %d", message.getHandle());
            oas::Logger::logf("Playing sound #%d", message.getHandle());
            oas::AudioHandler::playSource(message.getHandle());
            break;
        case oas::Message::MT_STOP_HL:
//            oas::Logger::logf("STOP %d", message.getHandle());
            oas::Logger::logf("Stopping sound #%d", message.getHandle());
            oas::AudioHandler::stopSource(message.getHandle());
            break;
        case oas::Message::MT_SSPO_HL_3F:
//            oas::Logger::logf("SSPO");
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
 //           oas::Logger::logf("SSLP");
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

            // If for some reason initialization fails, try again
            while (!oas::AudioHandler::initialize())
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

// public, static
void oas::Server::initialize(int argc, char **argv)
{
    oas::Server::_readConfigFile();

    if (!oas::ServerWindow::initialize(argc, argv))
    {
        _fatalError("Could not initialize the windowed user interface!");
    }
    
    if (!oas::FileHandler::initialize(oas::Server::_cacheDirectory))
    {
        _fatalError("Could not initialize the File Handler!");
    }

    if (!oas::AudioHandler::initialize())
    {
        _fatalError("Could not initialize the Audio Handler!");
    }

    if (!oas::SocketHandler::initialize(oas::Server::_port))
    {
        _fatalError("Could not initialize the Socket Handler!");
    }

    // Thread attribute variable
    pthread_attr_t threadAttr;

    // Initialize thread attribute
    pthread_attr_init(&threadAttr);

    // Set joinable thread attribute
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    // Spawn thread to run the loop that handles connections
    int threadError = pthread_create(&oas::Server::_serverThread, &threadAttr, &oas::Server::_serverLoop, NULL);
    
    // Destroy thread attribute
    pthread_attr_destroy(&threadAttr);

    if (threadError)
    {
        _fatalError("Could not create server thread!");
    }

    return;
}

double convert(struct timeval start, struct timeval end);

// private, static
void* oas::Server::_serverLoop(void *parameter)
{
    std::queue<Message*> messages;
    while (1)
    {
        // If there are no messages, getNextIncomingMessage() will block
        oas::SocketHandler::populateQueueWithIncomingMessages(messages);

        while (!messages.empty())
        {
            Message *nextMessage = messages.front();
            oas::Server::_processMessage(*nextMessage);
            //oas::Logger::logf("%.4lf, %.4lf, %.4lf", convert(nextMessage->start, nextMessage->added), convert(nextMessage->start, nextMessage->retrieved), convert(nextMessage->start, nextMessage->processed));
            oas::Logger::logf("Server processed message \"%s\"", nextMessage->getOriginalString().c_str());
            delete nextMessage;
            messages.pop();
        }
    }

    return NULL;
}

double convert(struct timeval start, struct timeval end)
{
    return ((end.tv_sec + ((double) end.tv_usec / 1000000.0)) - (start.tv_sec + ((double) start.tv_usec / 1000000.0))); 
}

// private, static
void oas::Server::_fatalError(const char *errorMessage)
{
    std::cerr << "\n\nOAS: Fatal Error occured!\n     Error: " << errorMessage;
    std::cerr << "\nExiting OAS...\n\n";
    exit(1);
}

// Main
int main(int argc, char **argv)
{
    // This method will initialize all of the components of the server
    oas::Server::initialize(argc, argv);

    return Fl::run();
}

