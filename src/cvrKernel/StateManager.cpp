/*
 * StateManager.cpp
 *
 * Used to send and receive states between the DatabaseHandler and CalVR.
 *
 *  Created on: Apr 29, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 15, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#include <iostream>
#include <assert.h>

#include <cvrKernel/ComController.h>

#include <cdbapp/State.h>
#include <cvrKernel/States/CvrState.h>
#include <cvrKernel/StateManager.h>

using namespace cvr;

StateManager::~StateManager()
{
    mStates.clear();  // will unref automatically
}

void
StateManager::AlertListeners(CvrState* cvrstate)
{
    assert(cvrstate != NULL);
    osg::ref_ptr< CvrState > safety = cvrstate; // So that Listeners do not delete accidently

    // alert all registered listeners -- store iterators prior to iterating to avoid corrupted iterating
    Shouter<CvrState>::Listeners::iterator begin = mListeners.begin(),
        end = mListeners.end(), current;

    for (current = begin; current != end; ++current)
        cvrstate->ShoutAt(*current);
}

void
StateManager::CollaborateState(std::string const& uuid, bool const collaborate)
{
    if (!cvr::ComController::instance()->isMaster())
        return;

    CvrState* state = StateFromUuid(uuid);

    if (!state)
    {
        std::cerr << "StateManager::CollaborateState(-) received an unregistered CvrState uuid. (ignoring)" << std::endl;
        return;
    }

    if (collaborate)
        state->AddListener(&mDatabaseHandler);
    else
        state->RemoveListener(&mDatabaseHandler);
}

void
StateManager::Connect(std::string const& connectionAddress)
{
    if (cvr::ComController::instance()->isMaster())
        mDatabaseHandler.Connect( connectionAddress );
}

void
StateManager::Disconnect(void)
{
    if (cvr::ComController::instance()->isMaster())
        mDatabaseHandler.Disconnect();
}

void
StateManager::Hear(CvrState* cvrstate)
{
    // At this point, expect StateManager to already know about the state
    CvrState* registered = StateFromUuid( cvrstate->Uuid() );

    if (!registered)
        return;

    if (registered != cvrstate) // Optimization via pointer comparison
        registered->Variables( cvrstate->Variables() );
}

void
StateManager::Register(Listener<CvrState>* listener)
{
    if (!listener)
    {
        std::cerr << "StateManager::Register(Listener*) passed NULL parameter. (ignoring)" << std::endl;
        return;
    }

    mListeners.insert(listener);
}

void
StateManager::Register(CvrState* cvrstate)
{
    if (!cvrstate)
    {
        std::cerr << "StateManager::Register(CvrState*) passed NULL parameter. (ignoring)" << std::endl;
        return;
    }

    // Validate that the CvrState is currently unregistered
    if (StateFromUuid( cvrstate->Uuid() ))
    {
        std::cerr << "StateManager::Register(CvrState*) received a non-new CvrState. (ignoring)" << std::endl;
        return;
    }

    mStates[cvrstate->Uuid()] = cvrstate;
    cvrstate->AddListener(this);
}

CvrState*
StateManager::StateFromUuid(std::string const& uuid) const
{
    CvrStateMap::const_iterator it = mStates.find( uuid );
    if (it != mStates.end())
        return it->second;

    return NULL;
}

void
StateManager::Unregister(Listener<CvrState>* listener)
{
    if (!listener)
    {
        std::cerr << "StateManager::Unregister(Listener*) received a NULL listener*. (ignoring)" << std::endl;
        return;
    }

    mListeners.erase(listener);
}


void
StateManager::Unregister(CvrState* cvrstate)
{
    if (!cvrstate)
    {
        std::cerr << "StateManager::Unregister(CvrState*) received a NULL CvrState*. (ignoring)" << std::endl;
        return;
    }

    std::string uuid = cvrstate->Uuid();

    osg::ref_ptr<CvrState> registered = StateFromUuid(uuid);
    if (!registered)
    {
        std::cerr << "StateManager::Unregister(CvrState*) received an unregistered CvrState. (ignoring)" << std::endl;
        return;
    }

    mStates.erase(uuid);
}

void
StateManager::UpdateLocalStates(void)
{
    std::list< std::string > returned_states;

    LoadStateChanges(returned_states);

    std::list< std::string >::iterator it;
    for (it = returned_states.begin(); it != returned_states.end(); ++it)
    {
        cdb::State state = cdb::State::Read( *it );
        osg::ref_ptr<CvrState> cvrstate = CvrState::AdaptToDerivedCvrState( state );

        if (cvrstate)
        {
            cvrstate->ShoutAt(this);
            AlertListeners(cvrstate);
        }
    }
}

void
StateManager::UpdateServerStates(void)
{
    if (cvr::ComController::instance()->isMaster())
        mDatabaseHandler.SendHeardStates();
}

/**** BEGIN PROTECTED FUNCTIONS ****/


void
StateManager::LoadStateChanges(std::list< std::string >& returnedStates)
{
    std::string json_array_of_states = "";
    int file_size = 0;
    char *cArray;

    if (cvr::ComController::instance()->isMaster())
    {
        mDatabaseHandler.LoadStateChanges( returnedStates );
        cdb::State::ConvertStringsToJsonStateArray( returnedStates, json_array_of_states );
        file_size = json_array_of_states.length();

        cvr::ComController::instance()->sendSlaves(&file_size, sizeof(file_size));

        if (file_size > 0)
        {
            cvr::ComController::instance()->sendSlaves((void*)json_array_of_states.c_str(), sizeof(char)*file_size);
        }
    }
    else // Slave Node
    {
        cvr::ComController::instance()->readMaster(&file_size, sizeof(file_size));
        if (file_size > 0)
        {
            cArray = new char[file_size];
            cvr::ComController::instance()->readMaster(cArray, sizeof(char)*file_size);
            json_array_of_states = cArray;
            delete[] cArray;

            cdb::State::ConvertJsonStateArrayToStrings(json_array_of_states, returnedStates);
        }
    }
}


