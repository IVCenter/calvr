#include <kernel/PluginHelper.h>
#include <kernel/PluginManager.h>
#include <collaborative/CollaborativeManager.h>


using namespace cvr;

PluginHelper::PluginHelper()
{
}

PluginHelper::~PluginHelper()
{
}

osg::MatrixTransform * PluginHelper::getScene()
{
    return SceneManager::instance()->getScene();
}

osg::ClipNode * PluginHelper::getObjectsRoot()
{
    return SceneManager::instance()->getObjectsRoot();
}

const osg::MatrixTransform * PluginHelper::getObjectTransform()
{
    return SceneManager::instance()->getObjectTransform();
}

const osg::Matrix & PluginHelper::getObjectMatrix()
{
    return SceneManager::instance()->getObjectTransform()->getMatrix();
}

void PluginHelper::setObjectMatrix(osg::Matrix & mat)
{
    SceneManager::instance()->setObjectMatrix(mat);
}

float PluginHelper::getObjectScale()
{
    return SceneManager::instance()->getObjectScale();
}

void PluginHelper::setObjectScale(float scale)
{
    SceneManager::instance()->setObjectScale(scale);
}

const osg::Matrix & PluginHelper::getWorldToObjectTransform()
{
    return SceneManager::instance()->getWorldToObjectTransform();
}

const osg::Matrix & PluginHelper::getObjectToWorldTransform()
{
    return SceneManager::instance()->getObjectToWorldTransform();
}

int PluginHelper::getNumHands()
{
    return TrackingManager::instance()->getNumHands();
}

osg::Matrix & PluginHelper::getHandMat(int hand)
{
    return TrackingManager::instance()->getHandMat(hand);
}

int PluginHelper::getNumHeads()
{
    return TrackingManager::instance()->getNumHeads();
}

osg::Matrix & PluginHelper::getHeadMat(int head)
{
    return TrackingManager::instance()->getHeadMat(head);
}

int PluginHelper::getNumButtonStations()
{
    return TrackingManager::instance()->getNumButtonStations();
}

int PluginHelper::getNumButtons(int station)
{
    return TrackingManager::instance()->getNumButtons(station);
}

unsigned int PluginHelper::getRawButtonMask(int station)
{
    return TrackingManager::instance()->getRawButtonMask(station);
}

unsigned int PluginHelper::getHandButtonMask(int hand)
{
    return TrackingManager::instance()->getHandButtonMask(hand);
}

int PluginHelper::getNumValuatorStations()
{
    return TrackingManager::instance()->getNumValuatorStations();
}

int PluginHelper::getNumValuators(int station)
{
    return TrackingManager::instance()->getNumValuators(station);
}

float PluginHelper::getValuator(int station, int index)
{
    return TrackingManager::instance()->getValuator(station, index);
}

int PluginHelper::getMouseX()
{
    return InteractionManager::instance()->getMouseX();
}

int PluginHelper::getMouseY()
{
    return InteractionManager::instance()->getMouseY();
}

osg::Matrix & PluginHelper::getMouseMat()
{
    return InteractionManager::instance()->getMouseMat();
}

void PluginHelper::addRootMenuItem(MenuItem * item)
{
    MenuSystem::instance()->addMenuItem(item);
}

double PluginHelper::getLastFrameDuration()
{
    return CVRViewer::instance()->getLastFrameDuration();
}

double PluginHelper::getProgramDuration()
{
    return CVRViewer::instance()->getProgramDuration();
}

double PluginHelper::getFrameStartTime()
{
    return CVRViewer::instance()->getFrameStartTime();
}

double PluginHelper::getProgramStartTime()
{
    return CVRViewer::instance()->getProgramStartTime();
}

void PluginHelper::setClearColor(osg::Vec4 color)
{
    ScreenConfig::instance()->setClearColor(color);
}

int PluginHelper::getNumScreens()
{
    return ScreenConfig::instance()->getNumScreens();
}

ScreenInfo * PluginHelper::getScreenInfo(int screen)
{
    return ScreenConfig::instance()->getScreenInfo(screen);
}

void PluginHelper::registerSceneObject(SceneObject * object, std::string plugin)
{
    SceneManager::instance()->registerSceneObject(object,plugin);
}

void PluginHelper::unregisterSceneObject(SceneObject * object)
{
    SceneManager::instance()->unregisterSceneObject(object);
}

void PluginHelper::sendMessageByName(std::string plugin, int type, char * data)
{
    PluginManager::instance()->sendMessageByName(plugin,type,data);
}

void PluginHelper::sendCollaborativeMessageAsync(std::string plugin, int type, char * data, int size, bool sendLocal)
{
    CollaborativeManager::instance()->sendCollaborativeMessageAsync(plugin,type,data,size,sendLocal);
}

void PluginHelper::sendCollaborativeMessageSync(std::string plugin, int type, char * data, int size, bool sendLocal)
{
    CollaborativeManager::instance()->sendCollaborativeMessageSync(plugin,type,data,size,sendLocal);
}
