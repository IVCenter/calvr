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

    AudioBuffer(const std::string& filename);
    AudioBuffer();
    ~AudioBuffer();

private:
    void _init();

    ALuint _handle;
    std::string _filename;
};

}

#endif // end _OAS_AUDIO_BUFFER_H_
