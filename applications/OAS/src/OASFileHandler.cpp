/**
 * @file OASFileHandler.cpp
 * @author Shreenidhi Chowkwale
 *
 */

#include "OASFileHandler.h"

using namespace oas;

// static 
std::string FileHandler::_cacheDirectoryPath;

// static
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

void* FileHandler::readFile(const std::string& filename, int& fileSize)
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
        return NULL;
    }

    data = new char[fileInfo.st_size];

    std::ifstream fileIn(filePath.c_str(), std::ios::in | std::ios::binary);
    fileIn.read(data, fileInfo.st_size);

    if (fileIn.bad())
    {
        oas::Logger::errorf("FileHandler - Failed to read \"%s\" from disk.",
                            filePath.c_str());
        delete[] data;
        return NULL;
    }

    fileSize = fileInfo.st_size;
    return data;
}

bool FileHandler::loadXML(const std::string& filename, const std::string& root)
{
    // The Mini-XML library requires C-style file handling. 

    FILE *fp;

    // Open the file
    fp = fopen(filename.c_str(), "r");
    if (!fp)
    {
        oas::Logger::errorf("FileHandler - Unable to open XML file \"%s\".", filename.c_str());
        return false;
    }

    // Release any XML tree that may have already been loaded
    this->unloadXML();

    // Generate XML tree from file contents
    mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);

    if (!tree)
    {
        oas::Logger::errorf("FileHandler - Could not process \"%s\" as an XML file!", 
                            filename.c_str());
        fclose(fp);
        return false;
    }

    this->_tree = tree;

    this->_root = mxmlFindElement(this->_tree,
                                  this->_tree,
                                  root.c_str(),
                                  NULL,
                                  NULL,
                                  MXML_DESCEND_FIRST);

    if (!this->_root)
    {
        oas::Logger::errorf("FileHandler - Could not find \"%s\" as root of the XML file!",
                            root.c_str());
        fclose(fp);
        return false;
    }

    // Close the file, and return true for success
    fclose(fp);

    return true;
}

void FileHandler::unloadXML()
{
    if (this->_tree)
    {
        mxmlDelete(this->_tree);
        this->_tree = NULL;
    }
}

bool FileHandler::findXML(const char *name, const char *attr, const char *value, std::string& output)
{
    if (!this->_root)
    {
        oas::Logger::errorf("FileHandler - No valid XML file initialized!");
        return false;
    }

    mxml_node_t *node = mxmlFindElement(this->_root, 
                                        this->_root, 
                                        name, 
                                        attr, 
                                        value, 
                                        MXML_DESCEND_FIRST);


    if (node)
    {
        if (node->child && node->child->value.text.string)
            output = node->child->value.text.string;

        return true;
    }
    else
    {
        return false;
    }
}

FileHandler::FileHandler()
{
    this->_tree = NULL;
}

FileHandler::~FileHandler()
{

}

