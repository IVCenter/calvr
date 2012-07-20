#include <cvrConfig/XMLReader.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/CalVR.h>

#include <iostream>
#include <sstream>

#include <mxml.h>
#include <cstdio>
#include <stack>
#include <algorithm>

#include <mxml.h>

using namespace cvr;

XMLReader::XMLReader() : ConfigFileReader()
{
}

XMLReader::~XMLReader()
{
    for(int i = 0; i < _configRootList.size(); i++)
    {
        mxmlDelete(_configRootList[i]);
    }
    _configRootList.clear();
}

bool XMLReader::loadFile(std::string file, bool givePriority)
{

    FILE *fp;
    mxml_node_t * tree;

    std::string cfile = ConfigManager::getConfigDirectory() + "/" + file;

    if(_debugOutput)
    {
	std::cerr << "Loading config file: " << file << std::endl;
    }

    fp = fopen(cfile.c_str(),"r");
    if(fp == NULL)
    {
        fp = fopen(file.c_str(),"r");
        if(fp == NULL)
        {
            std::cerr << "Unable to open file: " << file << std::endl;
            return false;
        }
    }
    tree = mxmlLoadFile(NULL,fp,MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        std::cerr << "Unable to parse XML file: " << file << std::endl;
        return false;
    }

    // take care of global/local/COCONFIG/INCLUDE blocks

    mxml_node_t * xmlNode = tree;

    while((xmlNode = mxmlFindElement(tree,tree,"GLOBAL",NULL,NULL,MXML_DESCEND)))
    {
        if(xmlNode->parent)
        {
            mxml_node_t * tempnode = xmlNode->child;
            while(tempnode)
            {
                mxmlRemove(tempnode);
                mxmlAdd(xmlNode->parent,MXML_ADD_AFTER,NULL,tempnode);
                tempnode = xmlNode->child;
            }
            mxmlDelete(xmlNode);
        }
        else
        {
            std::cerr << "GLOBAL block with no parent." << std::endl;
            mxmlDelete(tree);
            return false;
        }
    }

    while((xmlNode = mxmlFindElement(tree,tree,"LOCAL",NULL,NULL,MXML_DESCEND)))
    {
        if(xmlNode->parent)
        {
            std::string blockHost;
            const char * attr = mxmlElementGetAttr(xmlNode,"host");

            std::string myHost = CalVR::instance()->getHostName();

            if(attr && !myHost.empty() && attr[0] != '\0')
            {
                blockHost = attr;
                bool hostfound = false;
                size_t startpos = 0;

                while((startpos = blockHost.find(myHost,startpos))
                        != std::string::npos)
                {
                    if((startpos == 0 || blockHost[startpos - 1] == ',')
                            && (startpos + myHost.length() == blockHost.length()
                                    || blockHost[startpos + myHost.length()]
                                            == ','))
                    {
                        hostfound = true;
                        break;
                    }
                    else
                    {
                        startpos++;
                    }
                }

                if(hostfound)
                {
                    mxml_node_t * tempnode = xmlNode->child;
                    while(tempnode)
                    {
                        mxmlRemove(tempnode);
                        mxmlAdd(xmlNode->parent,MXML_ADD_AFTER,NULL,tempnode);
                        tempnode = xmlNode->child;
                    }
                }
            }
            mxmlDelete(xmlNode);
        }
        else
        {
            std::cerr << "LOCAL block with no parent." << std::endl;
            mxmlDelete(tree);
            return false;
        }
    }

    _configRootList.push_back(tree);

    while((xmlNode = mxmlFindElement(tree,tree,"INCLUDE",NULL,NULL,MXML_DESCEND)))
    {
        if(xmlNode->parent)
        {
            mxml_node_t * tempnode = xmlNode->child;
            while(tempnode)
            {
                if(tempnode->type == MXML_TEXT)
                {
                    if(!loadFile(tempnode->value.text.string))
                    {
                        return false;
                    }
                    break;
                }
                tempnode = tempnode->next;
            }
            mxmlDelete(xmlNode);
        }
        else
        {
            std::cerr << "INCLUDE block with no parent." << std::endl;
            return false;
        }
    }

    return true;
}

std::string XMLReader::getEntry(std::string path, std::string def,
        bool * found)
{
    return getEntry("value",path,def,found);
}

std::string XMLReader::getEntry(std::string attribute, std::string path,
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

    for(int i = 0; i < _configRootList.size(); i++)
    {
        std::string pathFrag;
        std::string pathRemainder = path;
        mxml_node_t * xmlNode = _configRootList[i]->child;

        size_t location = pathRemainder.find_first_of('.');
        if(location == std::string::npos)
        {
            pathFrag = pathRemainder;
            pathRemainder = "";
        }
        else
        {
            pathFrag = pathRemainder.substr(0,location);
            if(location + 1 < pathRemainder.size())
            {
                pathRemainder = pathRemainder.substr(location + 1);
            }
            else
            {
                pathRemainder = "";
            }
        }

        std::stack<std::pair<mxml_node_t *,std::string> > parentStack;

        do
        {
            if(!parentStack.empty())
            {
                xmlNode = parentStack.top().first->next;
                if(!pathRemainder.empty())
                {
                    pathRemainder = pathFrag + "." + pathRemainder;
                }
                else
                {
                    pathRemainder = pathFrag;
                }
                pathFrag = parentStack.top().second;
                parentStack.pop();
            }
            while(xmlNode)
            {
                if(xmlNode->type != MXML_ELEMENT)
                {
                    xmlNode = xmlNode->next;
                    continue;
                }
                std::string nodeName = xmlNode->value.element.name;
                const char * nameAtt = mxmlElementGetAttr(xmlNode,"name");
                std::string suffix = nameAtt ? nameAtt : "";

                location = pathFrag.find_first_of(':');
                if((location == std::string::npos && pathFrag == nodeName)
                        || (location != std::string::npos
                                && pathFrag == nodeName + ":" + suffix))
                {
                    if(pathRemainder.empty())
                    {
                        const char * attr = mxmlElementGetAttr(xmlNode,
                                attribute.c_str());
                        if(attr)
                        {
                            if(found)
                            {
                                *found = true;
                            }
                            /*if(_debugOutput)
                            {
                                std::cerr << "Path: " << path << " Attr: "
                                        << attribute << " value: " << attr
                                        << std::endl;
                            }*/
                            return attr;
                        }
                        else
                        {
                            xmlNode = xmlNode->next;
                        }
                    }
                    else
                    {
                        parentStack.push(
                                std::pair<mxml_node_t *,std::string>(xmlNode,
                                        pathFrag));
                        location = pathRemainder.find_first_of('.');
                        if(location == std::string::npos)
                        {
                            pathFrag = pathRemainder;
                            pathRemainder = "";
                        }
                        else
                        {
                            pathFrag = pathRemainder.substr(0,location);
                            if(location + 1 < pathRemainder.size())
                            {
                                pathRemainder = pathRemainder.substr(
                                        location + 1);
                            }
                            else
                            {
                                pathRemainder = "";
                            }
                        }
                        xmlNode = xmlNode->child;
                    }
                }
                else
                {
                    xmlNode = xmlNode->next;
                }
            }
        }
        while(!parentStack.empty());
    }
    if(found)
    {
        *found = false;
    }
    /*if(_debugOutput)
    {
        std::cerr << "Path: " << path << " Attr: " << attribute << " value: "
                << def << " (default)" << std::endl;
    }*/
    return def;
}

std::string XMLReader::getEntryConcat(std::string attribute, std::string path,
                char separator, std::string def, bool * found)
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

    for(int i = 0; i < _configRootList.size(); i++)
    {
        std::string pathFrag;
        std::string pathRemainder = path;
        mxml_node_t * xmlNode = _configRootList[i]->child;

        size_t location = pathRemainder.find_first_of('.');
        if(location == std::string::npos)
        {
            pathFrag = pathRemainder;
            pathRemainder = "";
        }
        else
        {
            pathFrag = pathRemainder.substr(0,location);
            if(location + 1 < pathRemainder.size())
            {
                pathRemainder = pathRemainder.substr(location + 1);
            }
            else
            {
                pathRemainder = "";
            }
        }

        std::stack<std::pair<mxml_node_t *,std::string> > parentStack;

        do
        {
            if(!parentStack.empty())
            {
                xmlNode = parentStack.top().first->next;
                if(!pathRemainder.empty())
                {
                    pathRemainder = pathFrag + "." + pathRemainder;
                }
                else
                {
                    pathRemainder = pathFrag;
                }
                pathFrag = parentStack.top().second;
                parentStack.pop();
            }
            while(xmlNode)
            {
                if(xmlNode->type != MXML_ELEMENT)
                {
                    xmlNode = xmlNode->next;
                    continue;
                }
                std::string nodeName = xmlNode->value.element.name;
                const char * nameAtt = mxmlElementGetAttr(xmlNode,"name");
                std::string suffix = nameAtt ? nameAtt : "";

                location = pathFrag.find_first_of(':');
                if((location == std::string::npos && pathFrag == nodeName)
                        || (location != std::string::npos
                                && pathFrag == nodeName + ":" + suffix))
                {
                    if(pathRemainder.empty())
                    {
                        const char * attr = mxmlElementGetAttr(xmlNode,
                                attribute.c_str());
                        if(attr)
                        {
			    if(!wasFound)
			    {
				result = attr;
				wasFound = true;
			    }
			    else
			    {
				result = result + separator + attr;
			    }
                        }
                        xmlNode = xmlNode->next;
                    }
                    else
                    {
                        parentStack.push(
                                std::pair<mxml_node_t *,std::string>(xmlNode,
                                        pathFrag));
                        location = pathRemainder.find_first_of('.');
                        if(location == std::string::npos)
                        {
                            pathFrag = pathRemainder;
                            pathRemainder = "";
                        }
                        else
                        {
                            pathFrag = pathRemainder.substr(0,location);
                            if(location + 1 < pathRemainder.size())
                            {
                                pathRemainder = pathRemainder.substr(
                                        location + 1);
                            }
                            else
                            {
                                pathRemainder = "";
                            }
                        }
                        xmlNode = xmlNode->child;
                    }
                }
                else
                {
                    xmlNode = xmlNode->next;
                }
            }
        }
        while(!parentStack.empty());
    }

    if(found)
    {
        *found = wasFound;
    }

    if(!wasFound)
    {
	/*if(_debugOutput)
	{
	    std::cerr << "Path: " << path << " Attr: " << attribute << " value: "
		<< def << " (default)" << std::endl;
	}*/
	return def;
    }
    else
    {
	/*if(_debugOutput)
	{
	    std::cerr << "Path: " << path << " Attr: " << attribute << " value: "
		<< result << std::endl;
	}*/
	return result;
    }
}

float XMLReader::getFloat(std::string path, float def, bool * found)
{
    return getFloat("value",path,def,found);
}

float XMLReader::getFloat(std::string attribute, std::string path,
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

double XMLReader::getDouble(std::string path, double def, bool * found)
{
    return getDouble("value",path,def,found);
}

double XMLReader::getDouble(std::string attribute, std::string path,
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

int XMLReader::getInt(std::string path, int def, bool * found)
{
    return getInt("value",path,def,found);
}

int XMLReader::getInt(std::string attribute, std::string path, int def,
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

bool XMLReader::getBool(std::string path, bool def, bool * found)
{
    return getBool("value",path,def,found);
}

bool XMLReader::getBool(std::string attribute, std::string path, bool def,
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

osg::Vec3 XMLReader::getVec3(std::string path, osg::Vec3 def, bool * found)
{
    return getVec3("x","y","z",path,def,found);
}

osg::Vec3 XMLReader::getVec3(std::string attributeX, std::string attributeY,
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

osg::Vec4 XMLReader::getVec4(std::string path, osg::Vec4 def, bool * found)
{
    return getVec4("x","y","z","w",path,def,found);
}

osg::Vec4 XMLReader::getVec4(std::string attributeX, std::string attributeY,
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

osg::Vec3d XMLReader::getVec3d(std::string path, osg::Vec3d def,
        bool * found)
{
    return getVec3d("x","y","z",path,def,found);
}

osg::Vec3d XMLReader::getVec3d(std::string attributeX,
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

osg::Vec4d XMLReader::getVec4d(std::string path, osg::Vec4d def,
        bool * found)
{
    return getVec4d("x","y","z","w",path,def,found);
}

osg::Vec4d XMLReader::getVec4d(std::string attributeX,
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

osg::Vec4 XMLReader::getColor(std::string path, osg::Vec4 def, bool * found)
{
    return getVec4("r","g","b","a",path,def,found);
}

void XMLReader::getChildren(std::string path,
        std::vector<std::string> & destList)
{
    if(path.empty())
    {
        return;
    }

    for(int i = 0; i < _configRootList.size(); i++)
    {
        std::string pathFrag;
        std::string pathRemainder = path;
        mxml_node_t * xmlNode = _configRootList[i]->child;

        size_t location = pathRemainder.find_first_of('.');
        if(location == std::string::npos)
        {
            pathFrag = pathRemainder;
            pathRemainder = "";
        }
        else
        {
            pathFrag = pathRemainder.substr(0,location);
            if(location + 1 < pathRemainder.size())
            {
                pathRemainder = pathRemainder.substr(location + 1);
            }
            else
            {
                pathRemainder = "";
            }
        }

        std::stack<std::pair<mxml_node_t *,std::string> > parentStack;

        do
        {
            if(!parentStack.empty())
            {
                xmlNode = parentStack.top().first->next;
                if(!pathRemainder.empty())
                {
                    pathRemainder = pathFrag + "." + pathRemainder;
                }
                else
                {
                    pathRemainder = pathFrag;
                }
                pathFrag = parentStack.top().second;
                parentStack.pop();
            }
            while(xmlNode)
            {
                if(xmlNode->type != MXML_ELEMENT)
                {
                    xmlNode = xmlNode->next;
                    continue;
                }
                std::string nodeName = xmlNode->value.element.name;
                const char * nameAtt = mxmlElementGetAttr(xmlNode,"name");
                std::string suffix = nameAtt ? nameAtt : "";

                //std::cerr << "Looking at node: " << nodeName << " with suffix " << suffix << std::endl;

                location = pathFrag.find_first_of(':');
                if((location == std::string::npos && pathFrag == nodeName)
                        || (location != std::string::npos
                                && pathFrag == nodeName + ":" + suffix))
                {
                    //std::cerr << "Found Fragment." << std::endl;
                    if(pathRemainder.empty())
                    {
                        if(xmlNode->child)
                        {
                            mxml_node_t * cnode = xmlNode->child;
                            while(cnode)
                            {
                                if(cnode->type != MXML_ELEMENT)
                                {
                                    cnode = cnode->next;
                                    continue;
                                }
                                // ignore comment tags
                                if(strncmp(cnode->value.element.name,"!--",3))
                                {
                                    destList.push_back(
                                            cnode->value.element.name);
                                }
                                cnode = cnode->next;
                            }
                        }
                        xmlNode = xmlNode->next;
                    }
                    else
                    {
                        parentStack.push(
                                std::pair<mxml_node_t *,std::string>(xmlNode,
                                        pathFrag));
                        location = pathRemainder.find_first_of('.');
                        if(location == std::string::npos)
                        {
                            pathFrag = pathRemainder;
                            pathRemainder = "";
                        }
                        else
                        {
                            pathFrag = pathRemainder.substr(0,location);
                            if(location + 1 < pathRemainder.size())
                            {
                                pathRemainder = pathRemainder.substr(
                                        location + 1);
                            }
                            else
                            {
                                pathRemainder = "";
                            }
                        }
                        //std::cerr << "Looking for fragment: " << pathFrag << std::endl;
                        //std::cerr << "with remainder: " << pathRemainder << std::endl;
                        xmlNode = xmlNode->child;
                    }
                }
                else
                {
                    xmlNode = xmlNode->next;
                }
            }
        }
        while(!parentStack.empty());
    }
    return;
}
