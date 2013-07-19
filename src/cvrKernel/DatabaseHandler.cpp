/*
 * DatabaseHandler.cpp
 *
 * Used to send and receive states between a collaboration server and the Statemanager.
 *
 *  Created on: Apr 29, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 16, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#include <cvrKernel/DatabaseHandler.h>
#include <cdbapp/State.h>

bool
DatabaseHandler::LoadStateChanges(std::list< std::string >& loadedStates)
{
    uint64_t time_back; // To validate load

    // Load new/changed states
    mDatabase.SendLoadMsg(mLastLoadTimestep, time_back, loadedStates);

    // time_back of 0 implies nothing loaded - sanity check to ensure time is continuous
    if (time_back > mLastLoadTimestep)
    {
        mLastLoadTimestep = time_back;
        return true;    // database returned latest status
    }

    return false;
}

void
DatabaseHandler::Hear( CvrState*  cvrState )
{
    mHeardStates[ cvrState->Uuid() ] = cvrState->Write();
}

void
DatabaseHandler::SendHeardStates(void)
{
    // skip if we have nothing to do
    if (mHeardStates.empty())
        return;

    // populate a list of string representations for database
    std::list< std::string > states;

    std::map< std::string, std::string >::iterator it;
    for (it = mHeardStates.begin(); it != mHeardStates.end(); ++it)
        states.push_back( it->second );

    // send string representations of states to the database
    mDatabase.SendSaveMsg( states );

    // clear to-be-sent map
    mHeardStates.clear();
}

