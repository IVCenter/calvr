/**
 * @file ComController.h
 */

#ifndef CALVR_COM_CONTROLLER_H
#define CALVR_COM_CONTROLLER_H

#include <kernel/Export.h>
#include <kernel/CalVR.h>
#include <util/CVRSocket.h>
#include <util/MultiListenSocket.h>

#include <osg/ArgumentParser>

#include <string>
#include <vector>
#include <map>

namespace cvr
{

/**
 * @brief Handles cluster communication
 *
 * Allows for multinode communication and synchronization
 */
class CVRKERNEL_EXPORT ComController
{
    friend class CalVR;
    public:

        /**
         * @brief Sets up node sockets based on command line arguments
         * @param ap The arguments from the command line
         */
        bool init(osg::ArgumentParser * ap);

        /**
         * @brief Send a block of data to the slave nodes
         * @param data Pointer to data to send
         * @param size Ammount of data to send
         *
         * Only valid if called by master node
         */
        bool sendSlaves(void * data, int size);

        /**
         * @brief Read a block of data from the master node
         * @param data Pointer to buffer to receive data
         * @param size Ammount of data to read
         *
         * Only valid if called by slave node
         */
        bool readMaster(void * data, int size);

        /**
         * @brief Read data from all slave nodes
         * @param data Buffer to receive all data(numSlaves * size), if NULL
         *             the data is discarded
         * @param size Ammount of data to read from each slave node
         *
         * Only valid when called by master node
         */
        bool readSlaves(void * data, int size);

        /**
         * @brief Send data to the master node
         * @param data Buffer containing data to send
         * @param size Size of data to send
         *
         * Only valid when called by slave node
         */
        bool sendMaster(void * data, int size);

        /**
         * @brief Sync the cluster to this call
         *
         * Function does not complete until all nodes in the clust call it
         */
        bool sync();

        /**
         * @brief Returns true if this node is the master node
         */
        bool isMaster();

        /**
         * @brief Get the number of slave nodes in the cluster
         */
        int getNumSlaves();

        bool getIsSyncError() 
        {
            return _CCError;
        }

        /**
         * @brief Returns a pointer to the instance of this class
         */
        static ComController * instance();

    protected:
        ComController();
        virtual ~ComController();


        bool setupConnections(std::string & fileArgs);
        bool connectMaster();

        /**
         * Message passed during multinode startup
         */
        struct InitMsg
        {
                bool ok; ///< were there no errors
        };

        bool _isMaster; ///< am I the master node
        std::string _masterInterface; ///< the name of the master nodes network interface
        int _slaveNum; ///< my number, if i am a slave node
        int _port; ///< port to connect to the master node
        int _numSlaves; ///< number of slave nodes in the cluster
        int _maxSocketFD;
        fd_set _sockets;

        cvr::CVRSocket * _masterSocket; ///< socket to talk to master with
        std::map<int,cvr::CVRSocket *> _slaveSockets; ///< list of slave node sockets
        cvr::MultiListenSocket * _listenSocket;
        std::map<int,std::string> _startupMap;

        static ComController * _myPtr; ///< static self pointer

        bool _CCError;
};

}

#endif
