/*
 * CollaborationState.cpp
 *
 */

#include <cvrKernel/States/CollaborationState.h>

using namespace cvr;

/*static*/ std::string const CollaborationState::TYPE = "collaboration";

CollaborationState::CollaborationState() : CvrState(TYPE)
{
    Collaboration(false);
    Metadata("");
}

/*static*/ CvrState*
CollaborationState::Adapter( State const& state )
{
    return new CollaborationState( state );
}

/*static*/ void
CollaborationState::Register(void)
{
    static bool FIRST_ONE = true;
    if (FIRST_ONE)
    {
        FIRST_ONE = false;
        CvrState::Register(TYPE, &Adapter);
    }
}

bool const
CollaborationState::Collaboration(void)
{
    bool collaboration;
    GetVariable("collaboration", collaboration);

    return collaboration;
}

void
CollaborationState::Collaboration(bool const collaboration)
{
    SetVariable("collaboration", collaboration);
}

std::string const
CollaborationState::Metadata(void)
{
    std::string meta_uuid;
    GetVariable("metadata", meta_uuid);

    return meta_uuid;
}

void
CollaborationState::Metadata(std::string const& metadata)
{
    SetVariable("metadata", metadata);
}

// PROTECTED FUNCTIONS

CollaborationState::CollaborationState(State const& state) : CvrState(state)
{}

/*virtual*/
CollaborationState::~CollaborationState()
{}

