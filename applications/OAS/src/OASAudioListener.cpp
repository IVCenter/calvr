#include "OASAudioListener.h"
#include "OASLogger.h"

using namespace oas;

AudioListener& AudioListener::getInstance()
{
    static AudioListener instance;

    return instance;
}

bool AudioListener::isSoundSource() const
{
    return false;
}

bool AudioListener::update()
{
    return false;
}

unsigned int AudioListener::getHandle() const
{
    return 0;
}

bool AudioListener::setGain(ALfloat gain)
{
    _clearError();

    alListenerf(AL_GAIN, gain);

    if (_wasOperationSuccessful())
    {
        _gain = gain;
        return true;
    }

    return false;
}

bool AudioListener::setPosition(ALfloat x, ALfloat y, ALfloat z)
{
    _clearError();

    alListener3f(AL_POSITION, x, y, z);

    if (_wasOperationSuccessful())
    {
        _positionX = x;
        _positionY = y;
        _positionZ = z;
        return true;
    }

    return false;
}

bool AudioListener::setVelocity(ALfloat x, ALfloat y, ALfloat z)
{
    _clearError();

    alListener3f(AL_VELOCITY, x, y, z);

    if (_wasOperationSuccessful())
    {
        _velocityX = x;
        _velocityY = y;
        _velocityZ = z;
        return true;
    }

    return false;
}

bool AudioListener::setOrientation(ALfloat atX, ALfloat atY, ALfloat atZ,
                                   ALfloat upX, ALfloat upY, ALfloat upZ)
{
    _clearError();

    ALfloat orientation[6] = {atX, atY, atZ, upX, upY, upZ};

    alListenerfv(AL_ORIENTATION, orientation);

    if (_wasOperationSuccessful())
    {
        _orientation[0] = atX;
        _orientation[1] = atY;
        _orientation[2] = atZ;
        _orientation[3] = upX;
        _orientation[4] = upY;
        _orientation[5] = upZ;
        return true;
    }

    return false;
}

float AudioListener::getOrientationLookAtX() const
{
    return _orientation[0];
}

float AudioListener::getOrientationLookAtY() const
{
    return _orientation[1];
}

float AudioListener::getOrientationLookAtZ() const
{
    return _orientation[2];
}

float AudioListener::getOrientationUpX() const
{
    return _orientation[3];
}

float AudioListener::getOrientationUpY() const
{
    return _orientation[4];
}

float AudioListener::getOrientationUpZ() const
{
    return _orientation[5];
}

const char* AudioListener::getLabelForIndex(int index) const
{
    static const int k_numLabels = 13;
    static const char* labels[k_numLabels] =
    { "Gain", "PosX", "PosY", "PosZ", "VelX", "VelY", "VelZ", "LookAtX", "LookAtY", "LookAtZ",
      "UpX", "UpY", "UpZ"
    };

    if (index >= 0 && index < k_numLabels)
        return labels[index];
    else
        return "";
}

std::string AudioListener::getStringForIndex(int index) const
{
    char buffer[50] = {0};

    switch (index)
    {
        // Gain
        case 0:
            sprintf(buffer, "%.2f", getGain());
            break;
        // Position X
        case 1:
            sprintf(buffer, "%.3f", getPositionX());
            break;
        // Position Y
        case 2:
            sprintf(buffer, "%.3f", getPositionY());
            break;
        // Position Z
        case 3:
            sprintf(buffer, "%.3f", getPositionZ());
            break;
        // Velocity X
        case 4:
            sprintf(buffer, "%.3f", getVelocityX());
            break;
        // Velocity Y
        case 5:
            sprintf(buffer, "%.3f", getVelocityY());
            break;
        // Velocity Z
        case 6:
            sprintf(buffer, "%.3f", getVelocityZ());
            break;
        //Look At X
        case 7:
            sprintf(buffer, "%.3f", getOrientationLookAtX());
            break;
        // Look At Y
        case 8:
            sprintf(buffer, "%.3f", getOrientationLookAtY());
            break;
        // Look At Z
        case 9:
            sprintf(buffer, "%.3f", getOrientationLookAtZ());
            break;
        // Up X
        case 10:
            sprintf(buffer, "%.3f", getOrientationUpX());
            break;
        // Up Y
        case 11:
            sprintf(buffer, "%.3f", getOrientationUpY());
            break;
        // Up Z
        case 12:
            sprintf(buffer, "%.3f", getOrientationUpZ());
            break;
        default:
            break;
    }

    return buffer;

}


// Privates
AudioListener::AudioListener()
{
    _init();
}

AudioListener::~AudioListener()
{

}

void AudioListener::_init()
{
    _gain = 1.0;
    _positionX = 0.0;
    _positionY = 0.0;
    _positionZ = 0.0;
    _velocityX = 0.0;
    _velocityY = 0.0;
    _velocityZ = 0.0;

    _orientation[0] = 0.0;
    _orientation[1] = 0.0;
    _orientation[2] = -1.0;

    _orientation[3] = 0.0;
    _orientation[4] = 1.0;
    _orientation[5] = 0.0;
}

// private
void AudioListener::_clearError()
{
    // Error is retrieved and discarded
    alGetError();
}

// private
bool AudioListener::_wasOperationSuccessful()
{
    ALenum alError = alGetError();

    // If there was no error, return true
    if (AL_NO_ERROR == alError)
    {
        return true;
    }
    else
    {
        oas::Logger::errorf("OpenAL error for the Listener. Error code = %d", alError);

        ALenum alutError = alutGetError();

        if (ALUT_ERROR_NO_ERROR != alutError)
        {
            oas::Logger::errorf("More information provided by ALUT: \"%s\"",
                                alutGetErrorString(alutError));
        }

        return false;
    }

}
