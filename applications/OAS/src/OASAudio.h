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
class Buffer
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

    Buffer(const std::string& filename);
    Buffer();
    ~Buffer();

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
     * @brief Return if the source is directional or not
     */
    bool isDirectional() const;

    /**
     * @brief Play the source all the way through
     */
    void play();

    /**
     * @brief Stop playing the source
     */
    void stop();

    /**
     * @brief Set the position of the source. Units are arbitrary and relative only to each other
     */
    void setPosition(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the gain for this audio source.
     * @param gain A gain of 0.0 will mute the source. A gain of 1.0 is the default.
     */
    void setGain(ALfloat gain);

    /**
     * @brief Set the source to play in a continuous loop, until it is stopped
     */
    void setLoop(ALint isLoop);

    /**
     * @brief Set the velocity of the source.
     */
    void setVelocity(ALfloat x, ALfloat y, ALfloat z);

    /**
     * @brief Set the direction of the source
     */
    void setDirection(ALfloat x, ALfloat y, ALfloat z);


    /**
     * @brief Get the x, y, z direction
     */
    ALfloat getDirectionX();
    ALfloat getDirectionY();
    ALfloat getDirectionZ();

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

