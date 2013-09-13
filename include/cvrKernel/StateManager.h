/*
 * StateManager.h
 *
 * Rules and Concepts of State Management:
 * - Listener<CvrState> classes can be registered to the StateManager to hear alerted CvrStates.
 * - Any CvrState can be passed to AlertListeners(). This is to give Registered Listeners a chance to hear about new CvrStates.
 * - Any Listener that wants to hear updates about a heard/alerted CvrState should add themselves to the CvrState's local listener list [ cvrstate->AddListener(listener/this) ].
 * - If a listener hears a CvrState for the first time and wishes to see it stored, it should Register the CvrState with StateManager before the Hear() function returns.
 *  + Any Listener that Registers a CvrState is expected to Unregister it at some point, to allow deallocation.
 *  + The StateManager is in charge of keeping Registered states in memory.
 * - StateManager will never hold interest in whether or not a CvrState is "valid".
 * - To toggle a CvrState for collaboration use CollaborateState(string:uuid, bool:enable_collaboration)
 *  + The CvrState being Collaborated must be Registered.
 *  + Once marked for Collaboration, all Shouts thereafter will be heard by the DatabaseHandler, which will store it to be sent to the database.
 * - The StateManager should only be dealt with on the main thread.
 * - The StateManager owns CvrState memory once the CvrState is registered. Listeners that wish to store CvrStates should retain a UUID and grab the State from the StateManager as needed.
 *
 *  Created on: Apr 29, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 *              David Srour <dsrour@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 15, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef STATE_MANAGER_H_
#define STATE_MANAGER_H_

#include <string>
#include <map>

#include <osg/ref_ptr>

#include <cvrUtil/Listener.h>
#include <cvrUtil/Shouter.hpp>

#include <cvrKernel/CalVR.h>
#include <cvrKernel/DatabaseHandler.h>
#include <cvrKernel/States/CvrState.h>

namespace cvr {

class StateManager : public Listener<CvrState>
{
public:
    friend class CalVR;

    static StateManager*
    instance(void);

    void
    AlertListeners(CvrState* cvrstate);

    void
    CollaborateState(std::string const& uuid, bool const collaborate);

    void
    Connect(std::string const& connectionAddress);

    void
    Disconnect(void);

    void
    Register(Listener<CvrState>* listener);

    void
    Register(CvrState* cvrstate);

    CvrState*
    StateFromUuid(std::string const& uuid) const;

    void
    Unregister(Listener<CvrState>* listener);

    void
    Unregister(CvrState* cvrstate);

    void
    UpdateLocalStates(void);

    void
    UpdateServerStates(void);

protected:
    static StateManager* mInstance;

    StateManager();

    virtual
    ~StateManager();

    // NOTE: May want to split into multiple maps in the future for efficiency, as necc.
    typedef std::map< std::string, osg::ref_ptr<CvrState> > CvrStateMap;  // uuid -> cvrstate

    /* Hearing CvrStates (External concepts):
     * - Registered CvrState Exists? (matching UUID)
     *  + Update local State's variables to match incoming State's variables
     */
    void
    Hear(CvrState* cvrstate);

    void
    LoadStateChanges(std::list< std::string >& returned_states);

    bool mNotConnected;
    DatabaseHandler              mDatabaseHandler;
    // TODO: make Registered states contain a reference count, so multiple systems can Register/Unregister
    CvrStateMap                  mStates;
    Shouter<CvrState>::Listeners mListeners;
};

}

#endif /* STATE_MANAGER_H_ */
