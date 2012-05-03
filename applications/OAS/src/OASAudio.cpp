#include "OASAudio.h"

using namespace oas;

// Statics
ALuint Source::_nextHandle;
const ALfloat Source::_kConeInnerAngle;
const ALfloat Source::_kConeOuterAngle;

Listener* Listener::_instance = NULL;

// private constructor
Listener::Listener()
{
    _init();
}


// private destructor
Listener::~Listener()
{

}

// public, static
Listener* Listener::getInstance()
{
    if (!Listener::_instance)
    {
        _instance = new Listener();
    }

    return Listener::_instance;
}

void Listener::release()
{
    _init();
}

void Listener::setGain(ALfloat gain)
{
    _gain = gain;
    alListenerf(AL_GAIN, _gain);
}

void Listener::setPosition(ALfloat x, ALfloat y, ALfloat z)
{
    _positionX = x;
    _positionY = y;
    _positionZ = z;

    alListener3f(AL_POSITION, _positionX, _positionY, _positionZ);
}

void Listener::setVelocity(ALfloat x, ALfloat y, ALfloat z)
{
    _velocityX = x;
    _velocityY = y;
    _velocityZ = z;

    alListener3f(AL_VELOCITY, _velocityX, _velocityY, _velocityZ);
}

void Listener::setOrientation(ALfloat lookAtX, ALfloat lookAtY, ALfloat lookAtZ, ALfloat upX, ALfloat upY, ALfloat upZ)
{
    _orientationLookAtX = lookAtX;
    _orientationLookAtY = lookAtY;
    _orientationLookAtZ = lookAtZ;
    _orientationUpX = upX;
    _orientationUpY = upY;
    _orientationUpZ = upZ;

    alListenerfv(AL_ORIENTATION, &_orientationLookAtX);
}

// private
void Listener::_init()
{
    setGain(1.0);
    setPosition(0.0, 0.0, 0.0);
    setVelocity(0.0, 0.0, 0.0);
    setOrientation(0.0, 0.0, -1.0, 0.0, 1.0, 0.0);
}

Buffer::Buffer(const std::string& filename)
{
    _init();

    if (!filename.empty())
    {
        oas::FileHandler fileHandler;
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

Buffer::Buffer()
{
    _init();
}

Buffer::~Buffer()
{
    // Tell OpenAL to delete the resources allocated for the buffer
    if (isValid())
    {
        alDeleteBuffers(1, &_handle);
    }
}

// private
void Buffer::_init()
{
    _handle = AL_NONE;
}

// public
ALuint Buffer::getHandle() const
{
    return _handle;
}

// public
const std::string& Buffer::getFilename() const
{
    return _filename;
}

// public
bool Buffer::isValid() const
{
    return (_handle != AL_NONE);
}

Source::Source(ALuint buffer)
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

Source::Source()
{
    _init();
}

Source::~Source()
{
    _state = ST_UNKNOWN;
    if (isValid())
        alDeleteSources(1, &_id);
   
    _isValid = false;
}

// private
void Source::_init()
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
ALuint Source::_generateNextHandle()
{
    _nextHandle++;

    return _nextHandle;
}

// private
void Source::_clearError()
{
    // Error is retrieved and discarded
    alGetError();
}

// private
bool Source::_wasOperationSuccessful()
{
    // If there was no error, return true
    if (AL_NO_ERROR == alGetError())
        return true;
    else
        return false;
}

// static, public
void Source::resetSources()
{
    _nextHandle = 0;
}

bool Source::isValid() const
{
    return _isValid;
}

bool Source::isDirectional() const
{
    return _isDirectional;
}

bool Source::isLooping() const
{
    return _isLooping;
}

ALuint Source::getHandle() const
{
    return _handle;
}

ALuint Source::getBuffer() const
{
    return _buffer;
}

bool Source::play()
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alSourcePlay(_id);

        // Change state and return true iff operation successful
        if (_wasOperationSuccessful())
        {
            float offset = 0, prevOffset = 0;
            ALint state;
            alSourcef(_id, AL_SEC_OFFSET, 1.0);
/*
            do {
                alGetSourcef(_id, AL_SEC_OFFSET, &offset);
                alGetSourcei(_id, AL_SOURCE_STATE, &state);
                if (prevOffset != offset)
                {
                    std::cerr << offset << std::endl;
                    prevOffset = offset;
                }
            } while (state == AL_PLAYING);
*/
            _state = ST_PLAYING;
            return true;
        }
    }
    
    return false;
}

bool Source::stop()
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

bool Source::setPosition(ALfloat x, ALfloat y, ALfloat z)
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

bool Source::setGain(ALfloat gain)
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

bool Source::setLoop(ALint isLoop)
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

bool Source::setVelocity(ALfloat x, ALfloat y, ALfloat z)
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

bool Source::setDirection(ALfloat x, ALfloat y, ALfloat z)
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
                alSourcef(_id, AL_CONE_INNER_ANGLE, Source::_kConeInnerAngle);
                alSourcef(_id, AL_CONE_OUTER_ANGLE, Source::_kConeOuterAngle);
                _isDirectional = true;
            }

            return true;
        }
    }

    return false;
}

bool Source::deleteSource()
{
    if (isValid())
    {
        // Clear OpenAL error state
        _clearError();

        alDeleteSources(1, &_id);

        if (_wasOperationSuccessful())
        {
            _state = ST_DELETED;
            return true;
        }
    }

    return false;
}

Source::SourceState Source::getState() const
{
    return _state;
}

float Source::getPositionX() const
{
    return _positionX;
}

float Source::getPositionY() const
{
    return _positionY;
}

float Source::getPositionZ() const
{
    return _positionZ;
}

float Source::getDirectionX() const
{
    return _directionX;
}

float Source::getDirectionY() const
{
    return _directionY;
}

float Source::getDirectionZ() const
{
    return _directionZ;
}

float Source::getVelocityX() const
{
    return _velocityX;
}

float Source::getVelocityY() const
{
    return _velocityY;
}

float Source::getVelocityZ() const
{
    return _velocityZ;
}


