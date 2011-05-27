#ifdef WITH_INTERLEAVER

#include <kernel/ScreenLenticular.h>
#include <kernel/CVRViewer.h>
#include <kernel/CalVR.h>
#include <config/ConfigManager.h>
#include <input/TrackingManager.h>

#include <osgViewer/Renderer>

#include <iostream>
#include <sstream>
#include <cstdio>

#include <mxml.h>

#define FEET2MM 304.8

using namespace cvr;

std::map<int,il_context*> ScreenLenticular::_contextMap;

ScreenLenticular::ScreenLenticular() : ScreenBase()
{
    _lentDisp = NULL;
}

ScreenLenticular::~ScreenLenticular()
{
}

void ScreenLenticular::init(int)
{
    _shaderDir = CalVR::instance()->getHomeDir();
    _shaderDir += "/shaders/";

    initDisplay();
    if(!_lentDisp)
    {
        std::cerr << "Error setting up lenticular display." << std::endl;
        return;
    }

    _eyePos = new float[3 * _eyes];
    il_viewpoints(_lentDisp, _ipd, _viewDistance, _eyePos, _eyes);

    _postCallback = new PostDrawCallback;
    _postCallback->count = 0;
    _postCallback->eyes = _eyes;
    _postCallback->contextMap = &_contextMap;
    _postCallback->lentDisp = _lentDisp;
    _postCallback->eyePos = _eyePos;
    _postCallback->cameraList = & _cameraList;

    for(int i = 0; i < _eyes; i++)
    {
        _cameraList.push_back(new osg::Camera());
	osg::DisplaySettings * ds = new osg::DisplaySettings();
	_cameraList[i]->setDisplaySettings(ds);
        CVRViewer::instance()->addSlave(_cameraList[i].get(), osg::Matrixd(), osg::Matrixd());
        defaultCameraInit(_cameraList[i].get());

        PreDrawCallback * predc = new PreDrawCallback;
        predc->lentDisp = _lentDisp;
        predc->eyes = _eyes;
        predc->eye = i;
        predc->vertShader = _shaderDir + "interleaver.vert";
        predc->fragShader = _shaderDir + "interleaver.frag";
        predc->contextMap = &_contextMap;
        predc->cam = _cameraList[i].get();

        _preCallbackList.push_back(predc);

        _cameraList[i]->setPostDrawCallback(_postCallback);
        _cameraList[i]->setPreDrawCallback(predc);

        _viewList.push_back(osg::Matrix());
        _projList.push_back(osg::Matrix());
    }
}

void ScreenLenticular::computeViewProj()
{
    for(int i = 0; i < _eyes; i++)
    {
        osg::Vec3 eyePos;
        eyePos.x() = (-3.5 + i) * _ipd * FEET2MM;
        eyePos = (eyePos * _eyeSepMult) * getCurrentHeadMatrix();

        computeDefaultViewProj(eyePos,_viewList[i],_projList[i]);
    }
}

void ScreenLenticular::updateCamera()
{
    for(int i = 0; i < _cameraList.size(); i++)
    {
        _cameraList[i]->setViewMatrix(_viewList[i]);
        _cameraList[i]->setProjectionMatrix(_projList[i]);
    }
}

void ScreenLenticular::setClearColor(osg::Vec4 color)
{
    for(int i = 0; i < _cameraList.size(); i++)
    {
        _cameraList[i]->setClearColor(color);
    }
}

ScreenInfo * ScreenLenticular::findScreenInfo(osg::Camera * c)
{
    for(int i = 0; i < _cameraList.size(); i++)
    {
        if(c == _cameraList[i].get())
        {
            return _myInfo;
        }
    }
    return NULL;
}

void ScreenLenticular::initDisplay()
{
    std::stringstream ss;
    ss << "ChannelConfig.Channel:" << _myInfo->channelIndex << ".";

    std::string file = ConfigManager::getEntry("value",ss.str() + "ThumbConfigFile","");

    if(!file.empty() && readFromThumbFile(file))
    {
        std::cerr << "Read config from thumb file." << std::endl;
        return;
    }

    _eyes = ConfigManager::getInt("eyes",ss.str() + "Viewer",8);
    _ipd = ConfigManager::getFloat("ipd",ss.str() + "Viewer",0.208);
    _viewDistance = ConfigManager::getFloat("distance",ss.str() + "Viewer",9.7);
    _lentDisp = il_init_display(_eyes);

    _lentDisp->viewport_x = (int)_myInfo->myChannel->left;
    _lentDisp->viewport_y = (int)_myInfo->myChannel->bottom;
    _lentDisp->viewport_w = (int)_myInfo->myChannel->width;
    _lentDisp->viewport_h = (int)_myInfo->myChannel->height;
    _lentDisp->quality = ConfigManager::getFloat("quality",ss.str() + "Array",1.0);
    _lentDisp->debug = 1.0;
    _lentDisp->pitch = ConfigManager::getFloat("pitch",ss.str() + "Array",446.862244);
    _lentDisp->angle = ConfigManager::getFloat("angle",ss.str() + "Array",18.435000);
    _lentDisp->thick = ConfigManager::getFloat("thick",ss.str() + "Array",0.027600);
    _lentDisp->shift = ConfigManager::getFloat("shift",ss.str() + "Array",0.009270);

    _lentDisp->BL[0] = ConfigManager::getFloat("x",ss.str() + "BL",-0.851653);
    _lentDisp->BL[1] = ConfigManager::getFloat("y",ss.str() + "BL",-0.532283);
    _lentDisp->BL[2] = ConfigManager::getFloat("z",ss.str() + "BL",0.0);

    _lentDisp->BR[0] = ConfigManager::getFloat("x",ss.str() + "BR",0.851653);
    _lentDisp->BR[1] = ConfigManager::getFloat("y",ss.str() + "BR",-0.532283);
    _lentDisp->BR[2] = ConfigManager::getFloat("z",ss.str() + "BR",0.0);

    _lentDisp->TL[0] = ConfigManager::getFloat("x",ss.str() + "TL",-0.851653);
    _lentDisp->TL[1] = ConfigManager::getFloat("y",ss.str() + "TL",0.532283);
    _lentDisp->TL[2] = ConfigManager::getFloat("z",ss.str() + "TL",0.0);

    for(int i = 0; i < _eyes; i++)
    {
        std::stringstream slice;
        slice << "Slice:" << i;

        _lentDisp->cycle[i] = ConfigManager::getFloat("cycle",ss.str() + slice.str(),0.875);
        _lentDisp->step0[i] = ConfigManager::getFloat("step0",ss.str() + slice.str(),0.0);
        _lentDisp->step1[i] = ConfigManager::getFloat("step1",ss.str() + slice.str(),0.0);
        _lentDisp->step2[i] = ConfigManager::getFloat("step2",ss.str() + slice.str(),0.0);
        _lentDisp->step3[i] = ConfigManager::getFloat("step3",ss.str() + slice.str(),0.0);
        _lentDisp->depth[i] = ConfigManager::getFloat("depth",ss.str() + slice.str(),0.0);
    }
}

bool ScreenLenticular::readFromThumbFile(std::string file)
{

    char hostname[51];
    gethostname(hostname, 50);
    std::string myHostName = hostname;

    size_t lpos = myHostName.find(".local");
    if(lpos != std::string::npos)
    {
        myHostName = myHostName.substr(0, lpos);
    }

    FILE *fp;
    mxml_node_t * tree;

    fp = fopen(file.c_str(), "r");
    if(fp == NULL)
    {
        std::cerr << "Unable to open file: " << file << std::endl;
        return false;
    }
    tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        std::cerr << "Unable to parse XML file: " << file << std::endl;
        return false;
    }

    mxml_node_t *node;

    for (node = mxmlFindElement(tree, tree, "client", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, tree, "client", NULL, NULL, MXML_DESCEND))
    {
        std::string host, xmlfile;
        int context;

        xmlfile = mxmlElementGetAttr(node, "name");
        size_t pos = xmlfile.find_last_of('/');
        if(pos != std::string::npos)
        {
            xmlfile = xmlfile.substr(pos);
        }
        pos = file.find_last_of('/');

        //osg::notify(osg::ALWAYS) << "file: " << file << " xmlfile: " << xmlfile << " 1: " << file.substr(0, pos) << std::endl;

        xmlfile = file.substr(0, pos) + "/" + xmlfile;

        const char * con = mxmlElementGetAttr(node, "disp");
        sscanf(con, ":0.%d", &context);

        host = mxmlElementGetAttr(node, "addr");

        std::cerr << "file: " << xmlfile << " context: " << context << " host: " << host << std::endl;

        if(host != myHostName || context != _myInfo->myChannel->windowIndex)
        {
            continue;
        }

        mxml_node_t * tree2;

        fp = fopen(xmlfile.c_str(), "r");
        if(fp == NULL)
        {
            std::cerr << "Unable to open file: " << xmlfile << std::endl;
            return false;
        }
        tree2 = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
        fclose(fp);

        if(tree2 == NULL)
        {
            std::cerr << "Unable to parse XML file: " << xmlfile << std::endl;
            return false;
        }

        mxml_node_t * infonode = mxmlFindElement(tree2, tree2, "channel", NULL, NULL, MXML_DESCEND);

        if(infonode != NULL)
        {
            _ipd = atof(mxmlElementGetAttr(infonode, "d"));
            _viewDistance = atof(mxmlElementGetAttr(infonode, "z"));
        }

        mxml_node_t * node2;

        for (node2 = mxmlFindElement(tree2, tree2, "display", NULL, NULL, MXML_DESCEND); node2 != NULL; node2 = mxmlFindElement(node2, tree2, "display", NULL, NULL, MXML_DESCEND))
        {
            mxml_node_t * node3;
            node3 = mxmlFindElement(node2, node2, "viewport", NULL, NULL, MXML_DESCEND);
            if(!node3)
            {
                std::cerr << "Unable to find viewport in file: " << xmlfile << " for display: " << _screenIndex << std::endl;
                continue;
            }
            int vx, vy, vw, vh;

            vx = atoi(mxmlElementGetAttr(node3, "x"));
            vy = atoi(mxmlElementGetAttr(node3, "y"));
            vw = atoi(mxmlElementGetAttr(node3, "w"));
            vh = atoi(mxmlElementGetAttr(node3, "h"));

            if(vx != _myInfo->myChannel->left || vy != _myInfo->myChannel->bottom || vw != (int)_myInfo->myChannel->width || vh != (int)_myInfo->myChannel->height)
            {
                continue;
            }

            _screenIndex = atoi(mxmlElementGetAttr(node2, "index"));
            _eyes = atoi(mxmlElementGetAttr(node2, "channels"));

            _lentDisp = il_init_display(_eyes);
            if(!_lentDisp)
            {
                std::cerr << "Error creating display for " << _screenIndex << std::endl;
                return false;
            }

            for(int i = 0; i < _eyes; i++)
            {
                _lentDisp->step0[i] = 0.0;
                _lentDisp->step1[i] = 0.0;
                _lentDisp->step2[i] = 0.0;
                _lentDisp->step3[i] = 0.0;
                _lentDisp->depth[i] = 0.0;
            }

            _lentDisp->viewport_x = vx;
            _lentDisp->viewport_y = vy;
            _lentDisp->viewport_w = vw;
            _lentDisp->viewport_h = vh;
            //_lentDisp->viewport_w = (int)_myInfo->myChannel->width;
            //_lentDisp->viewport_h = (int)_myInfo->myChannel->height;

            bool hadError = false;

            node3 = mxmlFindElement(node2, node2, NULL, "name", "BL", MXML_DESCEND);
            if(!node3)
            {
                std::cerr << "Unable to find BL in file: " << xmlfile << " for display: " << _screenIndex << std::endl;
                hadError = true;
            }

            _lentDisp->BL[0] = atof(mxmlElementGetAttr(node3, "x"));
            _lentDisp->BL[1] = atof(mxmlElementGetAttr(node3, "y"));
            _lentDisp->BL[2] = atof(mxmlElementGetAttr(node3, "z"));

            node3 = mxmlFindElement(node2, node2, NULL, "name", "BR", MXML_DESCEND);
            if(!node3)
            {
                std::cerr << "Unable to find BR in file: " << xmlfile << " for display: " << _screenIndex << std::endl;
                hadError = true;
            }

            _lentDisp->BR[0] = atof(mxmlElementGetAttr(node3, "x"));
            _lentDisp->BR[1] = atof(mxmlElementGetAttr(node3, "y"));
            _lentDisp->BR[2] = atof(mxmlElementGetAttr(node3, "z"));

            node3 = mxmlFindElement(node2, node2, NULL, "name", "TL", MXML_DESCEND);
            if(!node3)
            {
                std::cerr << "Unable to find TL in file: " << xmlfile << " for display: " << _screenIndex << std::endl;
                hadError = true;
            }

            _lentDisp->TL[0] = atof(mxmlElementGetAttr(node3, "x"));
            _lentDisp->TL[1] = atof(mxmlElementGetAttr(node3, "y"));
            _lentDisp->TL[2] = atof(mxmlElementGetAttr(node3, "z"));

            //display->center[0] = (display->TL[0] + display->BR[0]) / 2.0;
            //display->center[1] = (display->TL[1] + display->BR[1]) / 2.0;
            //display->center[2] = (display->TL[2] + display->BR[2]) / 2.0;

            node3 = mxmlFindElement(node2, node2, "array", NULL, NULL, MXML_DESCEND);
            if(!node3)
            {
                std::cerr << "Unable to find array in file: " << xmlfile << " for display: " << _screenIndex << std::endl;
                hadError = true;
            }

            _lentDisp->quality = atof(mxmlElementGetAttr(node3, "quality"));
            _lentDisp->pitch = atof(mxmlElementGetAttr(node3, "pitch"));
            _lentDisp->angle = atof(mxmlElementGetAttr(node3, "angle"));
            _lentDisp->thick = atof(mxmlElementGetAttr(node3, "thick"));
            _lentDisp->shift = atof(mxmlElementGetAttr(node3, "shift"));
            _lentDisp->vdist2ipd = mxmlElementGetAttr(node3, "vdist2ipd") ? atof(mxmlElementGetAttr(node3, "vdist2ipd")) : 0.023;

            for(node3 = mxmlFindElement(node2, node2, "slice", NULL, NULL, MXML_DESCEND); node3 != NULL; node3 = mxmlFindElement(node3, node2, "slice", NULL, NULL, MXML_DESCEND))
            {
                int snum = atoi(mxmlElementGetAttr(node3, "index"));
                _lentDisp->cycle[snum] = atof(mxmlElementGetAttr(node3, "cycle"));
            }

            mxmlDelete(tree2);
            mxmlDelete(tree);

            if(hadError)
            {
                il_fini_display(_lentDisp);
                _lentDisp = NULL;
                return false;
            }

            return true;
        }
        mxmlDelete(tree2);
    }
    mxmlDelete(tree);

    return false;
}

OpenThreads::Mutex ScreenLenticular::PreDrawCallback::mutex;

void ScreenLenticular::PreDrawCallback::operator()(osg::RenderInfo & ri) const
{
    int context = ri.getContextID();
    if(!(*contextMap)[context])
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        (*contextMap)[context] = il_init_context(eyes,lentDisp->viewport_w,lentDisp->viewport_h,lentDisp->quality,vertShader.c_str(),fragShader.c_str(),"","");
    }
    if((*contextMap)[context])
    {
        osg::Viewport * vp = cam->getViewport();
        vp->x() = 0;
        vp->y() = 0;
        vp->width() = lentDisp->viewport_w * lentDisp->quality;
        vp->height() = lentDisp->viewport_h * lentDisp->quality;
        //std::cerr << "Prep disp " << eye << std::endl;
        il_prep((*contextMap)[context], lentDisp, eye);
    }
}

void ScreenLenticular::PostDrawCallback::operator()(osg::RenderInfo & ri) const
{
    int context = ri.getContextID();
    count++;

    if(count == eyes && lentDisp && (*contextMap)[context])
    {
        for(int i = 0; i < cameraList->size(); i++)
        {
            osg::Viewport * vp = (*cameraList)[i]->getViewport();
            vp->x() = lentDisp->viewport_x;
            vp->y() = lentDisp->viewport_y;
            vp->width() = lentDisp->viewport_w;
            vp->height() = lentDisp->viewport_h;
        }
        //std::cerr << "Draw frame." << std::endl;
        glDepthMask(GL_FALSE);
        il_draw((*contextMap)[context], lentDisp, eyePos);
        glDepthMask(GL_TRUE);
        count = 0;
    }
    //glFinish();
}

#endif
