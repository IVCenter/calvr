#include <cvrConfig/ConfigManager.h>
#include <cvrConfig/XMLReader.h>
#include <cvrKernel/CalVR.h>

#include <iostream>
#include <sstream>

#include <mxml.h>
#include <cstdio>
#include <stack>
#include <algorithm>

#include <mxml.h>

using namespace cvr;

std::vector<ConfigFileReader*> ConfigManager::_configFileList;
std::string ConfigManager::_configDir;
bool ConfigManager::_debugOutput = false;

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
    for(int i = 0; i < _configFileList.size(); i++)
    {
        delete _configFileList[i];
    }
    _configFileList.clear();
}

bool ConfigManager::init()
{
    const char * confDir = getenv("CALVR_CONFIG_DIR");
    if(confDir)
    {
        _configDir = confDir;
    }
    else
    {
        _configDir = CalVR::instance()->getConfigDir();
    }

    std::string file;
    const char * confFile = getenv("CALVR_CONFIG_FILE");
    if(confFile)
    {
        file = confFile;
    }
    else
    {
        std::cerr << "Warning: CALVR_CONFIG_FILE not set, using config.xml."
                << std::endl;
        file = "config.xml";
    }

    if(file.empty())
    {
        std::cerr << "Error: CALVR_CONFIG_FILE is empty." << std::endl;
        return false;
    }

    std::vector<std::string> fileList;
    size_t pos = 0;
    while((pos = file.find_first_of(':')) != std::string::npos)
    {
        if(pos)
        {
            fileList.push_back(file.substr(0,pos));
        }

        if(pos + 1 < file.size())
        {
            file = file.substr(pos + 1,file.size() - (pos + 1));
        }
        else
        {
            break;
        }
    }

    if(file.size())
    {
        fileList.push_back(file);
    }

    if(!fileList.size())
    {
        std::cerr << "Error: no valid config file in CALVR_CONFIG_FILE"
                << std::endl;
        return false;
    }

    for(int i = 0; i < fileList.size(); i++)
    {
        size_t pos = fileList[i].find_last_of('.');
        if(pos == std::string::npos || pos + 1 == fileList[i].size())
        {
            std::cerr
                    << "ConfigManager: Error: Unable to find extension for file: "
                    << fileList[i] << std::endl;
            return false;
        }
        std::string extension = fileList[i].substr(pos + 1,
                fileList[i].size() - (pos + 1));
        std::transform(extension.begin(),extension.end(),extension.begin(),
                ::tolower);

        ConfigFileReader * cfr = NULL;
        if(extension == "xml")
        {
            cfr = new XMLReader();
        }
        else
        {
            std::cerr
                    << "ConfigManager: Error: No reader could be identified for file: "
                    << fileList[i] << std::endl;
            return false;
        }

        if(cfr)
        {
            cfr->setDebugOutput(true);
            if(cfr->loadFile(fileList[i]))
            {
                _configFileList.push_back(cfr);
            }
            else
            {
                std::cerr << "ConfigManager: Error loading config files."
                        << std::endl;
                delete cfr;
                return false;
            }
            cfr->setDebugOutput(false);
        }
        else
        {
            std::cerr
                    << "ConfigManager: Error: ConfigFileReader pointer is NULL. file: "
                    << fileList[i] << std::endl;
            return false;
        }
    }

    _debugOutput = getBool("ConfigDebug",false);
    for(int i = 0; i < _configFileList.size(); i++)
    {
        _configFileList[i]->setDebugOutput(_debugOutput);
    }

    return true;
}

std::string ConfigManager::getEntry(std::string path, std::string def,
        bool * found)
{
    return getEntry("value",path,def,found);
}

std::string ConfigManager::getEntry(std::string attribute, std::string path,
        std::string def, bool * found)
{
    if(path.empty())
    {
        if(found)
        {
            *found = false;
        }
        return def;
    }

    bool wasFound = false;
    std::string result;

    for(int i = 0; i < _configFileList.size(); i++)
    {
        result = _configFileList[i]->getEntry(attribute,path,def,&wasFound);
        if(wasFound)
        {
            break;
        }
    }

    if(found)
    {
        *found = wasFound;
    }

    if(wasFound)
    {
        if(_debugOutput)
        {
            std::cerr << "Path: " << path << " Attr: " << attribute
                    << " value: " << result << std::endl;
        }
        return result;
    }
    else
    {
        if(_debugOutput)
        {
            std::cerr << "Path: " << path << " Attr: " << attribute
                    << " value: " << def << " (default)" << std::endl;
        }
        return def;
    }
}

std::string ConfigManager::getEntryConcat(std::string attribute,
        std::string path, char separator, std::string def, bool * found)
{
    if(path.empty())
    {
        if(found)
        {
            *found = false;
        }
        return def;
    }

    bool wasFound = false;
    std::string result;

    for(int i = 0; i < _configFileList.size(); i++)
    {
        bool tempFound = false;
        std::string tempResult;

        tempResult = _configFileList[i]->getEntryConcat(attribute,path,
                separator,def,&tempFound);
        if(tempFound)
        {
            if(!wasFound)
            {
                result = tempResult;
                wasFound = true;
            }
            else
            {
                result = result + separator + tempResult;
            }
        }
    }

    if(found)
    {
        *found = wasFound;
    }

    if(wasFound)
    {
        if(_debugOutput)
        {
            std::cerr << "Path: " << path << " Attr: " << attribute
                    << " value: " << result << std::endl;
        }
        return result;
    }
    else
    {
        if(_debugOutput)
        {
            std::cerr << "Path: " << path << " Attr: " << attribute
                    << " value: " << def << " (default)" << std::endl;
        }
        return def;
    }
}

float ConfigManager::getFloat(std::string path, float def, bool * found)
{
    return getFloat("value",path,def,found);
}

float ConfigManager::getFloat(std::string attribute, std::string path,
        float def, bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute,path,ss.str(),&hasEntry);
    if(hasEntry)
    {
        if(found)
        {
            *found = true;
        }
        return atof(result.c_str());
    }
    if(found)
    {
        *found = false;
    }
    return def;
}

double ConfigManager::getDouble(std::string path, double def, bool * found)
{
    return getDouble("value",path,def,found);
}

double ConfigManager::getDouble(std::string attribute, std::string path,
        double def, bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute,path,ss.str(),&hasEntry);
    if(hasEntry)
    {
        if(found)
        {
            *found = true;
        }
        return atof(result.c_str());
    }
    if(found)
    {
        *found = false;
    }
    return def;
}

int ConfigManager::getInt(std::string path, int def, bool * found)
{
    return getInt("value",path,def,found);
}

int ConfigManager::getInt(std::string attribute, std::string path, int def,
        bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute,path,ss.str(),&hasEntry);
    if(hasEntry)
    {
        if(found)
        {
            *found = true;
        }
        return atoi(result.c_str());
    }
    if(found)
    {
        *found = false;
    }
    return def;
}

bool ConfigManager::getBool(std::string path, bool def, bool * found)
{
    return getBool("value",path,def,found);
}

bool ConfigManager::getBool(std::string attribute, std::string path, bool def,
        bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute,path,ss.str(),&hasEntry);
    if(hasEntry)
    {
        if(found)
        {
            *found = true;
        }
        std::transform(result.begin(),result.end(),result.begin(),::tolower);
        if(result == "on" || result == "true")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if(found)
    {
        *found = false;
    }
    return def;
}

osg::Vec3 ConfigManager::getVec3(std::string path, osg::Vec3 def, bool * found)
{
    return getVec3("x","y","z",path,def,found);
}

osg::Vec3 ConfigManager::getVec3(std::string attributeX, std::string attributeY,
        std::string attributeZ, std::string path, osg::Vec3 def, bool * found)
{
    bool hasEntry = false;
    bool isFound;

    osg::Vec3 result;
    result.x() = getFloat(attributeX,path,def.x(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.y() = getFloat(attributeY,path,def.y(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.z() = getFloat(attributeZ,path,def.z(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }

    if(found)
    {
        *found = hasEntry;
    }
    return result;
}

osg::Vec4 ConfigManager::getVec4(std::string path, osg::Vec4 def, bool * found)
{
    return getVec4("x","y","z","w",path,def,found);
}

osg::Vec4 ConfigManager::getVec4(std::string attributeX, std::string attributeY,
        std::string attributeZ, std::string attributeW, std::string path,
        osg::Vec4 def, bool * found)
{
    bool hasEntry = false;
    bool isFound;

    osg::Vec4 result;
    result.x() = getFloat(attributeX,path,def.x(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.y() = getFloat(attributeY,path,def.y(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.z() = getFloat(attributeZ,path,def.z(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.w() = getFloat(attributeW,path,def.w(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }

    if(found)
    {
        *found = hasEntry;
    }
    return result;
}

osg::Vec3d ConfigManager::getVec3d(std::string path, osg::Vec3d def,
        bool * found)
{
    return getVec3d("x","y","z",path,def,found);
}

osg::Vec3d ConfigManager::getVec3d(std::string attributeX,
        std::string attributeY, std::string attributeZ, std::string path,
        osg::Vec3d def, bool * found)
{
    bool hasEntry = false;
    bool isFound;

    osg::Vec3d result;
    result.x() = getDouble(attributeX,path,def.x(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.y() = getDouble(attributeY,path,def.y(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.z() = getDouble(attributeZ,path,def.z(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }

    if(found)
    {
        *found = hasEntry;
    }
    return result;
}

osg::Vec4d ConfigManager::getVec4d(std::string path, osg::Vec4d def,
        bool * found)
{
    return getVec4d("x","y","z","w",path,def,found);
}

osg::Vec4d ConfigManager::getVec4d(std::string attributeX,
        std::string attributeY, std::string attributeZ, std::string attributeW,
        std::string path, osg::Vec4d def, bool * found)
{
    bool hasEntry = false;
    bool isFound;

    osg::Vec4d result;
    result.x() = getDouble(attributeX,path,def.x(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.y() = getDouble(attributeY,path,def.y(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.z() = getDouble(attributeZ,path,def.z(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }
    result.w() = getDouble(attributeW,path,def.w(),&isFound);
    if(isFound)
    {
        hasEntry = true;
    }

    if(found)
    {
        *found = hasEntry;
    }
    return result;
}

osg::Vec4 ConfigManager::getColor(std::string path, osg::Vec4 def, bool * found)
{
    return getVec4("r","g","b","a",path,def,found);
}

void ConfigManager::getChildren(std::string path,
        std::vector<std::string> & destList)
{
    if(path.empty())
    {
        return;
    }

    for(int i = 0; i < _configFileList.size(); i++)
    {
        _configFileList[i]->getChildren(path,destList);
    }

    return;
}
