#include "OASAudioBuffer.h"

using namespace oas;


AudioBuffer::AudioBuffer(const std::string& filename)
{
    _init();

    if (!filename.empty())
    {
        FileHandler fileHandler;
        int fileSize;
        void *data;

        data = fileHandler.readFile(filename, fileSize);

        if (data)
        {
            _handle = alutCreateBufferFromFileImage(data, fileSize);

            if (AL_NONE != _handle)
            {
                _filename = std::string(filename);
            }
        }
    }
}

AudioBuffer::AudioBuffer()
{
    _init();
}

AudioBuffer::~AudioBuffer()
{
    // Tell OpenAL to delete the resources allocated for the buffer
    if (isValid())
    {
        alDeleteBuffers(1, &_handle);
    }
}

// private
void AudioBuffer::_init()
{
    _handle = AL_NONE;
}

// public
ALuint AudioBuffer::getHandle() const
{
    return _handle;
}

// public
const std::string& AudioBuffer::getFilename() const
{
    return _filename;
}

// public
bool AudioBuffer::isValid() const
{
    return (_handle != AL_NONE);
}
