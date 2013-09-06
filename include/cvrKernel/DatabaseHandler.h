/*
 * DatabaseHandler.h
 *
 * Used to send and receive states between a collaboration server and CalVR.
 *
 *  Created on: Apr 29, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 16, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef DATABASE_HANDLER_H_
#define DATABASE_HANDLER_H_

#include <string>
#include <map>
#include <list>
#include <stdint.h>

#include <cdbapp/ApplicationDatabaseInterface.h>
#include <cvrKernel/States/CvrState.h>

namespace cvr {

class DatabaseHandler : public Listener<CvrState>
{
public:

    void
    Connect(std::string const& connectionAddress);

    void
    Disconnect(void);

    bool
    LoadStateChanges( std::list< std::string >& loadedStates );

    void
    SendHeardStates(void);

protected:

    /* Hearing CvrStates (External concepts):
     * - Store CvrState as a JSON string to be passed to the the collaboration server (database).
     */
    void
    Hear(CvrState* cvrState);

    cdb::ApplicationDatabaseInterface     mDatabase;
    uint64_t                              mLastLoadTimestep;
    std::map< std::string, std::string >  mHeardStates;

};

inline void
DatabaseHandler::Connect(std::string const& connectionAddress)
{
    mDatabase.Connect(connectionAddress);
    mLastLoadTimestep = 0;
}

inline void
DatabaseHandler::Disconnect(void)
{
    mDatabase.Disconnect();
}

}

#endif /* DATABASE_HANDLER_H_ */
