/**
 * @file OASFileHandler.cpp
 * @author Shreenidhi Chowkwale
 *
 */

#include "OASFileHandler.h"

using namespace oas;

// Statics
std::string     FileHandler::_cacheDirectoryPath;

bool FileHandler::initialize(const std::string &cacheDirectoryPath)
{
    // struct that contains file information
    struct stat fileInfo;

    // Check if the path points to a valid directory
    if (0 != stat(cacheDirectoryPath.c_str(), &fileInfo))   // stat returns 0 on success
    {
        oas::Logger::error("FileHandler - Cache directory setup failed");
        oas::Logger::errorf("Intended cache directory was \"%s\"", cacheDirectoryPath.c_str());
        return false;
    }
    else if (!S_ISDIR(fileInfo.st_mode)) // macro in sys/stat.h to determine if directory
    {
        oas::Logger::errorf("FileHandler - Cache directory path \"%s\" does not point to a directory!",
                            cacheDirectoryPath.c_str());
        return false;
    }

    FileHandler::_cacheDirectoryPath = cacheDirectoryPath;

    Logger::logf("FileHandler initialized...cache directory set to \"%s\"",
                 cacheDirectoryPath.c_str());
    return true;
}

bool FileHandler::doesFileExist(const std::string& filePath)
{
    struct stat fileInfo;

    if (0 != stat(filePath.c_str(), &fileInfo)) // stat returns 0 on success
    {
        return false;
    }

    return true;
}

bool FileHandler::writeFile(const std::string& filename, const char *data, unsigned int size)
{
    std::string fileOutPath = FileHandler::_cacheDirectoryPath + "/" + filename;
    std::ofstream fileOut (fileOutPath.c_str(), std::ios::out | std::ios::binary);

    if (!fileOut.is_open() || fileOut.bad())
    {
        oas::Logger::errorf("FileHandler - Could not create file \"%s\"!",
                            fileOutPath.c_str());
        fileOut.close();
        return false;
    }

    fileOut.write(data, size);

    if (fileOut.bad())
    {
        oas::Logger::errorf("FileHandler - Writing to file \"%s\" failed. Data may be corrupted.",
                            fileOutPath.c_str());
        fileOut.close();
        return false;
    }

    fileOut.close();
    return true;
}
//void FileHandler::newFile(const std::string& filename)
//{
//    // If a file is already open, close it
//    if(FileHandler::_fileOut.is_open())
//    {
//        FileHandler::_fileOut.close();
//    }
//
//    // Set file path
//    FileHandler::_fileOutPath = FileHandler::_cacheDirectoryPath + "/" + filename;
//
//    // If the file already exists, clear it out first
//    if (FileHandler::doesFileExist(FileHandler::_fileOutPath))
//    {
//        FileHandler::_fileOut.open(FileHandler::_fileOutPath.c_str(),
//                                   std::ios::out | std::ios::trunc | std::ios::binary);
//        FileHandler::endFile();
//    }
//
//    FileHandler::_fileOut.open(FileHandler::_fileOutPath.c_str(),
//                               std::ios::out | std::ios::app | std::ios::binary);
//
//    if (!FileHandler::_fileOut.is_open())
//    {
//        oas::Logger::errorf("FileHandler - Could not open \"%s\" for writing!",
//                            FileHandler::_fileOutPath.c_str());
//        FileHandler::_fileOut.close();
//    }
//    else if (FileHandler::_fileOut.bad())
//    {
//        oas::Logger::errorf("FileHandler - Could not create file \"%s\"!",
//                            FileHandler::_fileOutPath.c_str());
//        FileHandler::_fileOut.close();
//    }
//}
//
//bool FileHandler::writeToFile(const char *data, unsigned int size)
//{
//	// If file isn't open or data is NULL, return false and do nothing.
//    if (!FileHandler::_fileOut.is_open() || !data)
//    {
//        return false;
//    }
//
//	FileHandler::_fileOut.write(data, size);
//
//	if (FileHandler::_fileOut.bad())
//	{
//        oas::Logger::errorf("FileHandler - Writing to file \"%s\" failed. Data may be corrupted.",
//                            FileHandler::_fileOutPath.c_str());
//        FileHandler::endFile();
//        return false;
//	}
//	return true;
//}
//
//void FileHandler::endFile()
//{
//    if (FileHandler::_fileOut.is_open())
//    {
//        FileHandler::_fileOut.close();
//    }
//}

ALuint FileHandler::readFileIntoBuffer(const std::string& filename)
{
    std::string filePath;
    struct stat fileInfo;
    char *data;

    // Create string to hold the cache directory plus the filename
    filePath = FileHandler::_cacheDirectoryPath + "/" + filename;

    // Locate the file and gather information using stat()
    if (0 != stat(filePath.c_str(), &fileInfo)) // stat returns 0 on success
    {
        oas::Logger::warnf("FileHandler - Could not access \"%s\"", filePath.c_str());
        return AL_NONE;
    }

    data = new char[fileInfo.st_size];

    std::ifstream fileIn(filePath.c_str(), std::ios::in | std::ios::binary);
    fileIn.read(data, fileInfo.st_size);

    if (fileIn.bad())
    {
        oas::Logger::errorf("FileHandler - Failed to read \"%s\" from disk.",
                            filePath.c_str());
        delete data;
        return AL_NONE;
    }
    ALuint retval =  alutCreateBufferFromFileImage(data, fileInfo.st_size);

    // If retval is invalid, print error message
    if (AL_NONE == retval)
    {
        oas::Logger::errorf("FileHandler - Failed to convert \"%s\" to an audio source. "
                            "Reason: \"%s\"",
                            filePath.c_str(),
                            alutGetErrorString(alutGetError()));
    }

    delete data;
    return retval;
}

