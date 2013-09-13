/*
 * CvrState.cpp
 *
 */

#include <assert.h>
#include <iostream>
#include <cvrKernel/States/CvrState.h>

using namespace cvr;

// static member fields
/*static*/ CvrState::TypeAdapterMap CvrState::mTypeAdapters;

/*static*/ CvrState*
CvrState::AdaptToDerivedCvrState(State& state)
{
    std::string type;
    if (State::NO_ERROR != state.GetVariable("type", type))
    {
        std::cerr << "CvrState::AdaptToDerivedCvrState passed a State without"
                << " a type variable." << std::endl;
        return NULL;
    }

    TypeAdapterMap::iterator tamit = mTypeAdapters.find(type);
    if (mTypeAdapters.end() == tamit)
    {
        std::cerr << "CvrState::AdaptToDerivedState passed a State with unknown"
                << " type \'" << type << "\'." << std::endl;
        return NULL;
    }
    
    return tamit->second(state);
}

std::string const
CvrState::Type()
{
    std::string type;
    State::ErrorType error = GetVariable("type", type);
    assert( State::NO_ERROR == error );

    return type;
}

bool const
CvrState::Valid()
{
    bool valid;
    State::ErrorType error = GetVariable("valid",valid);
    assert( State::NO_ERROR == error );

    return valid;
}

void
CvrState::Valid(bool const valid)
{
    SetVariable("valid", valid);
}

/////// BEING PROTECTED FUNCTIONS //////////

CvrState::CvrState(std::string const& type)
{
    SetVariable("type", type);
    Valid(true);
}

CvrState::CvrState(State const& state) : State(state.Variables(), state.Uuid())
{}

/*static*/ void
CvrState::Register(std::string const& type, STATIC_ADAPTER staticAdapter)
{
    TypeAdapterMap::iterator tamit = mTypeAdapters.find(type);
    if (mTypeAdapters.end() == tamit)
    {
        std::pair< std::string, STATIC_ADAPTER > adapter(type, staticAdapter);
        mTypeAdapters.insert(tamit, adapter );
    }
    else if (tamit->second != staticAdapter)
    {
        std::cerr << "Warning: Differing Adapter functions provided for derived"
                << " CvrState of type \'" << type << "\'." << std::endl;
    }
}

/*virtual*/
CvrState::~CvrState()
{}

