#include <cvrKernel/FileHandler.h>
#include <cvrKernel/SceneManager.h>

#include <osgDB/ReadFile>

#include <iostream>
#include <algorithm>

using namespace cvr;

FileHandler * FileHandler::_myPtr = NULL;

FileHandler::FileHandler()
{
}

FileHandler::~FileHandler()
{
}

FileHandler * FileHandler::instance()
{
    if(!_myPtr)
    {
        _myPtr = new FileHandler();
    }
    return _myPtr;
}

bool FileHandler::loadFile(std::string file)
{
    std::string filext;
    // get file ext
    size_t pos = file.find_last_of("/\\");
    if(pos != std::string::npos && pos + 1 < file.size())
    {
        pos++;
        filext = file.substr(pos,file.size() - pos);
    }
    else
    {
        filext = file;
    }

    pos = filext.find_last_of(".");
    if(pos != std::string::npos && pos + 1 < filext.size())
    {
        pos++;
        filext = filext.substr(pos,filext.size() - pos);
    }
    else
    {
        filext = "";
    }

    std::transform(filext.begin(),filext.end(),filext.begin(),::tolower);

    if(_extMap.find(filext) != _extMap.end())
    {
        if(_extMap[filext]->loadFile(file))
        {
            return true;
        }
    }

    // if all else fails

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(file);

    if(loadedModel)
    {
        SceneManager::instance()->getObjectsRoot()->addChild(loadedModel);
        return true;
    }

    std::cerr << "Unable to load file " << file << std::endl;

    return false;
}

void FileHandler::registerExt(std::string ext, FileLoadCallback * flc)
{
    std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
    if(flc && _extMap.find(ext) == _extMap.end())
    {
        _extMap[ext] = flc;
        //std::cerr << "FileHandler: reg ext: " << ext << std::endl;
    }
}

void FileHandler::unregisterExt(std::string ext, FileLoadCallback * flc)
{
    std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
    std::map<std::string,FileLoadCallback*>::iterator it = _extMap.find(ext);
    if(it != _extMap.end() && _extMap[ext] == flc)
    {
        _extMap.erase(it);
        //std::cerr << "FileHandler: unreg ext: " << ext << std::endl;
    }
}

FileLoadCallback::FileLoadCallback(std::string exts)
{
    if(exts.empty())
    {
        return;
    }

    size_t pos = 0;

    do
    {
        size_t nextpos = exts.find_first_of(",",pos);
        if(nextpos == std::string::npos)
        {
            std::string ext = exts.substr(pos,exts.size() - pos);
            std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
            FileHandler::instance()->registerExt(ext,this);
            _exts.push_back(ext);
            pos = std::string::npos;
        }
        else
        {
            if(nextpos > pos)
            {
                std::string ext = exts.substr(pos,nextpos - pos);
                std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
                FileHandler::instance()->registerExt(ext,this);
                _exts.push_back(ext);
                pos = nextpos + 1;
                if(pos == exts.size())
                {
                    pos = std::string::npos;
                }
            }
            else
            {
                pos++;
                if(pos == exts.size())
                {
                    pos = std::string::npos;
                }
            }
        }

    }
    while(pos != std::string::npos);
}

FileLoadCallback::~FileLoadCallback()
{
    for(int i = 0; i < _exts.size(); i++)
    {
        FileHandler::instance()->unregisterExt(_exts[i],this);
    }
}
