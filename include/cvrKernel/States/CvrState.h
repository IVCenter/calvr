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

#include <string>
#include <osg/Referenced>

#include <cvrUtil/Shouter.hpp>
#include <cvrUtil/Listener.h>

#include <cdbapp/State.h>

class AudioAnnotationState;
class AudioServerState;
class GenericAnnotationState;
class LoadState;
class MetadataState;
class SlideshowState;
class SpatialState;
class WallState;

class CvrState : public State, public Shouter<CvrState>, public osg::Referenced
{
public:
    CvrState(std::string const& type);

    CvrState(State const& state);

    virtual AudioAnnotationState*
    AsAudioAnnotation(void) { return NULL; }

    virtual AudioAnnotationState const*
    AsAudioAnnotation(void) const { return NULL; }

    virtual AudioServerState*
    AsAudioServer(void) { return NULL; }

    virtual AudioServerState const*
    AsAudioServer(void) const { return NULL; }

    virtual GenericAnnotationState*
    AsGenericAnnotation(void) { return NULL; }

    virtual GenericAnnotationState const*
    AsGenericAnnotation(void) const { return NULL; }

    virtual LoadState*
    AsLoad(void) { return NULL; }

    virtual LoadState const*
    AsLoad(void) const { return NULL; }

    virtual MetadataState*
    AsMetadata(void) { return NULL; }

    virtual MetadataState const*
    AsMetadata(void) const { return NULL; }

    virtual SlideshowState*
    AsSlideshow(void) { return NULL; }

    virtual SlideshowState const*
    AsSlideshow(void) const { return NULL; }

    virtual SpatialState*
    AsSpatial(void) { return NULL; }

    virtual SpatialState const*
    AsSpatial(void) const { return NULL; }

    virtual WallState*
    AsWall(void) { return NULL; }

    virtual WallState const*
    AsWall(void) const { return NULL; }

    std::string const
    Type(void);

    bool const
    Valid(void);

    inline void
    Valid(bool const valid) { SetVariable("valid", valid); }

protected:
    virtual
    ~CvrState(void);

private:
    virtual void
    Init(void);
};

#endif /* CVRSTATE_H_ */
