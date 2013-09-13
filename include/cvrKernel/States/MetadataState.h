/*
 * MetadataState.h
 *
 * Vars (type = "metadata"):
 *  -   "path": "path/to/data"
 *  - "volume": [width, height, depth]
 *
 *  Created on: Apr 29, 2012
 *     Authors: David Srour <dsrour@eng.ucsd.edu>,
 *              John Mangan <jmangan@eng.ucsd.edu>
 *
 *  CalVR Port: Jul 22, 2013
 *      Author: John Mangan <jmangan@eng.ucsd.edu>
 */

#ifndef METADATASTATE_H_
#define METADATASTATE_H_

#include <osg/Vec3>
#include <cvrKernel/States/CvrState.h>

namespace cvr {

class MetadataState : public CvrState
{
public:
    static std::string const TYPE;

    MetadataState(void);

    static void
    Register(void);

    std::string const
    Path(void);

    void
    Path(std::string const& path);

    osg::Vec3 const
    Volume(void);

    void
    Volume(osg::Vec3 const& volume);

protected:
    static CvrState*
    Adapter( State const& state );

    MetadataState(State const& state);

    virtual
    ~MetadataState(void);
};

}

#endif /* METADATASTATE_H_ */
