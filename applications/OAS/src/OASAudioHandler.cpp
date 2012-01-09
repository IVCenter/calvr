#include "OASAudioHandler.h"

using namespace oas;

// Statics
BufferMap       AudioHandler::_bufferMap;
SourceMap       AudioHandler::_sourceMap;
Source*         AudioHandler::_recentSource;

// public, static
bool AudioHandler::initialize()
{
    if (!alutInit(NULL, NULL))
    {
        ALenum error = alutGetError();
        oas::Logger::errorf("AudioHandler - %s", alutGetErrorString(error));
        return false;
    }

    _recentSource = NULL;

    return true;
}

// public, static
void AudioHandler::release()
{
    // Release the sources
    SourceMapIterator sIter;

    for (sIter = _sourceMap.begin(); sIter != _sourceMap.end(); sIter++)
    {
        delete sIter->second;
    }
    
    _sourceMap.clear();

    // Release the buffers
    BufferMapIterator bIter;

    for (bIter = _bufferMap.begin(); bIter != _bufferMap.end(); bIter++)
    {
        delete bIter->second;
    }

    _bufferMap.clear();

    // Let ALUT do any remaining cleanup to destroy the context
    if (!alutExit())
    {
        ALenum error = alutGetError();
        oas::Logger::errorf("AudioHandler - %s", alutGetErrorString(error));
    }
}

// public, static
ALuint AudioHandler::getBuffer(const std::string& filename)
{
    if (filename.empty())
    {
        return AL_NONE;
    }

    // See if buffer with that filename exists already
    BufferMapIterator iterator = _bufferMap.find(filename);

    // If something was found 
    if (_bufferMap.end() != iterator)
    {
        // If the found buffer was valid
        if(iterator->second && iterator->second->isValid())
        {
            return iterator->second->getHandle();
        }
        // Else remove the previous entry
        else
        {
            if (iterator->second)
            {
                delete iterator->second;
            }
            _bufferMap.erase(iterator);
        }
    }

    // Make a new buffer
    Buffer *newBuffer = new Buffer(filename);
    if (!newBuffer->isValid())
    {
        delete newBuffer;
        oas::Logger::warnf("AudioHandler - Could not create a sound buffer for \"%s\"",
                            filename.c_str());
        return AL_NONE;
    }

    _bufferMap.insert(BufferPair(newBuffer->getFilename().c_str(), newBuffer));
    return newBuffer->getHandle();
}

// public, static
ALuint AudioHandler::createSource(ALuint buffer)
{
    if (AL_NONE == buffer)
    {
        return AL_NONE;
    }

    Source *newSource = new Source(buffer);

    if (newSource->isValid())
    {
        _sourceMap.insert(SourcePair(newSource->getHandle(), newSource));
        _recentSource = newSource;
        return newSource->getHandle();
    }
    else
    {
        delete newSource;
        return AL_NONE;
    }
}

// public, static
ALuint AudioHandler::createSource(const std::string& filename)
{
    ALuint buffer = AudioHandler::getBuffer(filename);
    return AudioHandler::createSource(buffer);
}

// public, static
void AudioHandler::deleteSource(const ALuint sourceHandle)
{
    // Find the source in the map, and if found, delete it
    SourceMapIterator iterator = AudioHandler::_sourceMap.find(sourceHandle);

    if (AudioHandler::_sourceMap.end() != iterator)
    {
        if (iterator->second)
        {
            delete iterator->second;
        }
        _sourceMap.erase(iterator);
    }
}

// private, static
Source* AudioHandler::_getSource(const ALuint sourceHandle)
{
    // Short circuit the map lookup. See if handle matches the most recently looked up source
    if (_recentSource && sourceHandle == _recentSource->getHandle())
    {
        return _recentSource;
    }

    // Else, we have to look through the map
    SourceMapConstIterator iterator = _sourceMap.find(sourceHandle);

    if (_sourceMap.end() != iterator && iterator->second)
    {
        _recentSource = iterator->second;
        return iterator->second;
    }
    else
    {
        return NULL;
    }
}

// public, static
void AudioHandler::playSource(const ALuint sourceHandle)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->play();
    }

}

// public, static
void AudioHandler::stopSource(const ALuint sourceHandle)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->stop();
    }
}

// public, static
void AudioHandler::setSourcePosition(const ALuint sourceHandle, const ALfloat x, const ALfloat y, const ALfloat z)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setPosition(x, y, z);
    }
}

// public, static
void AudioHandler::setSourceGain(const ALuint sourceHandle, const ALfloat gain)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setGain(gain);
    }
}

// public, static
void AudioHandler::setSourceLoop(const ALuint sourceHandle, const ALint isLoop)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setLoop(isLoop);
    }
}

// public, static
void AudioHandler::setSourceVelocity(const ALuint sourceHandle, const ALfloat x, const ALfloat y, const ALfloat z)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setVelocity(x, y, z);
    }
}

// public, static
void AudioHandler::setSourceSpeed(const ALuint sourceHandle, ALfloat speed)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setVelocity( speed * source->getDirectionX(),
                             speed * source->getDirectionY(),
                             speed * source->getDirectionZ());
    }
}

// public, static
void AudioHandler::setSourceDirection(const ALuint sourceHandle, ALfloat x, ALfloat y, ALfloat z)
{
    Source *source = AudioHandler::_getSource(sourceHandle);

    if (source)
    {
        source->setDirection(x, y, z);
    }
}

// public, static
void AudioHandler::setSourceDirection(const ALuint sourceHandle, ALfloat angleInDegrees)
{
    ALfloat angleInRadians = angleInDegrees * (acos(-1.0) / 180.0); // PI = acos(-1)

    AudioHandler::setSourceDirection( sourceHandle, 
                                      sin(angleInRadians),
                                      0.0,
                                      cos(angleInRadians));
}


//bool StringComparison::operator() (const char *lhs, const char *rhs) const
//{
//    if ((!lhs && !rhs) || (lhs && !rhs))
//    {
//        return false;
//    }
//    else if ((!lhs && rhs) || strcmp(lhs, rhs) < 0)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

