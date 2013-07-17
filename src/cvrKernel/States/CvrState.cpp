/*
 * CvrState.cpp
 *
 */

#include <assert.h>
#include <iostream>
#include <cvrKernel/States/CvrState.h>

CvrState::CvrState(std::string const& type)
{
    SetVariable("type", type);
    Init();
}

CvrState::CvrState(State const& state) : State(state.Variables(), state.Uuid())
{}

/*virtual*/
CvrState::~CvrState()
{}

/*virtual*/ void
CvrState::Init(void)
{
    Valid(true);
}

std::string const
CvrState::Type()
{
    std::string type;
    assert( State::NO_ERROR == GetVariable("type", type) );

    return type;
}

bool const
CvrState::Valid()
{
    bool valid;
    assert( State::NO_ERROR == GetVariable("valid",valid) );

    return valid;
}

