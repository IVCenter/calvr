#include <config/ConfigManager.h>
#include <kernel/CalVR.h>

#include<iostream>
#include <sstream>

#include <mxml.h>
#include <cstdio>
#include <stack>
#include <algorithm>

#ifdef WIN32
#include <Winsock2.h>
#include <stdlib.h>
#pragma comment(lib, "wsock32.lib")
#endif

using namespace cvr;

std::vector<mxml_node_t *> ConfigManager::_configRootList = std::vector<
        mxml_node_t *>();
bool ConfigManager::_debugOutput = false;

/*ConfigNode::ConfigNode(std::string n)
 {
 name = n;
 }

 ConfigNode::~ConfigNode()
 {
 for(std::vector<ConfigNode *>::iterator it = children.begin(); it != children.end(); it++)
 {
 delete *it;
 }
 }

 void ConfigNode::merge(ConfigNode * node, bool givePriority)
 {

 }*/

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
    for(int i = 0; i < _configRootList.size(); i++)
    {
        mxmlDelete(_configRootList[i]);
        _configRootList.clear();
    }
}

bool ConfigManager::loadFile(std::string file, bool givePriority)
{

    FILE *fp;
    mxml_node_t * tree;

    std::string cfile = _configDir + "/" + file;

    std::cerr << "Loading config file: " << file << std::endl;

    fp = fopen(cfile.c_str(), "r");
    if(fp == NULL)
    {
        fp = fopen(file.c_str(), "r");
        if(fp == NULL)
        {
            std::cerr << "Unable to open file: " << file << std::endl;
            return false;
        }
    }
    tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        std::cerr << "Unable to parse XML file: " << file << std::endl;
        return false;
    }

    // take care of global/local/COCONFIG/INCLUDE blocks

    mxml_node_t * xmlNode = tree;

    /*while((xmlNode = mxmlFindElement(tree, tree, "COCONFIG", NULL, NULL, MXML_DESCEND)))
     {
     if(xmlNode->parent)
     {
     mxml_node_t * tempnode = xmlNode->child;
     while(tempnode)
     {
     mxmlRemove(tempnode);
     mxmlAdd(xmlNode->parent, MXML_ADD_AFTER, NULL, tempnode);
     tempnode = xmlNode->child;
     }
     mxmlDelete(xmlNode);
     }
     else
     {
     std::cerr << "COCONFIG block with no parent." << std::endl;
     mxmlDelete(tree);
     return false;
     }
     }*/

    while((xmlNode = mxmlFindElement(tree, tree, "GLOBAL", NULL, NULL,
                                     MXML_DESCEND)))
    {
        if(xmlNode->parent)
        {
            mxml_node_t * tempnode = xmlNode->child;
            while(tempnode)
            {
                mxmlRemove(tempnode);
                mxmlAdd(xmlNode->parent, MXML_ADD_AFTER, NULL, tempnode);
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

    while((xmlNode = mxmlFindElement(tree, tree, "LOCAL", NULL, NULL,
                                     MXML_DESCEND)))
    {
        if(xmlNode->parent)
        {
            std::string blockHost;
            const char * attr = mxmlElementGetAttr(xmlNode, "host");

            //char hostname[51];
            //gethostname(hostname, 50);
            //std::string myHost = hostname;
	    std::string myHost = CalVR::instance()->getHostName();

            if(attr && !myHost.empty() && attr[0] != '\0')
            {
                blockHost = attr;
                bool hostfound = false;
                size_t startpos = 0;

                while((startpos = blockHost.find(myHost, startpos))
                        != std::string::npos)
                {
                    if((startpos == 0 || blockHost[startpos - 1] == ',')
                            && (startpos + myHost.length()
                                    == blockHost.length() || blockHost[startpos
                                    + myHost.length()] == ','))
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
                        mxmlAdd(xmlNode->parent, MXML_ADD_AFTER, NULL, tempnode);
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

    while((xmlNode = mxmlFindElement(tree, tree, "INCLUDE", NULL, NULL,
                                     MXML_DESCEND)))
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
                        mxmlDelete(tree);
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
            mxmlDelete(tree);
            return false;
        }
    }

    return true;

    /*std::stack<ConfigNode *> parentStack;

     ConfigNode * cnRoot = new ConfigNode(file);
     ConfigNode * tempNode;
     mxml_node_t * xmlNode = tree;

     parentStack.push(cnRoot);

     while(parentStack.size() > 0)
     {
     if(xmlNode->type == MXML_ELEMENT)
     {
     // add to parent
     tempNode = new ConfigNode(xmlNode->value.element.name);
     tempNode->node = xmlNode;
     parentStack.top()->children.push_back(tempNode);
     std::cerr << "Adding Node: " << xmlNode->value.element.name << std::endl;

     if(xmlNode->child != NULL)
     {
     parentStack.push(xmlNode);
     xmlNode = xmlNode->child;
     }
     }
     else
     {
     // ignore value nodes
     xmlNode = parentStack.top();
     parentStack.pop();
     }
     }*/
}

bool ConfigManager::init()
{
    char * confDir = getenv("CALVR_CONFIG_DIR");
    if(confDir)
    {
        _configDir = confDir;
    }
    else
    {
        _configDir = CalVR::instance()->getHomeDir();
        _configDir = _configDir + "/config";
    }

    //std::string file = "config.xml";
    std::string file;
    char * confFile = getenv("CALVR_CONFIG_FILE");
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
            fileList.push_back(file.substr(0, pos));
        }

        if(pos + 1 < file.size())
        {
            file = file.substr(pos + 1, file.size() - (pos + 1));
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
        if(!loadFile(fileList[i]))
        {
            for(int i = 0; i < _configRootList.size(); i++)
            {
                mxmlDelete(_configRootList[i]);
                _configRootList.clear();
            }
            return false;
        }
    }

    // merge COVER tags
    /*for(int i = 0; i < _configRootList.size(); i++)
     {
     mxml_node_t * xmlNode1, * xmlNode2;
     xmlNode1 = _configRootList[i]->child;
     while(xmlNode1)
     {
     if(xmlNode1->type == MXML_ELEMENT)
     {
     std::string nodeName = xmlNode1->value.element.name;
     if(nodeName == "COVER")
     {
     break;
     }
     }
     xmlNode1 = xmlNode1->next;

     }
     if(!xmlNode1)
     {
     continue;
     }
     xmlNode2 = xmlNode1->next;
     while(xmlNode2)
     {
     mxml_node_t * next = xmlNode2->next;
     if(xmlNode2->type == MXML_ELEMENT)
     {
     std::string nodeName = xmlNode2->value.element.name;
     if(nodeName == "COVER")
     {
     mxml_node_t * tempnode = xmlNode2->child;
     while(tempnode)
     {
     mxmlRemove(tempnode);
     mxmlAdd(xmlNode1, MXML_ADD_AFTER, NULL, tempnode);
     tempnode = xmlNode2->child;
     }
     mxmlDelete(xmlNode2);
     }
     }
     xmlNode2 = next;

     }
     }*/

    _debugOutput = getBool("ConfigDebug", false);

    return true;
}

std::string ConfigManager::getEntry(std::string path, std::string def,
                                    bool * found)
{
    return getEntry("value", path, def, found);
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
            pathFrag = pathRemainder.substr(0, location);
            if(location + 1 < pathRemainder.size())
            {
                pathRemainder = pathRemainder.substr(location + 1);
            }
            else
            {
                pathRemainder = "";
            }
        }

        //std::cerr << "Looking for fragment: " << pathFrag << std::endl;
        //std::cerr << "with remainder: " << pathRemainder << std::endl;

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
                    /*std::cerr << "Not elememnt node. type: " << xmlNode->type << std::endl;
                     if(xmlNode->type == MXML_OPAQUE)
                     {
                     std::cerr << "Opaque node value: " << xmlNode->value.opaque << std::endl;
                     }
                     if(xmlNode->type == MXML_TEXT)
                     {
                     std::cerr << "Text node value: " << xmlNode->value.text.string << std::endl;
                     }*/
                    xmlNode = xmlNode->next;
                    continue;
                }
                std::string nodeName = xmlNode->value.element.name;
                const char * nameAtt = mxmlElementGetAttr(xmlNode, "name");
                std::string suffix = nameAtt ? nameAtt : "";

                //std::cerr << "Looking at node: " << nodeName << " with suffix " << suffix << std::endl;

                location = pathFrag.find_first_of(':');
                if((location == std::string::npos && pathFrag == nodeName)
                        || (location != std::string::npos && pathFrag
                                == nodeName + ":" + suffix))
                {
                    //std::cerr << "Found Fragment." << std::endl;
                    if(pathRemainder.empty())
                    {
                        //found node
                        const char * attr =
                                mxmlElementGetAttr(xmlNode, attribute.c_str());
                        if(attr)
                        {
                            if(found)
                            {
                                *found = true;
                            }
                            if(_debugOutput)
                            {
                                std::cerr << "Path: " << path << " Attr: "
                                        << attribute << " value: " << attr
                                        << std::endl;
                            }
                            return attr;
                        }
                        else
                        {
                            /*if(found)
                             {
                             *found = false;
                             }
                             std::cerr << "Path: " << path << " Attr: " << attribute << " value: " << def << " (default)" << std::endl;
                             return def;*/
                            xmlNode = xmlNode->next;
                        }
                    }
                    else
                    {
                        parentStack.push(
                                         std::pair<mxml_node_t *,std::string>(
                                                                              xmlNode,
                                                                              pathFrag));
                        location = pathRemainder.find_first_of('.');
                        if(location == std::string::npos)
                        {
                            pathFrag = pathRemainder;
                            pathRemainder = "";
                        }
                        else
                        {
                            pathFrag = pathRemainder.substr(0, location);
                            if(location + 1 < pathRemainder.size())
                            {
                                pathRemainder = pathRemainder.substr(location
                                        + 1);
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
        } while(!parentStack.empty());
    }
    if(found)
    {
        *found = false;
    }
    if(_debugOutput)
    {
        std::cerr << "Path: " << path << " Attr: " << attribute << " value: "
                << def << " (default)" << std::endl;
    }
    return def;
}

float ConfigManager::getFloat(std::string path, float def, bool * found)
{
    return getFloat("value", path, def, found);
}

float ConfigManager::getFloat(std::string attribute, std::string path,
                              float def, bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute, path, ss.str(), &hasEntry);
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
    return getDouble("value", path, def, found);
}

double ConfigManager::getDouble(std::string attribute, std::string path,
                              double def, bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute, path, ss.str(), &hasEntry);
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
    return getInt("value", path, def, found);
}

int ConfigManager::getInt(std::string attribute, std::string path, int def,
                          bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute, path, ss.str(), &hasEntry);
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
    return getBool("value", path, def, found);
}

bool ConfigManager::getBool(std::string attribute, std::string path, bool def,
                            bool * found)
{
    bool hasEntry = false;
    std::stringstream ss;
    ss << def;
    std::string result = getEntry(attribute, path, ss.str(), &hasEntry);
    if(hasEntry)
    {
        if(found)
        {
            *found = true;
        }
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
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
	std::string attributeZ, std::string path, osg::Vec3 def,
	bool * found)
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

osg::Vec4 ConfigManager::getVec4(std::string path, osg::Vec4 def, 
	bool * found)
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

osg::Vec3d ConfigManager::getVec3d(std::string path, osg::Vec3d def, bool * found)
{
    return getVec3d("x","y","z",path,def,found);
}

osg::Vec3d ConfigManager::getVec3d(std::string attributeX, std::string attributeY, 
	std::string attributeZ, std::string path, osg::Vec3d def,
	bool * found)
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

osg::Vec4d ConfigManager::getVec4d(std::string attributeX, std::string attributeY, 
	std::string attributeZ, std::string attributeW, std::string path, 
	osg::Vec4d def, bool * found)
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
            pathFrag = pathRemainder.substr(0, location);
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
                const char * nameAtt = mxmlElementGetAttr(xmlNode, "name");
                std::string suffix = nameAtt ? nameAtt : "";

                //std::cerr << "Looking at node: " << nodeName << " with suffix " << suffix << std::endl;

                location = pathFrag.find_first_of(':');
                if((location == std::string::npos && pathFrag == nodeName)
                        || (location != std::string::npos && pathFrag
                                == nodeName + ":" + suffix))
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
                                if(strncmp(cnode->value.element.name, "!--", 3))
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
                                         std::pair<mxml_node_t *,std::string>(
                                                                              xmlNode,
                                                                              pathFrag));
                        location = pathRemainder.find_first_of('.');
                        if(location == std::string::npos)
                        {
                            pathFrag = pathRemainder;
                            pathRemainder = "";
                        }
                        else
                        {
                            pathFrag = pathRemainder.substr(0, location);
                            if(location + 1 < pathRemainder.size())
                            {
                                pathRemainder = pathRemainder.substr(location
                                        + 1);
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
        } while(!parentStack.empty());
    }
    return;
}
