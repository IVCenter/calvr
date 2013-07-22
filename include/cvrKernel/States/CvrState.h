/*
 * CvrState.h
 *
 * Base state for CalVR.  Has a type variable.
 * It also derives Shouter.
 *
 *  Created on: Apr 29, 2012
 *     Authors: David Srour <dsrour@eng.ucsd.edu>,
 *              John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 15, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef CVRSTATE_H_
#define CVRSTATE_H_

#include <map>
#include <string>
#include <osg/Referenced>

#include <cvrUtil/Shouter.hpp>

#include <cdbapp/State.h>

class CvrState : public cdb::State, public Shouter<CvrState>, public osg::Referenced
{
public:
    /*
     * Returns a CvrState* to the appropriate subclass implementation registered
     * to the given State's type, or NULL if none exist.
     */
    static CvrState*
    AdaptToDerivedCvrState(cdb::State& state);

    std::string const
    Type(void);

    bool const
    Valid(void);

    void
    Valid(bool const valid);

protected:
    typedef CvrState* (*STATIC_ADAPTER)(cdb::State const&);

    static void
    Register(std::string const& type, STATIC_ADAPTER staticAdapter);

    // To be used for new states
    CvrState(std::string const& type);

    // To be used for adapters
    CvrState(cdb::State const& state);

    virtual
    ~CvrState(void);

private:
    typedef std::map< std::string, STATIC_ADAPTER > TypeAdapterMap;
    static TypeAdapterMap mTypeAdapters;
};

#endif /* CVRSTATE_H_ */
