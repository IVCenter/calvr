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

AudioBuffer::AudioBuffer(ALint waveShape, ALfloat frequency, ALfloat phase, ALfloat duration)
{
    _init();

    // Create the buffer, according to the waveshape
    switch (waveShape)
    {
        case 1:
           _handle = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, frequency, phase, duration);
           break;
        case 2:
           _handle = alutCreateBufferWaveform(ALUT_WAVEFORM_SQUARE, frequency, phase, duration);
           break;
        case 3:
           _handle = alutCreateBufferWaveform(ALUT_WAVEFORM_SAWTOOTH, frequency, phase, duration);
           break;
        case 4:
           _handle = alutCreateBufferWaveform(ALUT_WAVEFORM_WHITENOISE, frequency, phase, duration);
           break;
        case 5:
           _handle = alutCreateBufferWaveform(ALUT_WAVEFORM_IMPULSE, frequency, phase, duration);
           break;
        default:
            _handle = AL_NONE;
            break;
    }

    // If buffer generated successfully, generate a unique filename for this buffer
    if (AL_NONE != _handle)
    {
        char buffer[250];
        sprintf(buffer, "wave: %i, %f, %f, %f", waveShape, frequency, phase, duration);
        _filename = std::string(buffer);
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
