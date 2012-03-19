/**
 * @file OASFileHandler.h
 * @author Shreenidhi Chowkwale
 */

#ifndef _OAS_FILE_HANDLER_H_
#define _OAS_FILE_HANDLER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
#include <AL/alut.h>
#include <mxml.h>
#include "OASLogger.h"

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
     * Static method to sets up FileHandler with a root cache directory. 
     * All FileHandler instances will share this cache directory path.
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
    bool doesFileExist(const std::string& filePath);

    /**
     * @brief Writes or Overwrites data to file
     * @param filename Name of file to create/overwrite
     * @param data Data to write
     * @param size Size of data to write
     * @retval True Write was successful
     * @retval False Write failed
     */
    bool writeFile(const std::string& filename, const char *data, unsigned int size);

    /**
     * @brief Reads data from file, allocates space for it on the heap
     * @param filename Name of file to read
     * @param fileSize Will contain the size in bytes of the file that is read.
     * @return Pointer to allocated data. The caller needs to free it when finished
     * @retval NULL on failure
     */
    void* readFile(const std::string& filename, int& fileSize);

    /**
     * @brief Loads an XML file for parsing
     * @param filename Name of XML file to open
     * @param root Root name of the XML file
     * @retval True File was opened successfuly
     * @retval False Error occured loading the file
     */
    bool loadXML(const std::string& filename, const std::string& root);

     /**
      * @brief Releases any loaded XML file
      */
    void unloadXML();

    /**
     * @brief Looks up an element in the currently loaded XML file
     * @param name Element name or NULL for any
     * @param attr Attribute name, or NULL for none
     * @param value Attribute value, or NULL for any
     * @param output If an element was found, then this will be the resulting string.
     * @retval True Element was found
     * @retval False Element not found
     */
    bool findXML(const char *name, const char *attr, const char *value, std::string& output);

    /**
     * @brief 
     */
    FileHandler();
    ~FileHandler();

private:
    static std::string      _cacheDirectoryPath;

    FILE *_file;
    mxml_node_t* _tree;
    mxml_node_t* _root;
};

}

#endif

