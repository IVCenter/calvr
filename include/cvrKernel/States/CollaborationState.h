/*
 * CollaborationState.h
 *
 * Vars (type = "spatial"):
 *  - "collaborate": bool
 *  - "metadata": "metadata_state_uuid"
 *
 *  Created on: Sep 10, 2012
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef COLLABORATIONSTATE_H_
#define COLLABORATIONSTATE_H_

#include <cvrKernel/States/CvrState.h>

namespace cvr {

class CollaborationState : public CvrState
{
public:
    static std::string const TYPE;

    CollaborationState(void);

    static void
    Register(void);

    bool const
    Collaboration(void);

    void
    Collaboration(bool const collaboration);

    std::string const
    Metadata(void);

    void
    Metadata(std::string const& metadata);

protected:
    static CvrState*
    Adapter( State const& state );

    CollaborationState(State const& state);

    virtual
    ~CollaborationState(void);
};

}

#endif /* COLLABORATIONSTATE_H_ */
