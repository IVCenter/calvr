#include "OASAudioSource.h"
#include "OASLogger.h"

using namespace oas;

// Statics
ALuint AudioSource::_nextHandle = 0;
const ALfloat AudioSource::_kConeInnerAngle;
const ALfloat AudioSource::_kConeOuterAngle;


AudioSource::AudioSource(ALuint buffer)
{
    // Set values to default
    _init();

    // Clear OpenAL error state
    _clearError();

    // Generate source
    alGenSources(1, &_id);
    // Bind buffer to source
    alSourcei(_id, AL_BUFFER, buffer);
    _buffer = buffer;

    _isValid = _wasOperationSuccessful();

    // Update the state of the audio source
    if (isValid())
        update(true);
}

AudioSource::AudioSource()
{
    _init();
}

AudioSource::~AudioSource()
{
    _state = ST_UNKNOWN;
    if (isValid() && alIsSource(_id))
    {
        alDeleteSources(1, &_id);
    }
}

// private
void AudioSource::_init()
{
    _id = AL_NONE;
    _handle = this->_generateNextHandle();
    _buffer = AL_NONE;
    _positionX = _positionY = _positionZ = 0.0;
    _velocityX = _velocityY = _velocityZ = 0.0;
    _directionX = _directionY = _directionZ = 0.0;
    _gain = 1.0;
    _pitch = 1.0;
    _isValid = false;
    _isLooping = false;
    _isDirectional = false;
    _state = ST_UNKNOWN;
}

// private
ALuint AudioSource::_generateNextHandle()
{
    _nextHandle++;

    return _nextHandle - 1;
}

// private
void AudioSource::_clearError()
{
    // Error is retrieved and discarded
    alGetError();
}

// private
bool AudioSource::_wasOperationSuccessful()
{
    ALenum alError = alGetError();

    // If there was no error, return true
    if (AL_NO_ERROR == alError)
    {
        return true;
    }
    else
    {
        oas::Logger::errorf("OpenAL error for sound source %d. Error code = %d", this->_handle,
        		alError);

        ALenum alutError = alutGetError();
        if (ALUT_ERROR_NO_ERROR != alutError)
        {
        	oas::Logger::errorf("More information provided by ALUT: \"%s\"",
        	                    alutGetErrorString(alutError));
        }

        return false;
    }
}

// static, public
void AudioSource::resetSources()
{
    _nextHandle = 0;
}

bool AudioSource::update(bool forceUpdate)
{
    ALint alState;
    SourceState newState;

    // The state does not need to be checked if the source isn't being played
    if (!forceUpdate && this->_state != ST_PLAYING)
        return false;

    // Retrieve state information from OpenAL
    alGetSourcei(this->_id, AL_SOURCE_STATE, &alState);

    switch (alState)
    {
        case AL_PLAYING:
            newState = ST_PLAYING;
            break;
        case AL_STOPPED:
            newState = ST_STOPPED;
            break;
        case AL_INITIAL:
            newState = ST_INITIAL;
            break;
        case AL_PAUSED:
            newState = ST_PAUSED;
            break;
        default:
            newState = ST_UNKNOWN;
            break;
    }

    // If the new state is the same as the old state, return false
    if (newState == this->_state)
    {
        return false;
    }
    else
    {
        this->_state = newState;
        return true;
    }
}

bool AudioSource::isDirectional() const
{
    return _isDirectional;
}

bool AudioSource::isLooping() const
{
    return _isLooping;
}

ALuint AudioSource::getHandle() const
{
    return _handle;
}

ALuint AudioSource::getBuffer() const
{
    return _buffer;
}

bool AudioSource::play()
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourcePlay(_id);

        // Change state and return true iff operation successful
        if (_wasOperationSuccessful())
        {
            _state = ST_PLAYING;
            return true;
        }
    }

    return false;
}

bool AudioSource::stop()
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourceStop(_id);

        // Change state and return true iff operation successful
        if (_wasOperationSuccessful())
        {
            _state = ST_STOPPED;
            return true;
        }
    }

    return false;
}

bool AudioSource::setPosition(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSource3f(_id, AL_POSITION, x, y, z);

        if (_wasOperationSuccessful())
        {
            _positionX = x;
            _positionY = y;
            _positionZ = z;
            return true;
        }
    }

    return false;
}

bool AudioSource::setGain(ALfloat gain)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourcef(_id, AL_GAIN, gain);

        if (_wasOperationSuccessful())
        {
            _gain = gain;
            return true;
        }
    }

    return false;
}

bool AudioSource::setLoop(ALint isLoop)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourcei(_id, AL_LOOPING, (isLoop != 0) ? AL_TRUE : AL_FALSE);

        if (_wasOperationSuccessful())
        {
            _isLooping = ((isLoop != 0) ? AL_TRUE : AL_FALSE);
            return true;
        }
    }

    return false;
}

bool AudioSource::setVelocity(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSource3f(_id, AL_VELOCITY, x, y, z);

        if (_wasOperationSuccessful())
        {
            _velocityX = x;
            _velocityY = y;
            _velocityZ = z;
            return true;
        }
    }

    return false;
}

bool AudioSource::setDirection(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSource3f(_id, AL_DIRECTION, x, y, z);

        if (_wasOperationSuccessful())
        {
            _directionX = x;
            _directionY = y;
            _directionZ = z;

            // If zero vector, i.e. no direction, then set source as non-directional
            if ((x == 0.0) && (y == 0.0) && (z == 0.0))
            {
                _isDirectional = false;
            }
            // Else, the vector specifies some direction.
            // Set directional cone properties if they weren't already set
            else if (!isDirectional())
            {
                // Set the inner and outer cone angles
                alSourcef(_id, AL_CONE_INNER_ANGLE, AudioSource::_kConeInnerAngle);
                alSourcef(_id, AL_CONE_OUTER_ANGLE, AudioSource::_kConeOuterAngle);
                _isDirectional = true;
            }

            return true;
        }
    }

    return false;
}

bool AudioSource::setPitch(ALfloat pitchFactor)
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourcef(_id, AL_PITCH, pitchFactor);

        if (_wasOperationSuccessful())
        {
            _pitch = pitchFactor;
            return true;
        }
    }

    return false;
}

bool AudioSource::deleteSource()
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alDeleteSources(1, &_id);

        if (_wasOperationSuccessful())
        {
            _state = ST_DELETED;
            this->invalidate();
            return true;
        }
        else
        {
        	return false;
        }
    }
    else
    {
    	return true;
    }
}

AudioSource::SourceState AudioSource::getState() const
{
    return _state;
}

float AudioSource::getPitch() const
{
    return _pitch;
}

float AudioSource::getDirectionX() const
{
    return _directionX;
}

float AudioSource::getDirectionY() const
{
    return _directionY;
}

float AudioSource::getDirectionZ() const
{
    return _directionZ;
}

bool AudioSource::isSoundSource() const
{
    return true;
}

const char* AudioSource::getLabelForIndex(int index) const
{
    static const int k_numLabels = 13;
    static const char* labels[k_numLabels] =
    { "Status", "Gain", "Loop", "Pitch", "PosX", "PosY", "PosZ", "VelX", "VelY", "VelZ", "DirX",
      "DirY", "DirZ"
    };

    if (index >= 0 && index < k_numLabels)
        return labels[index];
    else
        return "";
}

std::string AudioSource::getStringForIndex(int index) const
{
    char buffer[50] = {0};

    switch (index)
    {
        // Status
        case 0:
            if (ST_INITIAL == getState())
                sprintf(buffer, "Stopped");
            else if (ST_PLAYING == getState())
                sprintf(buffer, "Playing");
            else if (ST_STOPPED == getState())
                sprintf(buffer, "Stopped");
            else if (ST_PAUSED == getState())
                sprintf(buffer, "Paused");
            else if (ST_DELETED == getState())
                sprintf(buffer, "Deleting");
            else
                sprintf(buffer, "Unknown");
            break;
        // Gain
        case 1:
            sprintf(buffer, "%.2f", getGain());
            break;
        // Looping
        case 2:
            if (isLooping())
                sprintf(buffer, "On");
            else
                sprintf(buffer, "Off");
            break;
        // Pitch
        case 3:
            sprintf(buffer, "%.3f", getPitch());
            break;
        // Position X
        case 4:
            sprintf(buffer, "%.3f", getPositionX());
            break;
        // Position Y
        case 5:
            sprintf(buffer, "%.3f", getPositionY());
            break;
        // Position Z
        case 6:
            sprintf(buffer, "%.3f", getPositionZ());
            break;
        // Velocity X
        case 7:
            sprintf(buffer, "%.3f", getVelocityX());
            break;
        // Velocity Y
        case 8:
            sprintf(buffer, "%.3f", getVelocityY());
            break;
        // Velocity Z
        case 9:
            sprintf(buffer, "%.3f", getVelocityZ());
            break;
        // Direction X
        case 10:
            sprintf(buffer, "%.3f", getDirectionX());
            break;
        // Direction Y
        case 11:
            sprintf(buffer, "%.3f", getDirectionY());
            break;
        // Direction Z
        case 12:
            sprintf(buffer, "%.3f", getDirectionZ());
            break;
        default:
            break;
    }

    return buffer;
}

int AudioSource::getIndexCount()
{
    return 13;
}
