#include "OASAudio.h"

using namespace oas;

// Statics
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
    alGetError();

    // Generate source
    alGenSources(1, &_id);
    // Bind buffer to source
    alSourcei(_id, AL_BUFFER, buffer);
    _buffer = buffer;

    // Set the source to be relative to the listener
    alSourcei(_id, AL_SOURCE_RELATIVE, AL_TRUE);

    ALenum error = alGetError();

    // If error didn't occur
    if (AL_NO_ERROR == error)
    {
        _isValid = true;
    }
}

Source::Source()
{
    _init();
}

Source::~Source()
{
    _isValid = false;
    alDeleteSources(1, &_id);
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
}

// private
ALuint Source::_generateNextHandle()
{
    static ALuint nextHandle = 0;

    nextHandle++;

    return nextHandle;
}

bool Source::isValid() const
{
    return _isValid;
}

bool Source::isDirectional() const
{
    return _isDirectional;
}

ALuint Source::getHandle() const
{
    return _handle;
}

ALuint Source::getBuffer() const
{
    return _buffer;
}

void Source::play()
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSourcePlay(_id);
    }
}

void Source::stop()
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSourceStop(_id);
    }
}

void Source::setPosition(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSource3f(_id, AL_POSITION, x, y, z);

        _positionX = x;
        _positionY = y;
        _positionZ = z;

    }
}

void Source::setGain(ALfloat gain)
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSourcef(_id, AL_GAIN, gain);
        _gain = gain;
    }
}

void Source::setLoop(ALint isLoop)
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        _isLooping = ((isLoop != 0) ? AL_TRUE : AL_FALSE);
        alSourcei(_id, AL_LOOPING, _isLooping);
    }
}

void Source::setVelocity(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSource3f(_id, AL_VELOCITY, x, y, z);

        _velocityX = x;
        _velocityY = y;
        _velocityZ = z;
    }
}

void Source::setDirection(ALfloat x, ALfloat y, ALfloat z)
{
    if (isValid())
    {
        // Clear OpenAL error state
        alGetError();

        alSource3f(_id, AL_DIRECTION, x, y, z);

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
    }
}

ALfloat Source::getDirectionX()
{
    return _directionX;
}

ALfloat Source::getDirectionY()
{
    return _directionY;
}

ALfloat Source::getDirectionZ()
{
    return _directionZ;
}

