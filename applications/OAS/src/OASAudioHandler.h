/**
 * @file    OASAudioHandler.h
 * @author  Shreenidhi Chowkwale
 */

#ifndef _OAS_AUDIO_HANDLER_H_
#define _OAS_AUDIO_HANDLER_H_

#include <map>
#include <iostream>
#include <cmath>
#include <queue>
#include <AL/alut.h>
#include "OASAudioSource.h"
#include "OASAudioListener.h"
#include "OASAudioBuffer.h"
#include "OASLogger.h"

namespace oas
{

/**
 * This class manages multiple audio sources and buffers, and provides wrappers for modifying these
 * units.
 */

//class StringComparison
//{
//    public:
//        bool operator() (const char *lhs, const char *rhs) const;
//};

// Buffer Map types
typedef std::map<std::string, AudioBuffer*>     BufferMap;
typedef BufferMap::iterator                     BufferMapIterator;
typedef BufferMap::const_iterator               BufferMapConstIterator;
typedef std::pair<std::string, AudioBuffer*>    BufferPair;

// Source Map types
typedef std::map<ALuint, AudioSource*>          SourceMap;
typedef SourceMap::iterator                     SourceMapIterator;
typedef SourceMap::const_iterator               SourceMapConstIterator;
typedef std::pair<ALuint, AudioSource*>         SourcePair;

class AudioHandler
{
public:
    static bool initialize(std::string const& deviceString);
    static void release();

    /**
     * @brief Gets the buffer that is associated with the file pointed to by filename. 
     *        Creates a new buffer if necessary.
     */
    static ALuint getBuffer(const std::string& filename);

    /**
     * @brief Create a new source based on the input buffer
     * @retval Unique handle for the created source, or -1 on error
     */
    static int createSource(const ALuint buffer);

    /**
     * @brief Create a new source with the audio file that is pointed to by filename
     * @retval Unique handle for the created source, or -1 on error
     */
    static int createSource(const std::string& filename);

    /**
     * @brief Create a new source based on the specified waveform.
     * @param waveShape Sine        -> waveShape = 1
     *                  Square      -> waveShape = 2
     *                  Sawtooth    -> waveShape = 3
     *                  Whitenoise  -> waveShape = 4
     *                  Impulse     -> waveShape = 5
     * @param frequency Frequency of the waveform, in hertz
     * @param phase Phase of the waveform, in degrees from -180 to +180
     * @param duration Duration of waveform in seconds
     * @retval Unique handle for the created source, or -1 on error
     */
    static int createSource(ALint waveShape, ALfloat frequency, ALfloat phase, ALfloat duration);

    /**
     * @brief Retrieve a copy of the most recently modified unit
     * @retval NULL if none exists
     */
    static const AudioUnit* getRecentlyModifiedAudioUnit();

    /**
     * @brief Retrieve copies of all updated sources inside the given queue
     * @param sources
     */
    static void populateQueueWithUpdatedSources(std::queue <const AudioUnit*> &sources);

    /**
     * @note:
     * The following functions operate on existing sources. If the given source handle is invalid,
     * there is no change made to the OpenAL state, and the function does nothing.
     *
     * If the operation is successful, the particular source is marked as the most recently modified source.
     * The most recently modified source can then be retrieved with the appropriate function.
     */

    /**
     * @brief Deletes the source with the given handle. Any associated buffers are not deleted.
     */
    static void deleteSource(const ALuint source);

    /**
     * @brief Begin playing the source with the given handle.
     */
    static void playSource(const ALuint source);

    /**
     * @brief Stop playing the source with the given handle.
     */
    static void stopSource(const ALuint source);

    /**
     * @brief Set the source's position.
     */
    static void setSourcePosition(const ALuint source, const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the source's gain. 
     * @param gain A value 0.0 will effectively mute the source.
     */
    static void setSourceGain(const ALuint source, const ALfloat gain);

    /**
     * @brief Set the source to play repeatedly
     * @param isLoop A value of 1 will enable looping, and a value of 0 will disable looping.
     */
    static void setSourceLoop(const ALuint source, const ALint isLoop);

    /**
     * @brief Set the source to be moving at the given speed in the same direction that it is facing.
     */
    static void setSourceSpeed(const ALuint source, const ALfloat speed);

    /**
     * @brief Set the source to be moving with the given velocity
     */
    static void setSourceVelocity(const ALuint source, const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the direction the source is pointing at, using Cartesian coordinates.
     */
    static void setSourceDirection(const ALuint source, const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the direction the source is pointing at with just an angle.
     * @param angleInRadians The angle must be given in radians
     */
    static void setSourceDirection(const ALuint source, const ALfloat angleInRadians);

    /**
     * @brief Change the pitch of the source.
     * @param pitchFactor Doubling the factor will increase by one octave, and halving will decrease by one octave.
     *                    Default = 1.
     */
    static void setSourcePitch(const ALuint source, const ALfloat pitchFactor);

    /**
     * @brief Change the overall gain via the listener object
     */
    static void setListenerGain(const ALfloat gain);

    /**
     * @brief Set the position of the listener
     */
    static void setListenerPosition(const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the velocity of the listener
     */
    static void setListenerVelocity(const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the listener orientation
     */
    static void setListenerOrientation(const ALfloat atX, const ALfloat atY, const ALfloat atZ,
                                       const ALfloat upX, const ALfloat upY, const ALfloat upZ);

private:
    static AudioSource* _getSource(const ALuint source);
    static void _clearRecentlyModifiedAudioUnit();
    static void _setRecentlyModifiedAudioUnit(const AudioUnit*);

    static BufferMap _bufferMap;
    static SourceMap _sourceMap;

    static AudioSource* _recentSource;

    static const AudioUnit *_recentlyModifiedAudioUnit;

    static std::string _deviceString;
    static ALCdevice* _device;
    static ALCcontext* _context;
};

}
#endif

