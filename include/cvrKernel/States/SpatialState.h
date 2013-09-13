/*
 * SpatialState.h
 *
 * Vars (type = "spatial"):
 *  - "position": [x, y, z]
 *  - "rotation": [x, y, z, w]
 *  - "scale"   : [x, y, z]
 *  - "metadata": "metadata_state_uuid"
 *
 *  Created on: Apr 29, 2012
 *     Authors: David Srour <dsrour@eng.ucsd.edu>,
 *              John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Sep 3, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef SPATIALSTATE_H_
#define SPATIALSTATE_H_

#include <osg/Vec3>
#include <osg/Quat>
#include <cvrKernel/States/CvrState.h>

namespace cvr {

class SpatialState : public CvrState
{
public:
    static std::string const TYPE;

    SpatialState(void);

    static void
    Register(void);

    osg::Vec3 const
    Position(void);

    void
    Position(osg::Vec3 const& position);

    osg::Quat const
    Rotation(void);

    void
    Rotation(osg::Quat const& rotation);

    osg::Vec3 const
    Scale(void);

    void
    Scale(osg::Vec3 const& scale);

    bool const
    Navigation(void);

    void
    Navigation(bool const navigation);

    std::string const
    Metadata(void);

    void
    Metadata(std::string const& metadata);

    std::string const
    Parent(void);

    void
    Parent(std::string const& spatial);

protected:
    static CvrState*
    Adapter( State const& state );

    SpatialState(State const& state);

    virtual
    ~SpatialState(void);
};

}

#endif /* SPATIALSTATE_H_ */
