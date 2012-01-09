/**
 * @file OASFileHandler.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_FILE_HANDLER_H_
#define _OAS_FILE_HANDLER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <AL/alut.h>
#include "OASLogger.h"
#include <limits.h>

/**
 * Although these max sizes are somewhat arbitrary, they should still prevent overflow.
 */
#define MAX_PATH_SIZE       1024
#define MAX_FILENAME_SIZE   1024

namespace oas
{

/**
 * Provides file I/O operations and manages caching on disk.
 */
class FileHandler
{
public:
    /**
     * @brief Sets up the File Handler with a root cache directory.
     * @param cacheDirectoryPath Path of the cache directory
     * @retval True Directory is valid and exists
     * @retval False Directory is invalid or does not exist
     */
    static bool initialize(const std::string &cacheDirectoryPath);

    /**
     * @brief Checks if 'cacheDirectory/filename' exists.
     * @param filename Name of file to check
     * @retval True File exists and can be opened
     * @retval False File does not exist or cannot be opened
     */
    static bool doesFileExist(const std::string& filePath);

    /**
     * @brief Writes or Overwrites data to file
     * @param filename Name of file to create/overwrite
     * @param data Data to write
     * @param size Size of data to write
     * @retval True Write was successful
     * @retval False Write failed
     */
    static bool writeFile(const std::string& filename, const char *data, unsigned int size);

    /**
     * @brief Reads data from file, creates OpenAL buffer
     * @param filename Name of file to read
     * @return Handle to OpenAL buffer
     * @retval AL_NONE on failure
     */
    static ALuint readFileIntoBuffer(const std::string& filename);

private:
    static std::string      _cacheDirectoryPath;
};

}

#endif

