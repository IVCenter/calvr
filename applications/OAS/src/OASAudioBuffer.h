/**
 * @file    OASAudioBuffer.h
 * @author  Shreenidhi Chowkwale
 *
 */

#ifndef _OAS_AUDIO_BUFFER_H_
#define _OAS_AUDIO_BUFFER_H_

#include <string>
#include <AL/alut.h>
#include "OASFileHandler.h"

namespace oas
{

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

    /**
     * @brief Creates a new audio buffer based on the given file
     * @param filename
     */
    AudioBuffer(const std::string& filename);

    /**
     * @brief Create a new buffer based on the specified waveform.
     * @param waveShape Sine        -> waveShape = 1
     *                  Square      -> waveShape = 2
     *                  Sawtooth    -> waveShape = 3
     *                  Whitenoise  -> waveShape = 4
     *                  Impulse     -> waveShape = 5
     * @param frequency Frequency of the waveform, in hertz
     * @param phase Phase of the waveform, in degrees from -180 to +180
     * @param duration Duration of waveform in seconds
     */
    AudioBuffer(ALint waveShape, ALfloat frequency, ALfloat phase, ALfloat duration);

    AudioBuffer();
    ~AudioBuffer();

private:
    void _init();

    ALuint _handle;
    std::string _filename;
};

}

#endif // end _OAS_AUDIO_BUFFER_H_
