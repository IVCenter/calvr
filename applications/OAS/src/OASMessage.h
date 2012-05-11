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
#define M_TEST									"TEST"
#define M_GET_HANDLE            				"GHDL"
#define M_RELEASE_HANDLE        				"RHDL"
#define M_PREPARE_FILE_TRANSFER 				"PTFI"
#define M_PLAY									"PLAY"
#define M_STOP									"STOP"
#define M_SET_SOUND_POSITION					"SSPO"
#define M_SET_SOUND_GAIN						"SSVO"
#define M_SET_SOUND_LOOP						"SSLP"
#define M_SET_SOUND_VELOCITY					"SSVE"
#define M_SET_SOUND_DIRECTION					"SSDI"
#define M_SET_SOUND_DIRECTION_AND_GAIN 			"SSDV"
#define M_SET_SOUND_DIRECTION_RELATIVE 			"SSDR"
#define M_SET_SOUND_DIRECTION_AND_GAIN_RELATIVE "SSRV"
#define M_SYNC									"SYNC"
#define M_QUIT									"QUIT"


// Maximum number of float parameters
#define MAX_NUMBER_FLOAT_PARAM    4

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
        MT_DATA = 0,       // Should never need this message type
        MT_TEST,           // Test sound
        MT_GHDL_FN,        // Get handle for given filename
        MT_RHDL_HL,        // Release handle
        MT_PTFI_FN_1I,     // Prepare for file transmission, filename & size
        MT_PLAY_HL,        // Play handle
        MT_STOP_HL,        // Stop handle
        MT_SSPO_HL_3F,
        MT_SSVO_HL_1F,
        MT_SSLP_HL_1I,
        MT_SSVE_HL_1F,
        MT_SSVE_HL_3F,
        MT_SSDI_HL_1F,
        MT_SSDI_HL_3F,
        MT_SSDV_HL_1F_1F,
        MT_SSDR_HL_1F,
        MT_SSRV_HL_1F_1F,
        MT_SSRV_HL_3F_1F,
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
        MERROR_UNKNOWN_MESSAGE_TYPE    // Unknown message type
    };

    ALuint getHandle() const;
    MessageError parseString(char*& messageString, unsigned int maxParseAmount, unsigned int& totalParsed);
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

    bool _parseStringGetString  (char *string, char*& pEnd, char*& result);
    bool _parseStringGetLong    (char *string, char*& pEnd, long& result);
    bool _parseStringGetFloat  (char *string, char*& pEnd, ALfloat& result);

    bool _validateParseAmounts  (char *startBuf, char *pEnd, int maxParseAmount,
                                  unsigned int& totalParsed);

    bool _parseHandleParameter  (char *startBuf, char*& pEnd, int maxParseAmount,
                                  unsigned int& totalParsed);
    bool _parseFilenameParameter(char *startBuf, char*& pEnd, int maxParseAmount,
                                  unsigned int& totalParsed);
    bool _parseIntegerParameter (char *startBuf, char*& pEnd, int maxParseAmount,
                                  unsigned int& totalParsed);
    bool _parseFloatParameter  (char *startBuf, char*& pEnd, int maxParseAmount,
                                  unsigned int& totalParsed, unsigned int index);
};

}

#endif

