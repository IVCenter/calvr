/**
 * @file OASMessage.cpp
 * @author Shreenidhi Chowkwale
 *
 */

#include "OASMessage.h"

using namespace oas;

Message::Message()
{
    _init();
}

Message::Message(MessageType mtype)
{
    _init();
    _mtype = mtype;
}

Message::Message(const Message& other)
{
    _mtype = other.getMessageType();
    _handle = other.getHandle();
    _iParam = other.getIntegerParam();
    _needsResponse = other.needsResponse();
    _errorType = other.getError();
    _filename = other.getFilename();
    _originalString = other.getOriginalString();

    for (int i = 0; i < MAX_NUMBER_FLOAT_PARAM; i++)
    {
        _fParams[i] = other.getFloatParam(i);
    }
}

Message::~Message()
{

}

// private
void Message::_init()
{
    _mtype = Message::MT_UNKNOWN;
    _handle = AL_NONE;
    _iParam = 0;
    gettimeofday(&start, NULL);

    for (int i = 0; i < MAX_NUMBER_FLOAT_PARAM; i++)
    {
        _fParams[i] = 0.0;
    }

    _needsResponse = false;
    _errorType = MERROR_NONE;
}

// private
bool Message::_parseStringGetString(char* string, char*& pEnd, char*& result)
{
    char *pChar;

    pChar = strtok(string, " ,;\n\r\t");
    if (!pChar)
    {
        _errorType = MERROR_INCOMPLETE_MESSAGE;
        return false;
    }
    _errorType = MERROR_NONE;
    pEnd = pChar + strlen(pChar);
    result = pChar;

    return true;
}

//private
bool Message::_parseStringGetLong(char *string, char*& pEnd, long& result)
{
    char *pChar, *endp;
    long res;
    
    pChar = strtok(string, " ,;\n\r\t");
    if (!pChar)
    {
        _errorType = MERROR_INCOMPLETE_MESSAGE;
        return false;
    }

    pEnd = pChar + strlen(pChar);

    res = strtol(pChar, &endp, 10);
    if (*endp)
    {
        _errorType = MERROR_BAD_FORMAT;
        return false;
    }
    
    _errorType = MERROR_NONE;
    result = res;

    return true;
}

// private
bool Message::_parseStringGetFloat(char *string, char*& pEnd, ALfloat& result)
{
    char *pChar, *endp;
    ALfloat res;

    pChar = strtok(string, " ,;\n\r\t");
    if (!pChar)
    {
        _errorType = MERROR_INCOMPLETE_MESSAGE;
        return false;
    }

    pEnd = pChar + strlen(pChar);

    res = (ALfloat) strtod(pChar, &endp);
    
    if (*endp)
    {
        _errorType = MERROR_BAD_FORMAT;
        return false;
    }

    _errorType = MERROR_NONE;
    result = res;

    return true;
}

// private
bool Message::_validateParseAmounts(char *startBuf, char *pEnd, const int maxParseAmount, int& totalParsed)
{
    int amountParsed = pEnd - startBuf;

    if ( maxParseAmount < amountParsed)
    {
        _errorType = MERROR_INCOMPLETE_MESSAGE;
        return false;
    }
    else
    {
        totalParsed = amountParsed;
        return true;
    }
}

// private
bool Message::_parseHandleParameter(char *startBuf, char*& pEnd, const int maxParseAmount, int& totalParsed)
{
	long longVal;
	
	if (_parseStringGetLong(NULL, pEnd, longVal) 
        && _validateParseAmounts(startBuf, pEnd, maxParseAmount, totalParsed))
    {
        _handle = (ALuint) longVal;
    	return true;
    }
   
    return false;
}

// private
bool Message::_parseFilenameParameter(char *startBuf, char*& pEnd, const int maxParseAmount, int& totalParsed)
{
	char *pChar;
	
    if (_parseStringGetString(NULL, pEnd, pChar)
        && _validateParseAmounts(startBuf, pEnd, maxParseAmount, totalParsed))
    {
        if (pChar)
        {
            // Skip over any leading non-alphanumeric characters in the filename
            // i.e. converts "./directory/file" to "directory/file"
            while (*pChar && !isalnum(*pChar))
                pChar++;
        }
        _filename = pChar;
        return true;
    }

    return false;  
}

// private
bool Message::_parseIntegerParameter(char *startBuf, char*& pEnd, const int maxParseAmount, int& totalParsed)
{
	long longVal;
	
	if (_parseStringGetLong(NULL, pEnd, longVal)
        && _validateParseAmounts(startBuf, pEnd, maxParseAmount, totalParsed))
    {
        _iParam = longVal;
    	return true;
    }

    return false;
}

// private
bool Message::_parseFloatParameter(char *startBuf, char*& pEnd, const int maxParseAmount, int& totalParsed, unsigned int index)
{
    if (index < 0 || index > MAX_NUMBER_FLOAT_PARAM)
    {
        _errorType = MERROR_BAD_FORMAT;
        return false;
    }

    ALfloat floatVal;

    if (_parseStringGetFloat(NULL, pEnd, floatVal)
        && _validateParseAmounts(startBuf, pEnd, maxParseAmount, totalParsed))
    {
        _fParams[index] = floatVal;
        return true;
    }

    return false;
}

Message::MessageError Message::parseString(char*& messageString, const int maxParseAmount, int& totalParsed)
{
    //    
    // All cases:
    //          endMessageString needs to point to just after the end of the current message
    //          endMessageString = startMessageString + totalParsed
    //          return appropriate error message
    // Case 1: messageString does not contain a complete message
    //          need to make a copy of the messagestring so that original buffer is not mangled
    //          need to halt parsing if maxParseAmount is reached
    //          the next read must append to the message string, but the message must be parsed from the very beginning
    // Case 2: messageString contains valid message plus 0 or more bytes of extra stuff
    //          values in message should be set accordingly
    // Case 3: messageString does not contain valid message (e.g. parameters incorrect (and not incomplete) or message
    //			 type unknown)
    //          return appropriate error message
    //char *startBuf, char*& pEnd, int maxParseAmount, unsigned int& totalParsed)

    // Perform preliminary validation of the input string
    if (!messageString)
    {
    	_errorType = MERROR_BAD_FORMAT;
        return _errorType;
    }

    char tokenBuf[512];
    char *pType, *pEnd, *pChar;
    bool isSuccess = false;

    totalParsed = 0;
    _errorType = MERROR_NONE;

    // Skip any leading non-alphabetic characters
    while (*messageString && !isalpha(*messageString) && totalParsed < maxParseAmount)
    {
        *messageString = '\0';
        messageString++;
        totalParsed++;
    }

    // Check if the above loop skipped over the whole string
    if (!*messageString || totalParsed == maxParseAmount)
    {
        _errorType = MERROR_EMPTY_MESSAGE;
        return _errorType;
    }

    // Replace all newline/carriage returns with a blank space to fix FLTK browser text rendering
    for (pChar = messageString; *pChar; pChar++)
    {
        if (*pChar == '\n' || *pChar == '\r')
        {
            *pChar = ' ';
        }
    }

    //oas::Logger::logf("Parsing string \"%s\"", messageString);

    this->_originalString = std::string(messageString);

    // Copy the original string into a buffer
    strcpy(tokenBuf, messageString);
    
    // Get the first token: message type
    // Parse, and then make sure we aren't parsing more than we should be
    if (!_parseStringGetString(tokenBuf, pEnd, pType)
        || !_validateParseAmounts(tokenBuf, pEnd, maxParseAmount, totalParsed))
    { 
        // Fail immediately if errors occur at this point
        return _errorType;
    }
   

    // Parse the rest of the message based on the message type

    // GHDL
    if (0 == strcmp(pType, M_GET_HANDLE))
    {
        // Set message type
        _mtype = Message::MT_GHDL_FN;
        
        // We need to send a response after processing this message
        _needsResponse = true;

		// Parse token: the filename
        isSuccess = _parseFilenameParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
    }
    // RHDL
    else if (0 == strcmp(pType, M_RELEASE_HANDLE))
    {
        // Set message type
        _mtype = Message::MT_RHDL_HL;

		// Parse token: the handle 
        isSuccess = _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
    }
    // PTFI
    else if (0 == strcmp(pType, M_PREPARE_FILE_TRANSFER))
    {
        // Set message type
        _mtype = Message::MT_PTFI_FN_1I;

		// Parse tokens: the filename and the filesize
        isSuccess =    _parseFilenameParameter(tokenBuf, pEnd, maxParseAmount, totalParsed) 
                    && _parseIntegerParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
    }    
    // PLAY
    else if (0 == strcmp(pType, M_PLAY))
    {
        // Set message type
        _mtype = Message::MT_PLAY_HL;
        
        // Parse token: the handle
        isSuccess = _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
	}
	// STOP
    else if (0 == strcmp(pType, M_STOP))
    {
        // Set message type
        _mtype = Message::MT_STOP_HL;
        
        // Parse token: the handle
        isSuccess = _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
	}
    // SSPO
    else if (0 == strcmp(pType, M_SET_SOUND_POSITION))
    {
        // Set message type
        _mtype = Message::MT_SSPO_HL_3F;

        // Parse tokens: the handle, and 3 ALfloats
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
    }
    // SSVO
    else if (0 == strcmp(pType, M_SET_SOUND_GAIN))
    {
        // Set message type
        _mtype = Message::MT_SSVO_HL_1F;

        // Parse tokens: the handle, and 1 ALfloat
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);
    }
    // SSLP
    else if (0 == strcmp(pType, M_SET_SOUND_LOOP))
    {
        // Set message type
        _mtype = Message::MT_SSLP_HL_1I;

        // Parse tokens: the handle, and 1 integer
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseIntegerParameter(tokenBuf, pEnd, maxParseAmount, totalParsed);
    }
    // SSVE
    else if (0 == strcmp(pType, M_SET_SOUND_VELOCITY))
    {
        // Set message type to the simpler SSVE message version, with only 1 ALfloat parameter
        _mtype = Message::MT_SSVE_HL_1F;

        // Parse the minimum # of tokens: the handle, and 1 ALfloat
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);

        // If there's more parameters, then SSVE message is used with a vector
        if (isSuccess
            && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1))
        {
            isSuccess = _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
            // Update message type accordingly
            _mtype = Message::MT_SSVE_HL_3F;
        }
        // Else, parsing failed OR we're done parsing the simpler SSVE message version
    }
    // SSDI
    else if (0 == strcmp(pType, M_SET_SOUND_DIRECTION))
    {
        // Set message type to the simpler SSDI message version, with only 1 ALfloat parameter
        _mtype = Message::MT_SSDI_HL_1F;

        // Determine which type of SSDI message it is by parsing halfway through
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);

        // If there's more parameters, then SSDI message is used with a vector
        if (isSuccess
            && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1))
        {
            isSuccess = _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
            // Update message type accordingly
            _mtype = Message::MT_SSDI_HL_3F;
        }
        // Else, parsing failed OR we're done parsing the simpler SSDI message version
    }
    // SSDV
    else if (0 == strcmp(pType, M_SET_SOUND_DIRECTION_AND_GAIN))
    {
        // Set message type
        _mtype = Message::MT_SSDV_HL_1F_1F;

        // Parse tokens: the handle, and 2 ALfloats
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1);
    }
    // SSDR
    else if (0 == strcmp(pType, M_SET_SOUND_DIRECTION_RELATIVE))
    {
        // Set message type
        _mtype = Message::MT_SSDR_HL_1F;

        // Parse tokens: the handle, and 1 ALfloat
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);
    }
    // SSRV
    else if (0 == strcmp(pType, M_SET_SOUND_DIRECTION_AND_GAIN_RELATIVE))
    {
        // Set message type to SSRV directional
        _mtype = Message::MT_SSRV_HL_1F_1F;

        // Determine which type of SSRV message it is by parsing halfway through
        isSuccess =    _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1);

        // If there's more parameters, then SSRV used with vector params
        if (isSuccess
            && _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2))
        {
            isSuccess = _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 3);
            // Update the message type accordingly
            _mtype = Message::MT_SSRV_HL_3F_1F;
        }
        // Else, parsing failed or SSRV was directional and we're done
    }
    // SPIT
    else if (0 == strcmp(pType, M_SET_SOUND_PITCH))
    {
        // Set message type to SPIT
        _mtype = Message::MT_SPIT_HL_1F;

        // Parse tokens: The handle and the pitch factor
        isSuccess =     _parseHandleParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);
    }
    // WAVE
    else if (0 == strcmp(pType, M_GENERATE_SOUND_FROM_WAVEFORM))
    {
        // Set message type to WAVE
        _mtype = Message::MT_WAVE_1I_3F;

        // Parse tokens: Wave type, frequency, phase, and duration
        isSuccess =     _parseIntegerParameter(tokenBuf, pEnd, maxParseAmount, totalParsed)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
        _needsResponse = true;
    }
    // SLPO
    else if (0 == strcmp(pType, M_SET_LISTENER_POSITION))
    {
        // Set message type to SLPO
        _mtype = Message::MT_SLPO_3F;

        // Parse tokens: x, y, z of the listener's position
        isSuccess =     _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
    }
    // SLVE
    else if (0 == strcmp(pType, M_SET_LISTENER_VELOCITY))
    {
        // Set message type to SLVE
        _mtype = Message::MT_SLVE_3F;

        // Parse tokens: x, y, z of the listener's velocity
        isSuccess =     _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2);
    }
    // GAIN
    else if (0 == strcmp(pType, M_SET_LISTENER_GAIN))
    {
        // Set message type to GAIN
        _mtype = Message::MT_GAIN_1F;

        // Parse tokens: gain
        isSuccess =     _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0);
    }
    // SLOR
    else if (0 == strcmp(pType, M_SET_LISTENER_ORIENTATION))
    {
        // Set message type to SLOR
        _mtype = Message::MT_SLOR_3F_3F;

        // Parse tokens: x, y, z of "At" vector, and x, y, z of "Up" vector
        isSuccess =     _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 0)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 1)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 2)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 3)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 4)
                    &&  _parseFloatParameter(tokenBuf, pEnd, maxParseAmount, totalParsed, 5);
    }
    // SYNC
    else if (0 == strcmp(pType, M_SYNC))
    {
        // Set message type
        _mtype = Message::MT_SYNC;

        // We need to send a response after processing this message
        _needsResponse = true;

        isSuccess = true;
    }
    // QUIT
    else if (0 == strcmp(pType, M_QUIT))
    {
        // Set message type
        _mtype = Message::MT_QUIT;

        isSuccess = true;
    }
    // Unknown message type
    else
    {
        _mtype = Message::MT_UNKNOWN;
        _errorType = MERROR_UNKNOWN_MESSAGE_TYPE;
        isSuccess = false;
    }

    // If parsing was not successful, return the error that was encountered
    if (!isSuccess)
    {
        return _errorType;
    }

    // Parsing was successful. Advance the message string by the amount that was parsed
    messageString = messageString + totalParsed;

    return MERROR_NONE;
}

Message::MessageType Message::getMessageType() const
{
    return _mtype;
}

ALuint Message::getHandle() const
{
    return _handle;
}

void Message::setFilename(const std::string& filename)
{
    if (!filename.empty())
    {
        _filename = filename;
    }
}

const std::string& Message::getFilename() const
{
    return _filename;
}
        
void Message::setIntegerParam(long iParam)
{
    _iParam = iParam;   
}
        
long Message::getIntegerParam() const
{
    return _iParam;
}

void Message::setFloatParam(ALfloat value, unsigned int index)
{
    if (index >= 0 && index < MAX_NUMBER_FLOAT_PARAM)
    {
        _fParams[index] = value;
    }
}

ALfloat Message::getFloatParam(unsigned int index) const
{
    if (index >= 0 && index < MAX_NUMBER_FLOAT_PARAM)
    {
        return _fParams[index];
    }
    return 0.0;
}

bool Message::needsResponse() const
{
    return _needsResponse;
}

Message::MessageError Message::getError() const
{
    return _errorType;
}

const std::string& Message::getOriginalString() const
{
    return _originalString;
}
