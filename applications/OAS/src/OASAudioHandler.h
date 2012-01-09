/**
 * @file    OASAudioHandler.h
 * @author  Shreenidhi Chowkwale
 */

#ifndef _OAS_AUDIO_HANDLER_H_
#define _OAS_AUDIO_HANDLER_H_

#include <map>
#include <iostream>
#include <cmath>
#include <AL/alut.h>
#include "OASAudio.h"
#include "OASLogger.h"

namespace oas
{

/**
 * This class manages multiple audio sources and buffers, and provides wrappers for modifying these
 * units.
 */

/** @TODO Change BufferMap to use C++ strings instead of C strings */
//class StringComparison
//{
//    public:
//        bool operator() (const char *lhs, const char *rhs) const;
//};

// Buffer Map types
typedef std::map<std::string, Buffer*>      BufferMap;
typedef BufferMap::iterator                 BufferMapIterator;
typedef BufferMap::const_iterator           BufferMapConstIterator;
typedef std::pair<std::string, Buffer*>     BufferPair;

// Source Map types
typedef std::map<ALuint, Source*>           SourceMap;
typedef SourceMap::iterator                 SourceMapIterator;
typedef SourceMap::const_iterator           SourceMapConstIterator;
typedef std::pair<ALuint, Source*>          SourcePair;

class AudioHandler
{
public:
    static bool initialize();
    static void release();

    /**
     * @brief Gets the buffer that is associated with the file pointed to by filename. 
     *        Creates a new buffer if necessary.
     */
    static ALuint getBuffer(const std::string& filename);

    /**
     * @brief Create a new source based on that buffer.
     */
    static ALuint createSource(const ALuint buffer);

    /**
     * @brief Create a new source with the audio file that is pointed to by filename
     */
    static ALuint createSource(const std::string& filename);


    /**
     * @note:
     * The following functions operate on existing sources. If the given source handle is invalid,
     * there is no change made to the OpenAL state, and the function does nothing.
     *
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
     * @brief Set the direction the source is pointing at, using cartesian coordinates.
     */
    static void setSourceDirection(const ALuint source, const ALfloat x, const ALfloat y, const ALfloat z);

    /**
     * @brief Set the direction the source is pointing at with just an angle
     * @param angleInDegrees The angle must be given in degrees, not radians.
     */
    static void setSourceDirection(const ALuint source, const ALfloat angleInDegrees);

private:
    static BufferMap _bufferMap;
    static SourceMap _sourceMap;

    static Source* _recentSource;

    static Source* _getSource(const ALuint source);

};

}
#endif

