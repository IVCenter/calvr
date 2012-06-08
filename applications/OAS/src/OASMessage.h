/**
 * @file    OASMessage.h
 * @author  Shreenidhi Chowkwale
 */

#ifndef _OAS_MESSAGE_H_
#define _OAS_MESSAGE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <AL/alut.h>
#include <sys/time.h>
#include "OASLogger.h"

namespace oas
{

// Message Type Strings
#define M_TEST									    "TEST"
#define M_GET_HANDLE            				    "GHDL"
#define M_RELEASE_HANDLE        				    "RHDL"
#define M_PREPARE_FILE_TRANSFER 				    "PTFI"
#define M_PLAY									    "PLAY"
#define M_STOP									    "STOP"
#define M_SET_SOUND_POSITION					    "SSPO"
#define M_SET_SOUND_GAIN						    "SSVO"
#define M_SET_SOUND_LOOP						    "SSLP"
#define M_SET_SOUND_VELOCITY					    "SSVE"
#define M_SET_SOUND_DIRECTION					    "SSDI"
#define M_SET_SOUND_DIRECTION_AND_GAIN 			    "SSDV"
#define M_SET_SOUND_DIRECTION_RELATIVE 	    	    "SSDR"
#define M_SET_SOUND_DIRECTION_AND_GAIN_RELATIVE     "SSRV"
#define M_SET_SOUND_PITCH                           "SPIT"
#define M_GENERATE_SOUND_FROM_WAVEFORM              "WAVE"
#define M_SET_LISTENER_POSITION                     "SLPO"
#define M_SET_LISTENER_VELOCITY                     "SLVE"
#define M_SET_LISTENER_GAIN                         "GAIN"
#define M_SET_LISTENER_ORIENTATION                  "SLOR"
#define M_SYNC									    "SYNC"
#define M_QUIT									    "QUIT"

// Maximum number of float parameters
#define MAX_NUMBER_FLOAT_PARAM    6

class Message
{
public:

    /** All of the different messages that can be received
     * Suffixes indicate parameters for each message:
     *      _FN - filename (char *)
     *      _HL - handle (long)
     *      _1I - one integer value (long)
     *      _1F - one floating point value (ALfloat)
     *      _3F - three floating point values (ALfloat x 3)
     */
    enum MessageType
    {
        MT_DATA = 0,        // Should never need this message type
        MT_TEST,            // Test sound
        MT_GHDL_FN,         // Get handle for a given filename
        MT_RHDL_HL,         // Release handle
        MT_PTFI_FN_1I,      // Prepare for file transmission, with the given filename & file size
        MT_PLAY_HL,         // Play handle
        MT_STOP_HL,         // Stop handle
        MT_SSPO_HL_3F,      // Set sound position
        MT_SSVO_HL_1F,      // Set sound gain
        MT_SSLP_HL_1I,      // Set sound loop
        MT_SSVE_HL_1F,      // Set sound velocity (with magnitude only, conforms to old server spec)
        MT_SSVE_HL_3F,      // Set sound velocity
        MT_SSDI_HL_1F,      // Set sound direction (in degrees, acting in x-y plane only)
        MT_SSDI_HL_3F,      // Set sound direction
        MT_SSDV_HL_1F_1F,   // Set sound direction and gain
        MT_SSDR_HL_1F,      // Set sound direction relative to listener
        MT_SSRV_HL_1F_1F,   // Set sound direction relative to listener and gain in one command
        MT_SSRV_HL_3F_1F,   // Set sound direction relative to listener and gain in one command
        MT_SPIT_HL_1F,      // Set pitch
        MT_WAVE_1I_3F,      // Generate a sound based on waveform
        MT_SLPO_3F,         // Set listener position
        MT_SLVE_3F,         // Set listener velocity
        MT_GAIN_1F,         // Set global (listener) gain
        MT_SLOR_3F_3F,      // Set listener orientation
        MT_SYNC,
        MT_QUIT,
        MT_UNKNOWN
    };

    /** Errors that can occur
     */
    enum MessageError
    {
        MERROR_NONE = 0,               // No error
        MERROR_INCOMPLETE_MESSAGE,     // Message incomplete
        MERROR_BAD_FORMAT,             // Message misformatted
        MERROR_EMPTY_MESSAGE,
        MERROR_UNKNOWN_MESSAGE_TYPE
    // Unknown message type
    };

    ALuint getHandle() const;
    MessageError parseString(char*& messageString, const int maxParseAmount, int& totalParsed);
    MessageType getMessageType() const;
    void setFilename(const std::string& filename);
    const std::string& getFilename() const;
    void setIntegerParam(long iParam);
    long getIntegerParam() const;
    void setFloatParam(ALfloat value, unsigned int index);
    ALfloat getFloatParam(unsigned int index) const;
    bool needsResponse() const;
    MessageError getError() const;
    const std::string& getOriginalString() const;

    Message();
    Message(MessageType mtype);
    Message(const Message& other);
    ~Message();

    struct timeval start;
    struct timeval added;
    struct timeval retrieved;
    struct timeval processed;

private:
    MessageType _mtype;
    ALuint _handle;
    std::string _filename;
    long _iParam;
    ALfloat _fParams[MAX_NUMBER_FLOAT_PARAM];
    bool _needsResponse;
    MessageError _errorType;
    std::string _originalString;

    void _init();

    bool _parseStringGetString(char *string, char*& pEnd, char*& result);
    bool _parseStringGetLong(char *string, char*& pEnd, long& result);
    bool _parseStringGetFloat(char *string, char*& pEnd, ALfloat& result);

    bool _validateParseAmounts(char *startBuf, char *pEnd, const int maxParseAmount,
                               int& totalParsed);

    bool _parseHandleParameter(char *startBuf, char*& pEnd, const int maxParseAmount,
                               int& totalParsed);
    bool _parseFilenameParameter(char *startBuf, char*& pEnd, const int maxParseAmount,
                                 int& totalParsed);
    bool _parseIntegerParameter(char *startBuf, char*& pEnd, const int maxParseAmount,
                                int& totalParsed);
    bool _parseFloatParameter(char *startBuf, char*& pEnd, const int maxParseAmount,
                              int& totalParsed, unsigned int index);
};

}

#endif

