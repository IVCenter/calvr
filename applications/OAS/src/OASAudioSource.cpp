#include "OASAudioSource.h"
#include "OASLogger.h"

using namespace oas;

// Statics
ALuint AudioSource::_nextHandle;
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

    // Set the source to be relative to the listener
    alSourcei(_id, AL_SOURCE_RELATIVE, AL_TRUE);

    _isValid = _wasOperationSuccessful();
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
    _isValid = false;
    _state = ST_UNKNOWN;
}

// private
ALuint AudioSource::_generateNextHandle()
{
    _nextHandle++;

    return _nextHandle;
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
        	oas::Logger::errorf("More information provided by ALUT: \"%s\"", alutGetErrorString(alutError));
        }

        return false;
    }
}

// static, public
void AudioSource::resetSources()
{
    _nextHandle = 0;
}

bool AudioSource::isValid() const
{
    return _isValid;
}

void AudioSource::invalidate()
{
	_isValid = false;
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

