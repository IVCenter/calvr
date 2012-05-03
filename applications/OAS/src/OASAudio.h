/**
 * @file    OASAudio.h
 * @author  Shreenidhi Chowkwale
 *
 */

#ifndef _OAS_AUDIO_H_
#define _OAS_AUDIO_H_

#include <string>
#include <cstring>
#include <cmath>
#include <AL/alut.h>
#include "OASFileHandler.h"

namespace oas
{

/**
 * Manages all properties of the singleton Listener object in OpenAL
 */
class Listener
{
public:
    /**
     * @brief Get the singleton instance
     */
    Listener* getInstance();

    /**
     * @brief Reset the listener to default state
     */
    void release();

    /**
     * @brief Set the overall gain for the listener.
     * @param gain A gain of 0.0 will mute the listener. A gain of 1.0 is the default. 
     */
    void setGain(ALfloat gain);

    /**
     * @brief Set the position of the listener
     */
    void setPosition(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the velocity of the listener
     */
    void setVelocity(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the "look at" and "up" orientation vectors
     */
    void setOrientation(ALfloat lookAtX, ALfloat lookAtY, ALfloat lookAtZ, ALfloat upX, ALfloat upY, ALfloat upZ);

private:
    Listener();
    ~Listener();
    void _init();

    static Listener* _instance;

    ALfloat _gain;
    ALfloat _positionX, _positionY, _positionZ;
    ALfloat _velocityX, _velocityY, _velocityZ;
    ALfloat _orientationLookAtX, _orientationLookAtY, _orientationLookAtZ;
    ALfloat _orientationUpX, _orientationUpY, _orientationUpZ;
};


/**
 * Contains the sound from one particular file.
 */
class AudioBuffer
{
public:
    
    /**
     * @brief A buffer is uniquely identified by its name, which is an ALuint value.
     */
    ALuint getHandle() const;

    /**
     * @brief Get the filename that the buffer is associated with
     */
    const std::string& getFilename() const;

    /**
     * @brief Return if the buffer is valid or not
     */
    bool isValid() const;

    AudioBuffer(const std::string& filename);
    AudioBuffer();
    ~AudioBuffer();

private:
    void _init();  

    ALuint _handle;
    std::string _filename;
};

/**
 * Manages an individual sound source/emitter.
 */
class Source
{
public:
    /**
     * The state is defined as follows:
     * UNKNOWN: state is unknown
     * PLAYING: source is playing or has finished playing all the way through
     * PAUSED:  source is paused at a specific point, and playback will resume from here
     * STOPPED: source is stopped and playback will resume from the beginning
     * DELETED: source is in the process of being deleted
     */
    enum SourceState
    {
        ST_UNKNOWN = 0,
        ST_PLAYING,
        ST_PAUSED,
        ST_STOPPED,
        ST_DELETED
    };

    /**
     * @brief Get the handle for this source
     */
    ALuint getHandle() const;

    /**
     * @brief Get the name of the underlying buffer that is attached to this source
     */
    ALuint getBuffer() const;

    /**
     * @brief Return if the source is valid or not
     */
    bool isValid() const;


    /**
     * @brief Play the source all the way through
     */
    bool play();

    /**
     * @brief Stop playing the source. Playback will resume from the beginning
     */
    bool stop();

    /**
     * @brief Set the position of the source. Units are arbitrary and relative only to each other
     */
    bool setPosition(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the gain for this audio source.
     * @param gain A gain of 0.0 will mute the source. A gain of 1.0 is the default.
     */
    bool setGain(ALfloat gain);

    /**
     * @brief Set the source to play in a continuous loop, until it is stopped
     */
    bool setLoop(ALint isLoop);

    /**
     * @brief Set the velocity of the source.
     */
    bool setVelocity(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the direction of the source
     */
    bool setDirection(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Deletes the audio resources allocated for this sound source
     */
    bool deleteSource();

    /**
     * @brief Get the current state of the source
     */
    SourceState getState() const;

    /**
     * @brief Get the current position of the source
     */
    float getPositionX() const;
    float getPositionY() const;
    float getPositionZ() const;

    /**
     * @brief Get the x, y, z direction
     */
    float getDirectionX() const;
    float getDirectionY() const;
    float getDirectionZ() const;

    /**
     * @brief Get the current direction of the source
     */
    float getVelocityX() const;
    float getVelocityY() const;
    float getVelocityZ() const;

    /**
     * @brief Determine if the source is looping or not
     */
    bool isLooping() const;

    /**
     * @brief Determine if the source is directional or not
     */
    bool isDirectional() const;

    
    /**
     * @brief Resets the handle counter, and any other state applicable to all sources
     */
    static void resetSources();

    Source(ALuint buffer);
    Source();
    ~Source();

private:
    void _init();
    ALuint _generateNextHandle();
    void _clearError();
    bool _wasOperationSuccessful();

    /*
     * 'id' is used to interact with the OpenAL library, and the values are arbitrary.
     * The 'id' is strictly internal to the source, and no other object needs to know it.
     */
    ALuint _id;

    /*
     * 'handle' is used to interact with the client, and the values are guaranteed to
     * start from 1 and increment by 1 for each source that is generated.
     */
    ALuint _handle;

    ALuint _buffer;
    SourceState _state;

    ALfloat _positionX, _positionY, _positionZ;
    ALfloat _velocityX, _velocityY, _velocityZ;
    ALfloat _directionX, _directionY, _directionZ;
    ALfloat _gain;
    
    ALint _isLooping;
    bool _isValid;
    bool _isDirectional;

    static ALuint _nextHandle;
    
    static const ALfloat _kConeInnerAngle = 45.0;
    static const ALfloat _kConeOuterAngle = 360.0;
};


}

#endif

