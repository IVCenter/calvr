#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/ScreenMono.h>
#include <cvrKernel/ScreenStereo.h>
#include <cvrKernel/ScreenInterlacedTopBottom.h>
#include <cvrKernel/ScreenMVShader.h>
#include <cvrKernel/ScreenMVZones.h>
#include <cvrKernel/ScreenMVMaster.h>
#include <cvrKernel/ScreenMVStencil.h>
#include <cvrKernel/ScreenFixedViewer.h>
#include <cvrKernel/ScreenHMD.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrInput/TrackingManager.h>
#include <cvrConfig/ConfigManager.h>

#include <osgViewer/GraphicsWindow>

#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <locale>

#ifdef WITH_INTERLEAVER
#include <cvrKernel/ScreenLenticular.h>
#endif

#ifdef WITH_OVR
#include <cvrKernel/ScreenOculus.h>
#endif

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

double ScreenBase::_separation;
double ScreenBase::_eyeSepMult = 1.0;
double ScreenBase::_near;
double ScreenBase::_far;
bool ScreenBase::_omniStereo = false;
ScreenConfig * ScreenConfig::_myPtr = NULL;

ScreenConfig::ScreenConfig()
{
}

ScreenConfig::~ScreenConfig()
{
}

ScreenConfig * ScreenConfig::instance()
{
    if(!_myPtr)
    {
        _myPtr = new ScreenConfig();
    }
    return _myPtr;
}

bool ScreenConfig::init()
{
    char gsyncenv[128];
    memset(gsyncenv,'\0',128);
    if(ConfigManager::getBool("SyncToVBlank"))
    {
        strncpy(gsyncenv,"__GL_SYNC_TO_VBLANK=1",128);
        //putenv("__GL_SYNC_TO_VBLANK=1");
    }
    else
    {
        strncpy(gsyncenv,"__GL_SYNC_TO_VBLANK=0",128);
        //putenv("__GL_SYNC_TO_VBLANK=0");
    }
    putenv(gsyncenv);

    char * displayptr = getenv("DISPLAY");
    std::string displayenv;

    if(displayptr)
    {
        displayenv = displayptr;
    }

    _displayServer = 0;
    _displayPipe = 0;
    if(!displayenv.empty())
    {
        size_t pos = displayenv.find_last_of(':');
        if(pos != std::string::npos)
        {
            char * deptr = &displayenv[pos];
            int ret;
            if((ret = sscanf(deptr,":%d.%d",&_displayServer,&_displayScreen))
                    != 2)
            {
                std::cerr << "Warning: Unable to parse DISPLAY variable."
                        << std::endl;
                _displayServer = _displayScreen = 0;
            }
            else
            {
                //std::cerr << "Display server: " << _displayServer << " screen: " << _displayScreen << std::endl;
            }
        }
    }

    if(!readPipes() || !readWindows() || !readChannels() || !readScreens())
    {
        return false;
    }

    ScreenBase::_near = ConfigManager::getFloat("Near",10.0);
    ScreenBase::_far = ConfigManager::getFloat("Far",10000000);
    ScreenBase::_separation = ConfigManager::getFloat("separation","Stereo",
            64.0);
    ScreenBase::_omniStereo = ConfigManager::getBool("omniStereo","Stereo",
            false,NULL);

    if(!makeWindows() || !makeScreens())
    {
        return false;
    }

    float r, g, b, a;
    r = ConfigManager::getFloat("r","Background",0.0);
    g = ConfigManager::getFloat("g","Background",0.0);
    b = ConfigManager::getFloat("b","Background",0.0);
    a = ConfigManager::getFloat("a","Background",0.0);

    setClearColor(osg::Vec4(r,g,b,a));

    //computeViewProj();
    //updateCamera();

    bool stereoStatus = ConfigManager::getBool("value","EyeSeparation",true);

    if(stereoStatus)
    {
        setEyeSeparationMultiplier(1.0);
    }
    else
    {
        setEyeSeparationMultiplier(0.0);
    }

    std::cerr << "Screens Created: " << _screenList.size() << std::endl;

    return true;
}

void ScreenConfig::computeViewProj()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    for(int i = 0; i < _screenList.size(); i++)
    {
        _screenList[i]->computeViewProj();
    }
}

void ScreenConfig::updateCamera()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    for(int i = 0; i < _screenList.size(); i++)
    {
        _screenList[i]->updateCamera();
    }
}

int ScreenConfig::getNumWindows()
{
    return _windowInfoList.size();
}

WindowInfo * ScreenConfig::getWindowInfo(int window)
{
    if(window < 0 || window >= _windowInfoList.size())
    {
        return NULL;
    }
    return _windowInfoList[window];
}

int ScreenConfig::getNumScreens()
{
    return _screenList.size();
}

ScreenBase * ScreenConfig::getScreen(int screen)
{
    if(screen < 0 || screen >= _screenList.size())
    {
        return NULL;
    }

    return _screenList[screen];
}

ScreenInfo * ScreenConfig::getScreenInfo(int screen)
{
    if(screen < 0 || screen >= _screenList.size())
    {
        return NULL;
    }

    return _screenList[screen]->getScreenInfo();
}

ScreenInfo * ScreenConfig::getMasterScreenInfo(int screen)
{
    if(ComController::instance()->isMaster())
    {
        if(screen < 0 || screen >= _screenInfoList.size())
        {
            return NULL;
        }

        return _screenInfoList[screen];
    }
    else
    {
        if(screen < 0 || screen >= _masterScreenInfoList.size())
        {
            return NULL;
        }

        return _masterScreenInfoList[screen];
    }
}

void ScreenConfig::setEyeSeparationMultiplier(float mult)
{
    ScreenBase::_eyeSepMult = mult;
}

float ScreenConfig::getEyeSeparationMultiplier()
{
    return ScreenBase::_eyeSepMult;
}

int ScreenConfig::getCudaDevice(int context)
{
    for(int i = 0; i < _windowInfoList.size(); i++)
    {
        if(_windowInfoList[i]->gc)
        {
            if(_windowInfoList[i]->gc->getState()->getContextID() == context)
            {
                return _windowInfoList[i]->cudaDevice;
            }
        }
    }

    return 0;
}

int ScreenConfig::getNumContexts(int cudaDevice)
{
    int contexts = 0;

    for(int i = 0; i < _windowInfoList.size(); i++)
    {
        if(_windowInfoList[i]->cudaDevice == cudaDevice)
        {
            contexts++;
        }
    }

    return contexts;
}

bool ScreenConfig::readPipes()
{
    // find out how many pipes are specifed
    int configPipes = ConfigManager::getInt("NumPipes",100);
    bool found;
    struct PipeInfo * pipePtr;
    do
    {
        std::stringstream ss;
        ss << "PipeConfig.Pipe:" << _pipeInfoList.size();
        ConfigManager::getEntry("name",ss.str(),"",&found);
        if(found)
        {
            pipePtr = new struct PipeInfo;
            pipePtr->server = ConfigManager::getInt("server",ss.str(),0);
            pipePtr->screen = ConfigManager::getInt("screen",ss.str(),0);
            _pipeInfoList.push_back(pipePtr);
        }
    }
    while(found && _pipeInfoList.size() < configPipes);

    if(_pipeInfoList.size() == 0)
    {
        std::cerr << "No PipeConfig Found, using :0.0" << std::endl;
        pipePtr = new struct PipeInfo;
        pipePtr->server = 0;
        pipePtr->screen = 0;
        _pipeInfoList.push_back(pipePtr);
    }

    bool displayPipeFound = false;
    for(int i = 0; i < _pipeInfoList.size(); i++)
    {
        if(_pipeInfoList[i]->server == _displayServer
                && _pipeInfoList[i]->screen == _displayScreen)
        {
            displayPipeFound = true;
            _displayPipe = i;
            break;
        }
    }

    if(!displayPipeFound)
    {
        std::cerr << "Adding DISPLAY pipe" << std::endl;
        PipeInfo * pi = new PipeInfo;
        pi->server = _displayServer;
        pi->screen = _displayScreen;
        _pipeInfoList.push_back(pi);
        _displayPipe = _pipeInfoList.size() - 1;
    }

    std::cerr << "Found " << _pipeInfoList.size() << " pipe(s)." << std::endl;

    return true;
}

bool ScreenConfig::readWindows()
{
    // find out how many windows are specifed
    int configWindows = ConfigManager::getInt("NumWindows",100);
    bool found;
    struct WindowInfo * windowPtr;
    do
    {
        std::stringstream ss;
        ss << "WindowConfig.Window:" << _windowInfoList.size();
        ConfigManager::getEntry("name",ss.str(),"",&found);
        if(found)
        {
            windowPtr = new struct WindowInfo;
            windowPtr->width = ConfigManager::getInt("width",ss.str(),600);
            windowPtr->height = ConfigManager::getInt("height",ss.str(),400);
            windowPtr->left = ConfigManager::getInt("left",ss.str(),0);
            windowPtr->bottom = ConfigManager::getInt("bottom",ss.str(),0);
            windowPtr->decoration = ConfigManager::getBool("decoration",
                    ss.str(),false);
            windowPtr->supportsResize = ConfigManager::getBool("supportsResize",
                    ss.str(),false);
            windowPtr->overrideRedirect = ConfigManager::getBool(
                    "overrideRedirect",ss.str(),false);
            windowPtr->useCursor = ConfigManager::getBool("useCursor",ss.str(),
                    ComController::instance()->isMaster() ? true : false);
            windowPtr->quadBuffer = ConfigManager::getBool("quadBuffer",
                    ss.str(),false);

            int pipeIndex;
            std::string pipe = ConfigManager::getEntry("pipeIndex",ss.str(),"");
            std::transform(pipe.begin(),pipe.end(),pipe.begin(),::tolower);
            if(pipe == "display")
            {
                pipeIndex = _displayPipe;
            }
            else
            {
                pipeIndex = ConfigManager::getInt("pipeIndex",ss.str(),0);
            }

            if(pipeIndex < _pipeInfoList.size())
            {
                windowPtr->pipeIndex = pipeIndex;
                windowPtr->myPipe = _pipeInfoList[pipeIndex];
            }
            else
            {
                std::cerr << "Invalid Pipe specified for window "
                        << _windowInfoList.size() << std::endl;
                delete windowPtr;
                break;
            }
            windowPtr->contextGroup = ConfigManager::getInt("contextGroup",
                    ss.str(),-1);
            windowPtr->cudaDevice = ConfigManager::getInt("cudaDevice",ss.str(),
                    0);
            _windowInfoList.push_back(windowPtr);
        }
    }
    while(found && _windowInfoList.size() < configWindows);

    if(_windowInfoList.size() == 0)
    {
        std::cerr << "No WindowConfig Found." << std::endl;
        return false;
    }

    std::cerr << "Found " << _windowInfoList.size() << " window(s)."
            << std::endl;

    return true;
}

bool ScreenConfig::readChannels()
{
    // find out how many channels are specifed
    int configChannels = ConfigManager::getInt("NumChannels",100);
    bool found;
    struct ChannelInfo * channelPtr;
    do
    {
        std::stringstream ss;
        ss << "ChannelConfig.Channel:" << _channelInfoList.size();
        ConfigManager::getEntry("name",ss.str(),"",&found);
        if(found)
        {
            channelPtr = new struct ChannelInfo;
            channelPtr->width = ConfigManager::getFloat("width",ss.str(),600);
            channelPtr->height = ConfigManager::getFloat("height",ss.str(),400);
            channelPtr->left = ConfigManager::getFloat("left",ss.str(),0);
            channelPtr->bottom = ConfigManager::getFloat("bottom",ss.str(),0);
            channelPtr->stereoMode = ConfigManager::getEntry("stereoMode",
                    ss.str(),"MONO");
			channelPtr->swapStereoEyes = ConfigManager::getBool("swapStereoEyes", ss.str(), false);
            channelPtr->head = ConfigManager::getInt("head",ss.str(),0);
            if(channelPtr->head >= TrackingManager::instance()->getNumHeads())
            {
                channelPtr->head = 0;
            }
            int windowIndex = ConfigManager::getInt("windowIndex",ss.str(),0);
            if(windowIndex < _windowInfoList.size())
            {
                channelPtr->windowIndex = windowIndex;
                channelPtr->myWindow = _windowInfoList[windowIndex];
            }
            else
            {
                std::cerr << "Invalid window specified for channel "
                        << _channelInfoList.size() << std::endl;
                delete channelPtr;
                break;
            }
            _channelInfoList.push_back(channelPtr);
        }
    }
    while(found && _channelInfoList.size() < configChannels);

    if(_channelInfoList.size() == 0)
    {
        std::cerr << "No ChannelConfig Found." << std::endl;
        return false;
    }

    std::cerr << "Found " << _channelInfoList.size() << " channel(s)."
            << std::endl;
    return true;
}

bool ScreenConfig::readScreens()
{
    // find out how many screens are specifed
    int configScreens = ConfigManager::getInt("NumScreens",100);
    bool found;
    struct ScreenInfo * screenPtr;
    do
    {
        std::stringstream ss;
        ss << "ScreenConfig.Screen:" << _screenInfoList.size();
        ConfigManager::getEntry("name",ss.str(),"",&found);
        if(found)
        {
            screenPtr = new struct ScreenInfo;
            screenPtr->width = ConfigManager::getFloat("width",ss.str(),600);
            screenPtr->height = ConfigManager::getFloat("height",ss.str(),400);
            screenPtr->h = ConfigManager::getFloat("h",ss.str(),0);
            screenPtr->p = ConfigManager::getFloat("p",ss.str(),0);
            screenPtr->r = ConfigManager::getFloat("r",ss.str(),0);
            float x, y, z;
            x = ConfigManager::getFloat("originX",ss.str(),0);
            y = ConfigManager::getFloat("originY",ss.str(),0);
            z = ConfigManager::getFloat("originZ",ss.str(),0);
            screenPtr->xyz = osg::Vec3(x,y,z);

            osg::Matrix trans, rot;
            trans.makeTranslate(screenPtr->xyz);
            rot.makeRotate(
                    osg::Quat(screenPtr->r * M_PI / 180.0,osg::Vec3(0,1,0),
                            screenPtr->p * M_PI / 180.0,osg::Vec3(1,0,0),
                            screenPtr->h * M_PI / 180.0,osg::Vec3(0,0,1)));
            screenPtr->transform = rot * trans;

            int channelIndex = ConfigManager::getInt("channelIndex",ss.str(),
                    _screenInfoList.size());
            if(channelIndex < _channelInfoList.size())
            {
                screenPtr->channelIndex = channelIndex;
                screenPtr->myChannel = _channelInfoList[channelIndex];
            }
            else
            {
                std::cerr << "Invalid channel specified for screen "
                        << _screenInfoList.size() << std::endl;
                delete screenPtr;
                break;
            }
            _screenInfoList.push_back(screenPtr);
        }
    }
    while(found && _screenInfoList.size() < configScreens);

    if(_screenInfoList.size() == 0)
    {
        std::cerr << "No ScreenConfig Found." << std::endl;
        return false;
    }

    std::cerr << "Found " << _screenInfoList.size() << " screen(s)."
            << std::endl;
    return true;
}

bool ScreenConfig::makeWindows()
{
    osg::DisplaySettings * ds = osg::DisplaySettings::instance();
    ds->setNumMultiSamples(ConfigManager::getInt("MultiSample",0));

    std::map<int,osg::GraphicsContext*> contextMap;

    for(int i = 0; i < _windowInfoList.size(); i++)
    {
        osg::GraphicsContext::Traits * traits = new osg::GraphicsContext::Traits;
        traits->x = _windowInfoList[i]->left;
        traits->y = _windowInfoList[i]->bottom;
        traits->width = _windowInfoList[i]->width;
        traits->height = _windowInfoList[i]->height;
        traits->windowDecoration = _windowInfoList[i]->decoration;
        traits->doubleBuffer = true;
        traits->quadBufferStereo = _windowInfoList[i]->quadBuffer;
        traits->vsync = ConfigManager::getBool("SyncToVBlank",false);
        traits->sharedContext = 0;
        traits->windowName = "CalVR";
        traits->displayNum = _windowInfoList[i]->myPipe->server;
        traits->screenNum = _windowInfoList[i]->myPipe->screen;
        traits->supportsResize = _windowInfoList[i]->supportsResize;
        traits->overrideRedirect = _windowInfoList[i]->overrideRedirect;
        traits->useCursor = _windowInfoList[i]->useCursor;
        if(ConfigManager::getBool("Stencil",false))
        {
            traits->stencil = 8;
        }

        int samples = ConfigManager::getInt("MultiSample",0);
        if(samples)
        {
            traits->samples = samples;
            traits->sampleBuffers = 1;
        }

        if(_windowInfoList[i]->contextGroup >= 0)
        {
            /*if(!contextMap[_windowInfoList[i]->contextGroup])
             {
             contextMap[_windowInfoList[i]->contextGroup] = osg::GraphicsContext::createGraphicsContext(traits);
             }
             traits->sharedContext = contextMap[_windowInfoList[i]->contextGroup];
             _windowInfoList[i]->gc = osg::GraphicsContext::createGraphicsContext(
             traits);*/
            if(contextMap[_windowInfoList[i]->contextGroup])
            {
                traits->sharedContext =
                        contextMap[_windowInfoList[i]->contextGroup];
                _windowInfoList[i]->gc =
                        osg::GraphicsContext::createGraphicsContext(traits);
            }
            else
            {
                traits->sharedContext = NULL;
                _windowInfoList[i]->gc =
                        osg::GraphicsContext::createGraphicsContext(traits);
                contextMap[_windowInfoList[i]->contextGroup] =
                        _windowInfoList[i]->gc;
            }
        }
        else
        {
            _windowInfoList[i]->gc =
                    osg::GraphicsContext::createGraphicsContext(traits);
        }

        if(!_windowInfoList[i]->gc)
        {
            std::cerr << "Error: failed to create graphics context for window: "
                    << i << std::endl;
            return false;
        }

        osgViewer::GraphicsWindow * gw =
                dynamic_cast<osgViewer::GraphicsWindow*>(_windowInfoList[i]->gc);
        if(gw)
        {
            //std::cerr << "className: " << gw->className() << std::endl;
            std::string name = gw->className();
            if(name == "GraphicsWindowCarbon" || name == "GraphicsWindowCocoa")
            {
                CVRViewer::instance()->setInvertMouseY(true);
            }
        }
    }

    return true;
}

bool ScreenConfig::makeScreens()
{
    // make screens
    for(int i = 0; i < _screenInfoList.size(); i++)
    {
        ScreenBase * screen = NULL;

        if(_screenInfoList[i]->myChannel->stereoMode == "MONO")
        {
            screen = new ScreenMono();
            screen->_myInfo = _screenInfoList[i];
            screen->init(ScreenMono::CENTER);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "LEFT")
        {
            //screen = new ScreenMono();
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            //screen->init(ScreenMono::LEFT);
            screen->init(osg::DisplaySettings::LEFT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "RIGHT")
        {
            //screen = new ScreenMono();
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            //screen->init(ScreenMono::RIGHT);
            screen->init(osg::DisplaySettings::RIGHT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "HORIZONTAL_INTERLACE")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::HORIZONTAL_INTERLACE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "VERTICAL_INTERLACE")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::VERTICAL_INTERLACE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "VERTICAL_SPLIT")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::VERTICAL_SPLIT);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "HORIZONTAL_SPLIT")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::HORIZONTAL_SPLIT);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "ANAGLYPHIC")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::ANAGLYPHIC);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "CHECKERBOARD")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::CHECKERBOARD);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "QUAD_BUFFER")
        {
            screen = new ScreenStereo();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::QUAD_BUFFER);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "MULTI_VIEWER")
        {
            screen = new ScreenMVStencil();
            screen->_myInfo = _screenInfoList[i];
            screen->init();
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_LEFT")
        {
            screen = new ScreenMVStencil();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::LEFT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_RIGHT")
        {
            screen = new ScreenMVStencil();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::RIGHT_EYE);
        }
#ifdef WITH_SCREEN_MULTI_VIEWER
        else if(_screenInfoList[i]->myChannel->stereoMode == "MULTI_VIEWER_AP")
        {
            screen = new ScreenMVShader();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::LEFT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_AP_LEFT")
        {
            screen = new ScreenMVShader();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::LEFT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_AP_RIGHT")
        {
            screen = new ScreenMVShader();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::RIGHT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_AP_HORIZONTAL_INTERLACE")
        {
            screen = new ScreenMVShader();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::HORIZONTAL_INTERLACE);
        }
#endif
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_2_LEFT")
        {
            screen = new ScreenMVZones();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::LEFT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_2_RIGHT")
        {
            screen = new ScreenMVZones();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::RIGHT_EYE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_2_HORIZONTAL_INTERLACE")
        {
            screen = new ScreenMVZones();
            screen->_myInfo = _screenInfoList[i];
            screen->init(osg::DisplaySettings::HORIZONTAL_INTERLACE);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "MULTI_VIEWER_MASTER")
        {
            screen = new ScreenMVMaster();
            screen->_myInfo = _screenInfoList[i];
            screen->init();
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "FIXED_VIEWER")
        {
            screen = new ScreenFixedViewer();
            screen->_myInfo = _screenInfoList[i];
            screen->init();
        }
        else if(_screenInfoList[i]->myChannel->stereoMode == "HMD")
        {
            screen = new ScreenHMD();
            screen->_myInfo = _screenInfoList[i];
            screen->init(-1);
        }
        else if(_screenInfoList[i]->myChannel->stereoMode
                == "INTERLACED_TOP_BOTTOM")
        {
            screen = new ScreenInterlacedTopBottom();
            screen->_myInfo = _screenInfoList[i];
            screen->init();
        }
#ifdef WITH_INTERLEAVER
        else if(_screenInfoList[i]->myChannel->stereoMode == "LENTICULAR")
        {
            screen = new ScreenLenticular();
            screen->_myInfo = _screenInfoList[i];
            screen->init();
        }
#endif
#ifdef WITH_OVR
		else if(_screenInfoList[i]->myChannel->stereoMode == "OCULUS")
        {
            screen = new ScreenOculus();
            screen->_myInfo = _screenInfoList[i];
            screen->init(-1);
        }
#endif
        if(screen)
        {
            _screenList.push_back(screen);
        }
        else
        {
            std::cerr << "No Screen type matching stereo mode: "
                    << _screenInfoList[i]->myChannel->stereoMode << std::endl;
            return false;
        }
    }
    return true;
}

void ScreenConfig::setClearColor(osg::Vec4 color)
{
    for(int i = 0; i < _screenList.size(); i++)
    {
        _screenList[i]->setClearColor(color);
    }
}

void ScreenConfig::findScreenInfo(osg::Camera * c, osg::Vec3 & center,
        float & width, float & height)
{
    if(_screenList.size() == 0)
    {
        return;
    }

    ScreenInfo * si = NULL;
    for(int i = 0; i < _screenList.size(); i++)
    {
        if((si = _screenList[i]->findScreenInfo(c)))
        {
            break;
        }
    }
    if(!si)
    {
        si = _screenInfoList[0];
    }

    center = si->xyz;
    width = si->width;
    height = si->height;
}

int ScreenConfig::findScreenNumber(osg::Camera * c)
{
    for(int i = 0; i < _screenList.size(); i++)
    {
        if(_screenList[i]->findScreenInfo(c))
        {
            return i;
        }
    }
    return -1;
}

void ScreenConfig::syncMasterScreens()
{
    if(ComController::instance()->isMaster())
    {
        //first sync values with created screens
        for(int i = 0; i < _windowInfoList.size(); i++)
        {
            if(_windowInfoList[i]->gc)
            {
                const osg::GraphicsContext::Traits * traits =
                        _windowInfoList[i]->gc->getTraits();
                //std::cerr << "Window width: " << traits->width << " height: " << traits->height << std::endl;
                _windowInfoList[i]->width = traits->width;
                _windowInfoList[i]->height = traits->height;
            }
        }

        for(int i = 0; i < _screenInfoList.size(); i++)
        {
            bool resized = false;
            float cwidth = 1024, cheight = 1024;
            if(_screenInfoList[i]->myChannel->myWindow->gc)
            {
                const osg::GraphicsContext::Traits * traits =
                        _screenInfoList[i]->myChannel->myWindow->gc->getTraits();
                if(traits)
                {
                    cwidth = traits->width;
                    cheight = traits->height;
                }
            }

            if(_screenInfoList[i]->myChannel->bottom
                    + _screenInfoList[i]->myChannel->height > cheight)
            {
                cheight = cheight - _screenInfoList[i]->myChannel->bottom;
                resized = true;
            }
            else
            {
                cheight = _screenInfoList[i]->myChannel->height;
            }

            if(_screenInfoList[i]->myChannel->left
                    + _screenInfoList[i]->myChannel->width > cwidth)
            {
                cwidth = cwidth - _screenInfoList[i]->myChannel->left;
                resized = true;
            }
            else
            {
                cwidth = _screenInfoList[i]->myChannel->width;
            }

            if(resized)
            {
                _screenList[i]->viewportResized(
                        (int)_screenInfoList[i]->myChannel->left,
                        (int)_screenInfoList[i]->myChannel->bottom,(int)cwidth,
                        (int)cheight);
                //std::cerr << "Screen width: " << _screenInfoList[i]->myChannel->width << " height: " << _screenInfoList[i]->myChannel->height << std::endl;
            }
        }

        int num = _pipeInfoList.size();
        ComController::instance()->sendSlaves(&num,sizeof(int));

        for(int i = 0; i < num; i++)
        {
            ComController::instance()->sendSlaves(_pipeInfoList[i],
                    sizeof(struct PipeInfo));
        }

        num = _windowInfoList.size();
        ComController::instance()->sendSlaves(&num,sizeof(int));

        for(int i = 0; i < num; i++)
        {
            ComController::instance()->sendSlaves(_windowInfoList[i],
                    sizeof(struct WindowInfo));
        }

        num = _channelInfoList.size();
        ComController::instance()->sendSlaves(&num,sizeof(int));

        for(int i = 0; i < num; i++)
        {
            ComController::instance()->sendSlaves(_channelInfoList[i],
                    sizeof(struct ChannelInfo));
            int stringsize = _channelInfoList[i]->stereoMode.size() + 1;
            //std::cerr << "Stringsize: " << stringsize << std::endl;
            char end = '\0';
            ComController::instance()->sendSlaves(&stringsize,sizeof(int));
            ComController::instance()->sendSlaves(
                    (void*)_channelInfoList[i]->stereoMode.c_str(),
                    _channelInfoList[i]->stereoMode.size());
            ComController::instance()->sendSlaves(&end,sizeof(char));
        }

        num = _screenInfoList.size();
        ComController::instance()->sendSlaves(&num,sizeof(int));

        for(int i = 0; i < num; i++)
        {
            ComController::instance()->sendSlaves(_screenInfoList[i],
                    sizeof(struct ScreenInfo));
        }
    }
    else
    {
        int num;
        ComController::instance()->readMaster(&num,sizeof(int));

        struct PipeInfo * pi;
        for(int i = 0; i < num; i++)
        {
            pi = new struct PipeInfo;
            ComController::instance()->readMaster(pi,sizeof(struct PipeInfo));
            _masterPipeInfoList.push_back(pi);
        }

        ComController::instance()->readMaster(&num,sizeof(int));

        struct WindowInfo * wi;
        for(int i = 0; i < num; i++)
        {
            wi = new struct WindowInfo;
            ComController::instance()->readMaster(wi,sizeof(struct WindowInfo));
            wi->myPipe = _masterPipeInfoList[wi->pipeIndex];
            wi->gc = NULL; // I would hope no one ever tries to use this
            _masterWindowInfoList.push_back(wi);
        }

        ComController::instance()->readMaster(&num,sizeof(int));

        struct ChannelInfo * ci;
        for(int i = 0; i < num; i++)
        {
            ci = new struct ChannelInfo;
            ComController::instance()->readMaster(ci,
                    sizeof(struct ChannelInfo));
            int stringsize;
            ComController::instance()->readMaster(&stringsize,sizeof(int));
            char * stereomode = new char[stringsize];
            ComController::instance()->readMaster(stereomode,stringsize);
            std::string tsm;
            memcpy(&ci->stereoMode,&tsm,sizeof(std::string));
            ci->stereoMode = stereomode;
            ci->myWindow = _masterWindowInfoList[ci->windowIndex];
            delete[] stereomode;
            _masterChannelInfoList.push_back(ci);
        }

        ComController::instance()->readMaster(&num,sizeof(int));

        struct ScreenInfo * si;
        for(int i = 0; i < num; i++)
        {
            si = new struct ScreenInfo;
            ComController::instance()->readMaster(si,sizeof(struct ScreenInfo));
            si->myChannel = _masterChannelInfoList[si->channelIndex];
            _masterScreenInfoList.push_back(si);
        }
    }
}
