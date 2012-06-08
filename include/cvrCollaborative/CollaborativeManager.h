/**
 * @file CollaborativeManager.h
 */
#ifndef CVR_COLLABORATIVE_MANAGER_H
#define CVR_COLLABORATIVE_MANAGER_H

#include <cvrCollaborative/Export.h>

#include <string>
#include <map>
#include <vector>
#include <queue>

#include <osg/MatrixTransform>

namespace cvr
{

/*struct ClientUpdate
 {
 float headPos[3];
 float headRot[4];
 float handPos[3];
 float handRot[4];
 float objScale;
 float objTrans[16];
 int numMes;
 };*/

/**
 * @addtogroup collab cvrCollaborative
 * @{
 */

/**
 * @brief Information passed for a client update
 */
struct ClientUpdate
{
        float objScale; ///< object space scale
        float objTrans[16]; ///< object space transform matrix
        int numMes; ///< number of collaborative messages to follow
};

/**
 * @brief Position/Rotation of a tracked body in a collaborative session
 */
struct BodyUpdate
{
        float pos[3]; ///< position
        float rot[4]; ///< rotation (quat)
};

/**
 * @brief Mode of the collaborative session
 *
 * LOCKED - all object space transforms synced to the session master
 * UNLOCKED - everyone controls their own transforms and other clients hands/heads are visible
 */
enum CollabMode
{
    LOCKED,
    UNLOCKED
};

/**
 * @brief Type for a collaborative message
 */
enum CollaborativeMessageType
{
    ADD_CLIENT = 0,
    REMOVE_CLIENT,
    SET_MASTER_ID,
    SET_COLLAB_MODE,
    PLUGIN_MESSAGE
};

/**
 * @brief Update sent from the collaborative server
 */
struct ServerUpdate
{
        int numUsers;
        CollabMode mode;
        int masterID;
        int numMes;
};

// TODO: add version numbers for server/client interface
/**
 * @brief Session initialization info sent by the server
 */
struct ServerInitInfo
{
        int numUsers; ///< number of clients in the session
};

/**
 * @brief Header for all messages sent through the collaborative session
 */
struct CollaborativeMessageHeader
{
        int type; ///< CollaborativeMessageType
        int pluginMessageType; ///< message type specified by a sending plugin
        char target[256]; ///< name of target for this message
        int size; ///< size of any message data
        bool deleteData; ///< should the data for this message be deleted after sending
};

/**
 * @brief Initialization information for a collaborative client
 */
struct ClientInitInfo
{
        int id; ///< server fills this in
        char name[256]; ///< client name
        int numHeads; ///< number of tracked heads
        int numHands; ///< number of tracked hands
};

class CollaborativeThread;
class CVRSocket;

/**
 * @brief Manages a collaborative session and handles collaborative message passing
 */
class CVRCOLLAB_EXPORT CollaborativeManager
{
    public:
        ~CollaborativeManager();

        /**
         * @brief Returns static self pointer
         */
        static CollaborativeManager * instance();

        /**
         * @brief Called once at class creation
         */
        bool init();

        /**
         * @brief Returns if connected to a collaborative session
         */
        bool isConnected();

        /**
         * @brief Connect to a collaborative session
         * @param host server host address/name
         * @param port server port
         */
        bool connect(std::string host, int port);

        /**
         * @brief Disconnect from a collaborative session
         */
        void disconnect();

        /**
         * @brief Per frame update call
         */
        void update();

        /**
         * @brief Returns this system's id number in the collaborative session
         */
        int getID()
        {
            return _id;
        }

        /**
         * @brief Returns this system's collaborative name
         */
        const std::string & getName()
        {
            return _myName;
        }

        /**
         * @brief Get the current mode of the collaborative session
         */
        CollabMode getMode()
        {
            return _mode;
        }

        /**
         * @brief Set the mode of the collaborative session
         */
        void setMode(CollabMode mode);

        /**
         * @brief Get the id number of the master client in the collaborative session
         */
        int getMasterID()
        {
            return _masterID;
        }

        /**
         * @brief Set the id for the master client in the collaborative session
         */
        void setMasterID(int id);

        /**
         * @brief Get the map of all clients in the collaborative session, indexed by id
         */
        std::map<int,ClientInitInfo> & getClientInitMap()
        {
            return _clientInitMap;
        }

        /**
         * @brief Get the number of tracked heads for a client id
         */
        int getClientNumHeads(int id);

        /**
         * @brief Get the number of tracked heads for a client id
         */
        int getClientNumHands(int id);

        /**
         * @brief Get the head transform of a client
         * @param id client id
         * @param head tracked head number
         */
        const osg::Matrix & getClientHeadMat(int id, int head);

        /**
         * @brief Get the hand transform of a client
         * @param id client id
         * @param hand tracked hand number
         */
        const osg::Matrix & getClientHandMat(int id, int hand);

        /**
         * @brief Sends a message to a plugin in a collaborative session (asynchronous)
         * @param target plugin to send message to
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
        void sendCollaborativeMessageAsync(std::string target, int type,
                char * data, int size, bool sendLocal = false);

        /**
         * @brief Sends a message to a plugin in a collaborative session (synchronous)
         * @param target plugin to send message to
         * @param type value the plugin gets as the message type
         * @param data data to send for this message
         * @param size size of data to send
         * @param sendLocal if true, this message is also sent to the local instance of this plugin using the standard
         *        message interface
         *
         * This is a synchronous call.  The function does not return until the message is sent.  The time taken depends on the 
         * event in the collaborative queue and the network latency to the server.
         */
        void sendCollaborativeMessageSync(std::string target, int type,
                char * data, int size, bool sendLocal = false);

    protected:
        CollaborativeManager();

        /**
         * @brief Update the collaborative tranforms
         */
        void updateCollabNodes();

        /**
         * @brief Make geometry for a hand
         */
        osg::Node * makeHand(int num);

        /**
         * @brief Make geometry for a head
         */
        osg::Node * makeHead(int num);

        /**
         * @brief Start an update to the collaborative session
         */
        void startUpdate();

        /**
         * @brief Handle a message received through the collaborative session
         */
        void processMessage(CollaborativeMessageHeader & cmh, char * data);

        static CollaborativeManager * _myPtr; ///< static self pointer

        std::string _myName; ///< name registered with collaborative session

        cvr::CVRSocket * _socket; ///< collaborative session socket

        cvr::CollaborativeThread * _thread; ///< thread that handles socket communication

        std::map<int,ClientUpdate> _clientMap; ///< map of all client object space transforms
        std::map<int,ClientInitInfo> _clientInitMap; ///< map of all general client information

        int _id; ///< my collaborative id
        bool _connected; ///< if connected to a collaborative session
        int _masterID; ///< id of the current master client in the session
        CollabMode _mode; ///< current collaborative mode

        std::map<int,std::vector<BodyUpdate> > _handBodyMap; ///< map of all the client tracked hand body information
        std::map<int,std::vector<BodyUpdate> > _headBodyMap; ///< map of all the client tracked head body information

        osg::ref_ptr<osg::MatrixTransform> _collabRoot; ///< root of all collaborative geometry
        std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > > _collabHands; ///< transforms for each hand geometry in the session
        std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > > _collabHeads; ///< transforms for each head geometry in the session

        std::queue<std::pair<CollaborativeMessageHeader,char*> > _messageQueue; ///< queue of collaborative messages to send
};

/**
 * @}
 */

}

#endif
