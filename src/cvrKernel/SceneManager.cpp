#include <cvrKernel/SceneManager.h>
#include <cvrInput/TrackingManager.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/ScreenConfig.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/SceneObject.h>
#include <cvrKernel/PluginManager.h>
#include <cvrUtil/OsgMath.h>

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include <iostream>
#include <list>
#include <queue>
#include <cassert>

using namespace cvr;

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

struct PrioritySort
{
        bool operator()(const std::pair<float,SceneObject*>& first
                , const std::pair<float,SceneObject*>& second)
        {
            return first.first > second.first;
        }
};

SceneManager * SceneManager::_myPtr = NULL;

SceneManager::SceneManager()
{
    _wallValid = false;
    _uniqueMapInUse = false;
}

SceneManager::~SceneManager()
{
}

SceneManager * SceneManager::instance()
{
    if(!_myPtr)
    {
        _myPtr = new SceneManager();
    }
    return _myPtr;
}

bool SceneManager::init()
{
    _actualRoot = new osg::Group();
    _sceneRoot = new osg::MatrixTransform();

    _depthPartitionLeft = new DepthPartitionNode();
    _depthPartitionLeft->setClearColorBuffer(false);
    _depthPartitionRight = new DepthPartitionNode();
    _depthPartitionRight->setClearColorBuffer(false);

    _objectTransform = new osg::MatrixTransform();
    _objectScale = new osg::MatrixTransform();
    _objectRoot = new osg::ClipNode();
    _menuRoot = new osg::MatrixTransform();

    _actualRoot->addChild(_depthPartitionLeft);
    _actualRoot->addChild(_depthPartitionRight);
    _depthPartitionLeft->addChild(_sceneRoot);
    _depthPartitionRight->addChild(_sceneRoot);
    _sceneRoot->addChild(_objectTransform);
    _sceneRoot->addChild(_menuRoot);
    _objectTransform->addChild(_objectScale);
    _objectScale->addChild(_objectRoot);

    bool dpart = ConfigManager::getBool("value",
            std::string("UseDepthPartition"),false);
    _depthPartitionLeft->setActive(dpart);
    _depthPartitionRight->setActive(dpart);

    _depthPartitionLeft->setNodeMask(
            _depthPartitionLeft->getNodeMask() & ~(CULL_MASK_RIGHT));
    _depthPartitionRight->setNodeMask(
            _depthPartitionRight->getNodeMask() & ~(CULL_MASK_LEFT));
    _depthPartitionRight->setNodeMask(
            _depthPartitionRight->getNodeMask() & ~(CULL_MASK));

    _depthPartitionRight->setForwardOtherTraversals(false);

    _scale = 1.0;
    _showAxis = false;
    _hidePointer = false;
    _menuOpenObject = NULL;

    _menuScale = ConfigManager::getFloat("value","ContextMenus.Scale",1.0);
    _menuMinDistance = ConfigManager::getFloat("value",
            "ContextMenus.MinDistance",750.0);
    _menuMaxDistance = ConfigManager::getFloat("value",
            "ContextMenus.MaxDistance",1500.0);
    _menuDefaultOpenButton = ConfigManager::getInt("value",
            "ContextMenus.DefaultOpenButton",1);


    _wallWidth = _wallHeight = 2000.0;

    std::string typestr = ConfigManager::getEntry("value","TiledWall.Type","PLANAR");
    if(typestr == "PLANAR")
    {
	_wallType = WT_PLANAR;
    }
    else
    {
	_wallType = WT_UNKNOWN;
    }

    switch(_wallType)
    {
	case WT_PLANAR:
	{
	    if(ConfigManager::getBool("autoDetect","TiledWall.Type",true))
	    {
		detectWallBounds();
	    }
	    else
	    {
		//TODO: manual wall description
	    }
	    _wallValid = true;
	}
	    break;
	default:
	    break;
    }
    //std::cerr << "WallWidth: " << _wallWidth << " WallHeight: " << _wallHeight << std::endl;

    initPointers();
    initLights();
    initSceneState();
    initAxis();

    bool b = ConfigManager::getBool("ShowAxis",false);

    setAxis(b);

    b = ConfigManager::getBool("HidePointer",false);
    setHidePointer(b);

    return true;
}

void SceneManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStatsAdvanced"))
    {
	stats = NULL;
    }

    if(stats)
    {
	startTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
    }

    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
	switch(TrackingManager::instance()->getPointerGraphicType(i))
        {
	    case POINTER:
	    {
		osg::Vec3 intersect;
		if(getPointOnTiledWall(TrackingManager::instance()->getHandMat(i),intersect))
		{
		    //TODO add wall rotation to pointer
		    osg::Matrix m;
		    m.makeTranslate(intersect);
		    _handTransforms[i]->setMatrix(m);
		}
		break;
	    }
	    default:
		_handTransforms[i]->setMatrix(
			TrackingManager::instance()->getHandMat(i));
		break;
	}
    }

    if(_showAxis)
    {
        for(int i = 0; i < _headAxisTransforms.size(); i++)
        {
            _headAxisTransforms[i]->setMatrix(
                    TrackingManager::instance()->getUnfrozenHeadMat(i));
        }
    }

    updateActiveObject();

    if(stats)
    {
        endTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Scene begin time", startTime);
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Scene end time", endTime);
        stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "Scene time taken", endTime-startTime);
    }
}

void SceneManager::postEventUpdate()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    for(std::map<int,SceneObject*>::iterator it = _activeObjects.begin();
            it != _activeObjects.end(); it++)
    {
        if(it->second)
        {
            it->second->moveCleanup();
	    SceneObject * object = it->second;
	    while(object)
	    {
		object->updateCallback(it->first,
			TrackingManager::instance()->getHandMat(it->first));
		object = object->_parent;
	    }
        }
    }
}

osg::MatrixTransform * SceneManager::getScene()
{
    return _sceneRoot.get();
}

osg::ClipNode * SceneManager::getObjectsRoot()
{
    return _objectRoot.get();
}

const osg::MatrixTransform * SceneManager::getObjectTransform()
{
    return _objectTransform.get();
}

void SceneManager::setObjectMatrix(osg::Matrix & mat)
{
    _objectTransform->setMatrix(mat);

    _obj2world = _objectScale->getMatrix() * _objectTransform->getMatrix();
    _world2obj = osg::Matrix::inverse(_obj2world);
}

double SceneManager::getObjectScale()
{
    return _scale;
}

void SceneManager::setObjectScale(double scale)
{
    _scale = scale;

    osg::Matrixd m;
    m.makeScale(osg::Vec3d(_scale,_scale,_scale));

    _objectScale->setMatrix(m);

    _obj2world = _objectScale->getMatrix() * _objectTransform->getMatrix();
    _world2obj = osg::Matrix::inverse(_obj2world);
}

const osg::Matrix & SceneManager::getWorldToObjectTransform()
{
    return _world2obj;
}

const osg::Matrix & SceneManager::getObjectToWorldTransform()
{
    return _obj2world;
}

osg::MatrixTransform * SceneManager::getMenuRoot()
{
    return _menuRoot.get();
}

void SceneManager::setAxis(bool on)
{
    if(on == _showAxis)
    {
        return;
    }

    if(on)
    {
        _objectRoot->addChild(_axisNode.get());
        _sceneRoot->addChild(_axisNode.get());

        for(int i = 0; i < _handTransforms.size(); i++)
        {
            _handTransforms[i]->addChild(_axisNode.get());
        }

        for(int i = 0; i < _headAxisTransforms.size(); i++)
        {
            _sceneRoot->addChild(_headAxisTransforms[i]);
        }

        _showAxis = true;
    }
    else
    {
        _objectRoot->removeChild(_axisNode.get());
        _sceneRoot->removeChild(_axisNode.get());

        for(int i = 0; i < _handTransforms.size(); i++)
        {
            _handTransforms[i]->removeChild(_axisNode.get());
        }

        for(int i = 0; i < _headAxisTransforms.size(); i++)
        {
            _sceneRoot->removeChild(_headAxisTransforms[i]);
        }

        _showAxis = false;
    }
}

void SceneManager::setHidePointer(bool b)
{
    if(b == _hidePointer)
    {
        return;
    }

    if(b)
    {
        for(int i = 0; i < _handTransforms.size(); i++)
        {
            _sceneRoot->removeChild(_handTransforms[i]);
        }
    }
    else
    {
        for(int i = 0; i < _handTransforms.size(); i++)
        {
            _sceneRoot->addChild(_handTransforms[i]);
        }
    }

    _hidePointer = b;
}

DepthPartitionNode * SceneManager::getDepthPartitionNodeLeft()
{
    return _depthPartitionLeft.get();
}

DepthPartitionNode * SceneManager::getDepthPartitionNodeRight()
{
    return _depthPartitionRight.get();
}

void SceneManager::setDepthPartitionActive(bool active)
{
    _depthPartitionLeft->setActive(active);
    _depthPartitionRight->setActive(active);
}

bool SceneManager::getDepthPartitionActive()
{
    return _depthPartitionLeft->getActive();
}

void SceneManager::setViewerScene(CVRViewer * cvrviewer)
{
    cvrviewer->setSceneData(_actualRoot);
}

bool SceneManager::processEvent(InteractionEvent * ie)
{
    int hand = -2;
    int button = -1;

    if(ie->asTrackedButtonEvent())
    {
        hand = ie->asTrackedButtonEvent()->getHand();
        button = ie->asTrackedButtonEvent()->getButton();
    }
    else
    {
	_uniqueMapInUse = true;
	for(std::map<SceneObject*,int>::iterator it =
		_uniqueActiveObjects.begin(); it != _uniqueActiveObjects.end();
		it++)
	{
	    if(_uniqueBlacklistMap.find(it->first) != _uniqueBlacklistMap.end())
	    {
		continue;
	    }

	    if(it->first->processEvent(ie))
	    {
		_uniqueMapInUse = false;

		for(std::map<SceneObject*,bool>::iterator it = _uniqueBlacklistMap.begin(); it != _uniqueBlacklistMap.end(); it++)
		{
		    _uniqueActiveObjects.erase(it->first);
		}
		_uniqueBlacklistMap.clear();

		return true;
	    }
	}

	for(std::map<SceneObject*,bool>::iterator it = _uniqueBlacklistMap.begin(); it != _uniqueBlacklistMap.end(); it++)
	{
	    _uniqueActiveObjects.erase(it->first);
	}
	_uniqueBlacklistMap.clear();

	_uniqueMapInUse = false;
	return false;
    }

    if(hand == -2)
    {
        return false;
    }

    if(_activeObjects[hand])
    {
        return _activeObjects[hand]->processEvent(ie);
    }
    else if(_menuOpenObject)
    {
        if(ie->getInteraction() == BUTTON_DOWN && _menuOpenObject->_menuButton)
        {
            return _menuOpenObject->processEvent(ie);
        }
    }
    return false;
}

void SceneManager::registerSceneObject(SceneObject * object, std::string plugin)
{
    if(object->_parent)
    {
        std::cerr << "SceneManager: error: trying to register SceneObject "
                << object->getName() << ", which is a child object."
                << std::endl;
        return;
    }

    if(_pluginObjectMap.find(plugin) == _pluginObjectMap.end())
    {
        _pluginObjectMap[plugin] = std::vector<SceneObject*>();
    }

    _pluginObjectMap[plugin].push_back(object);
    object->setRegistered(true);
}

void SceneManager::unregisterSceneObject(SceneObject * object)
{
    for(std::map<std::string,std::vector<SceneObject*> >::iterator it =
            _pluginObjectMap.begin(); it != _pluginObjectMap.end(); it++)
    {
        for(std::vector<SceneObject*>::iterator it2 = it->second.begin();
                it2 != it->second.end(); it2++)
        {
            if((*it2) == object)
            {
                // detach
                (*it2)->detachFromScene();
                it->second.erase(it2);
                for(std::map<int,SceneObject*>::iterator aobjit =
                        _activeObjects.begin(); aobjit != _activeObjects.end();
                        aobjit++)
                {
		    SceneObject * aoRoot = aobjit->second;
		    while(aoRoot && aoRoot->_parent)
		    {
			aoRoot = aoRoot->_parent;
		    }

                    if(aoRoot == object)
                    {
                        aobjit->second = NULL;
                    }
                }

		// close menu if needed
		SceneObject * menuSORoot = _menuOpenObject;
		while(menuSORoot && menuSORoot->_parent)
		{
		    menuSORoot = menuSORoot->_parent;
		}

		if(menuSORoot == object)
		{
		    closeOpenObjectMenu();
		}

		// if this happened during the SceneManager's processEvent, don't invalidate iterator
		if(!_uniqueMapInUse)
		{
		    _uniqueActiveObjects.erase(object);
		}
		else
		{
		    _uniqueBlacklistMap[object] = true;
		}

                object->setRegistered(false);
                return;
            }
        }
    }
}

std::vector< SceneObject* >
SceneManager::getSceneObjects(void)
{
    std::vector< SceneObject* > scene_objects;

    for (std::map< std::string, std::vector< SceneObject* > >::iterator it = _pluginObjectMap.begin();
        _pluginObjectMap.end() != it;
        ++it)
    {
        scene_objects.insert( scene_objects.end(), it->second.begin(), it->second.end() );
    }

    return scene_objects;
}

void SceneManager::setMenuOpenObject(SceneObject * object)
{
    if(object != _menuOpenObject)
    {
        closeOpenObjectMenu();
    }

    _menuOpenObject = object;

}

SceneObject * SceneManager::getMenuOpenObject()
{
    return _menuOpenObject;
}

void SceneManager::closeOpenObjectMenu()
{
    if(_menuOpenObject)
    {
        _menuOpenObject->closeMenu();
        _menuOpenObject = NULL;
    }
}

SceneManager::CameraCallbacks * SceneManager::getCameraCallbacks(osg::Camera * cam)
{
    if(_callbackMap.find(cam) != _callbackMap.end())
    {
	return &_callbackMap[cam];
    }
    else
    {
	return NULL;
    }
}

bool SceneManager::getPointOnTiledWall(const osg::Matrix & mat, osg::Vec3 & wallPoint)
{
    osg::Vec3 linePoint1(0,0,0), linePoint2(0,1,0);
    linePoint1 = linePoint1 * mat;
    linePoint2 = linePoint2 * mat;
    osg::Vec3 planePoint(0,0,0),planeNormal(0,-1,0);
    planePoint = planePoint * _wallTransform;
    planeNormal = planeNormal * _wallTransform;
    float w;
    return linePlaneIntersectionRef(linePoint1,linePoint2,planePoint,planeNormal,wallPoint,w);
}

void SceneManager::initPointers()
{
    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
        _handTransforms.push_back(new osg::MatrixTransform());
        _sceneRoot->addChild(_handTransforms[i]);
        _handTransforms[i]->setMatrix(
                TrackingManager::instance()->getHandMat(i));

        switch(TrackingManager::instance()->getPointerGraphicType(i))
        {
            case CONE:
            {
                osg::Cone * cone = new osg::Cone(osg::Vec3(0,500,0),10,2000);
                osg::Quat q = osg::Quat(-M_PI / 2.0,osg::Vec3(1.0,0,0));
                cone->setRotation(q);
                osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
                osg::Geode * geode = new osg::Geode();
                geode->addDrawable(sd);
                geode->setNodeMask(geode->getNodeMask() & ~INTERSECT_MASK);
                _handTransforms[i]->addChild(geode);
                break;
            }
	    case POINTER:
	    {
		/*float width = 150.0;
		float height = -150.0;
		osg::Vec4 color(1.0,1.0,1.0,1.0);
		osg::Vec3 pos(0,0,0);

		osg::Geometry * geo = new osg::Geometry();
		osg::Vec3Array* verts = new osg::Vec3Array();
		verts->push_back(pos);
		verts->push_back(pos + osg::Vec3(width,0,0));
		verts->push_back(pos + osg::Vec3(width,0,height));
		verts->push_back(pos + osg::Vec3(0,0,height));

		geo->setVertexArray(verts);

		osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
			osg::PrimitiveSet::QUADS,0);

		ele->push_back(0);
		ele->push_back(1);
		ele->push_back(2);
		ele->push_back(3);
		geo->addPrimitiveSet(ele);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(color);

		osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4> *colorIndexArray;
		colorIndexArray = new osg::TemplateIndexArray<unsigned int,
				osg::Array::UIntArrayType,4,4>;
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);

		geo->setColorArray(colors);
		geo->setColorIndices(colorIndexArray);
		geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::Vec2Array* texcoords = new osg::Vec2Array;
		texcoords->push_back(osg::Vec2(0,1));
		texcoords->push_back(osg::Vec2(1,1));
		texcoords->push_back(osg::Vec2(1,0));
		texcoords->push_back(osg::Vec2(0,0));
		geo->setTexCoordArray(0,texcoords);

		osg::Geode * geode = new osg::Geode();
                geode->addDrawable(geo);
                geode->setNodeMask(geode->getNodeMask() & ~INTERSECT_MASK);
                _handTransforms[i]->addChild(geode);

		osg::StateSet * stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		osg::Image * image = osgDB::readImageFile(CalVR::instance()->getHomeDir() + "/icons/mousePointer.png");
		if(image)
		{
		    osg::Texture2D* texture;
		    texture = new osg::Texture2D;
		    texture->setImage(image);

		    texture->setResizeNonPowerOfTwoHint(false);

		    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
		}*/

		float minDem = std::min(_wallWidth,_wallHeight);
		float size = minDem * 0.05;
		size = std::min(size,200.0f);

		float scaleX = size;
		float scaleZ = size;
		osg::Vec4 color(0.0,1.0,0.0,0.75);

		osg::Geometry * geo = new osg::Geometry();
		osg::Vec3Array* verts = new osg::Vec3Array();
		verts->push_back(osg::Vec3(0.7*scaleX,0,-0.5*scaleZ));
		verts->push_back(osg::Vec3(0.0*scaleX,0,0.0*scaleZ));
		verts->push_back(osg::Vec3(0.4*scaleX,0,-0.5*scaleZ));
		verts->push_back(osg::Vec3(0.25*scaleX,0,-0.75*scaleZ));

		geo->setVertexArray(verts);

		osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
			osg::PrimitiveSet::TRIANGLE_STRIP,0);

		ele->push_back(0);
		ele->push_back(1);
		ele->push_back(2);
		ele->push_back(3);
		geo->addPrimitiveSet(ele);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(color);

		osg::TemplateIndexArray<unsigned int,osg::Array::UIntArrayType,4,4> *colorIndexArray;
		colorIndexArray = new osg::TemplateIndexArray<unsigned int,
				osg::Array::UIntArrayType,4,4>;
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);
		colorIndexArray->push_back(0);

		geo->setColorArray(colors);
		geo->setColorIndices(colorIndexArray);
		geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		osg::Geode * geode = new osg::Geode();
                geode->addDrawable(geo);
                geode->setNodeMask(geode->getNodeMask() & ~INTERSECT_MASK);
                _handTransforms[i]->addChild(geode);

		osg::StateSet * stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		break;
	    }
            case NONE:
                break;
            default:
                break;
        }
    }
}

void SceneManager::initLights()
{
    osg::StateSet * stateset = _sceneRoot->getOrCreateStateSet();

    //TODO: replace with lighting manager

    //read any values from config file
    osg::Vec4 diffuse, specular, ambient, position;
    osg::Vec3 direction;
    float spotexp, spotcutoff;

    diffuse = ConfigManager::getColor("Light.Diffuse",
            osg::Vec4(1.0,1.0,1.0,1.0));
    specular = ConfigManager::getColor("Light.Specular",osg::Vec4(0,0,0,1.0));
    ambient = ConfigManager::getColor("Light.Ambient",
            osg::Vec4(0.3,0.3,0.3,1.0));
    position = ConfigManager::getVec4("Light.Position",
            osg::Vec4(0.0,-10000.0,10000.0,1.0));
    direction = ConfigManager::getVec3("Light.Direction",osg::Vec3(0,0,-1));
    spotexp = ConfigManager::getFloat("Light.SpotExponent",0);
    spotcutoff = ConfigManager::getFloat("Light.SpotCutoff",180.0);

    osg::LightSource * source = new osg::LightSource();
    osg::Light * light = new osg::Light(0);

    /*light->setDiffuse(osg::Vec4(1.0, 1.0, 1.0, 1.0));
     light->setSpecular(osg::Vec4(0, 0, 0, 1.0));
     light->setAmbient(osg::Vec4(0.3, 0.3, 0.3, 1.0));
     light->setPosition(osg::Vec4(0.0, -10000.0, 10000.0, 1.0));
     light->setDirection(osg::Vec3(0, 0, -1));
     //light->setDirection(osg::Vec3(0, 0.707106781, -0.707106781));
     light->setSpotExponent(0);
     light->setSpotCutoff(180.0);*/

    light->setDiffuse(diffuse);
    light->setSpecular(specular);
    light->setAmbient(ambient);
    light->setPosition(position);
    light->setDirection(direction);
    light->setSpotExponent(spotexp);
    light->setSpotCutoff(spotcutoff);

    source->setLight(light);
    _sceneRoot->addChild(source);

    //stateset->setAttributeAndModes(light, osg::StateAttribute::ON);
    source->setLocalStateSetModes(osg::StateAttribute::ON);
    source->setStateSetModes(*stateset,osg::StateAttribute::ON);

    if(_handTransforms.size() > 0 && false)
    {
        stateset = _handTransforms[0]->getOrCreateStateSet();

        source = new osg::LightSource();
        light = new osg::Light(1);
        //light->setDiffuse(osg::Vec4(1.0,1.0,1.0,1.0));
        //light->setSpecular(osg::Vec4(1.0,1.0,1.0,1.0));
        //light->setAmbient(osg::Vec4(0.3,0.3,0.3,1.0));
        //light->setPosition(osg::Vec4(0.0,-10000.0,10000.0,1.0));
        source->setLight(light);
        _handTransforms[0]->addChild(source);

        stateset->setAttributeAndModes(light,osg::StateAttribute::ON);
        source->setLocalStateSetModes(osg::StateAttribute::ON);
        source->setStateSetModes(*stateset,osg::StateAttribute::ON);
    }
}

void SceneManager::initSceneState()
{
    osg::StateSet * stateset = _sceneRoot->getOrCreateStateSet();

    osg::LightModel * lm = new osg::LightModel();
    lm->setTwoSided(true);
    lm->setLocalViewer(true);

    osg::Material * mat = new osg::Material();
    mat->setDiffuse(osg::Material::FRONT_AND_BACK,
            osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    mat->setAmbient(osg::Material::FRONT_AND_BACK,
            osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    mat->setSpecular(osg::Material::FRONT_AND_BACK,
            osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    mat->setAlpha(osg::Material::FRONT_AND_BACK,1.0f);
    mat->setColorMode(osg::Material::DIFFUSE);

    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    stateset->setAttributeAndModes(mat,osg::StateAttribute::ON);
    stateset->setAttributeAndModes(lm,osg::StateAttribute::ON);
    stateset->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
}

void SceneManager::initAxis()
{
    _axisNode = new osg::Group();

    osg::Cone * cone = new osg::Cone(osg::Vec3(0,0,0),15,32);
    osg::Quat q = osg::Quat(-M_PI / 2.0,osg::Vec3(1.0,0,0));
    cone->setRotation(q);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(sd);
    osg::MatrixTransform * mt = new osg::MatrixTransform();
    osg::Matrix m;
    m.makeTranslate(osg::Vec3(0,208,0));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    cone = new osg::Cone(osg::Vec3(0,0,0),15,32);
    q = osg::Quat(M_PI / 2.0,osg::Vec3(0,1.0,0));
    cone->setRotation(q);
    sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(1.0,0.0,0.0,1.0));
    geode = new osg::Geode();
    geode->addDrawable(sd);
    mt = new osg::MatrixTransform();
    m.makeTranslate(osg::Vec3(208,0,0));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    cone = new osg::Cone(osg::Vec3(0,0,0),15,32);
    sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(0.0,0.0,1.0,1.0));
    geode = new osg::Geode();
    geode->addDrawable(sd);
    mt = new osg::MatrixTransform();
    m.makeTranslate(osg::Vec3(0,0,208));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    geode = new osg::Geode();

    osg::Cylinder * cylinder = new osg::Cylinder(osg::Vec3(0,0,100),10,200);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(0.0,0.0,1.0,1.0));
    geode->addDrawable(sd);

    cylinder = new osg::Cylinder(osg::Vec3(100,0,0),10,200);
    q = osg::Quat(M_PI / 2.0,osg::Vec3(0,1.0,0));
    cylinder->setRotation(q);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(1.0,0.0,0.0,1.0));
    geode->addDrawable(sd);

    cylinder = new osg::Cylinder(osg::Vec3(0,100,0),10,200);
    q = osg::Quat(-M_PI / 2.0,osg::Vec3(1.0,0,0));
    cylinder->setRotation(q);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
    geode->addDrawable(sd);

    _axisNode->addChild(geode);

    for(int i = 0; i < TrackingManager::instance()->getNumHeads(); i++)
    {
        _headAxisTransforms.push_back(new osg::MatrixTransform());
        _headAxisTransforms[i]->addChild(_axisNode.get());
    }

    //_objectRoot->addChild(_axisNode);
}

void SceneManager::detectWallBounds()
{
    unsigned int const FLOATS_FOR_CORNERS = 12;
    osg::Vec3 final_tl, final_bl, final_tr, final_br;

    if (ComController::instance()->getNumSlaves() > 0)
    {
	// NOTE:  we are assuming master node IS head node
	// NODES -> MASTER passing 4 world corners of screen
	if (ComController::instance()->isMaster())
	{
	    int num_slaves = ComController::instance()->getNumSlaves();

	    unsigned int num_corners = num_slaves * FLOATS_FOR_CORNERS;
	    float* msgs_back = new float[num_corners];
	    if(!cvr::ComController::instance()->readSlaves(msgs_back, sizeof(float) *
			FLOATS_FOR_CORNERS) )
	    {
		delete[] msgs_back;
		return;
	    }
	    std::vector<float> corners(msgs_back, msgs_back + num_corners);
	    updateToExtremeCorners(corners);
	    delete[] msgs_back;

	    if(!cvr::ComController::instance()->sendSlaves(corners.data(),
			sizeof(float)*corners.size()) )
	    {
		return;
	    }

	    final_bl = osg::Vec3(corners[ 0], corners[ 1], corners[ 2]);
	    final_br = osg::Vec3(corners[ 3], corners[ 4], corners[ 5]);
	    final_tr = osg::Vec3(corners[ 6], corners[ 7], corners[ 8]);
	    final_tl = osg::Vec3(corners[ 9], corners[10], corners[11]);
	}
	else // Send to master
	{
	    osg::Vec3 world_tl, world_bl, world_tr, world_br;

	    std::vector<float> corners;

	    getNodeWorldCorners(corners);

	    if(!cvr::ComController::instance()->sendMaster(corners.data(),
			sizeof(float)*corners.size()) )
	    {
		return;
	    }

	    // Receive final extremes from master
	    float msgs_back[FLOATS_FOR_CORNERS];
	    if(!cvr::ComController::instance()->readMaster(msgs_back,
			sizeof(float) * FLOATS_FOR_CORNERS) )
	    {
		return;
	    }

	    final_bl = osg::Vec3(msgs_back[ 0], msgs_back[ 1], msgs_back[ 2]);
	    final_br = osg::Vec3(msgs_back[ 3], msgs_back[ 4], msgs_back[ 5]);
	    final_tr = osg::Vec3(msgs_back[ 6], msgs_back[ 7], msgs_back[ 8]);
	    final_tl = osg::Vec3(msgs_back[ 9], msgs_back[10], msgs_back[11]);

	    unsigned int j = 0;
	}
    }
    else // Make wall bounds the size of the window
    {
	std::vector<float> corners;

	getNodeWorldCorners(corners);

	final_bl = osg::Vec3(corners[ 0], corners[ 1], corners[ 2]);
	final_br = osg::Vec3(corners[ 3], corners[ 4], corners[ 5]);
	final_tr = osg::Vec3(corners[ 6], corners[ 7], corners[ 8]);
	final_tl = osg::Vec3(corners[ 9], corners[10], corners[11]);
    }

    osg::Vec3 wallCenter((final_bl.x()+final_tr.x())/2.0,final_tr.y(),(final_bl.z()+final_tr.z())/2.0);
    _wallTransform.makeTranslate(wallCenter);

    /*mWorldBounds.worldXMin = final_bl.x();
    mWorldBounds.worldXMax = final_tr.x();
    mWorldBounds.worldZMin = final_bl.z();
    mWorldBounds.worldZMax = final_tr.z();
    mWorldBounds.worldY  = final_tr.y();*/

    _wallWidth  = (final_tr.x() - final_bl.x());
    _wallHeight = (final_tr.z() - final_bl.z());
}

void SceneManager::updateToExtremeCorners(std::vector<float>& corners)
{
    unsigned int const FLOATS_FOR_CORNERS = 12;
    unsigned int num_corners = corners.size();
    assert( num_corners % FLOATS_FOR_CORNERS == 0 );

    osg::Vec3 bl, br, tr, tl;

    for (unsigned int i = 0; i < num_corners; i += FLOATS_FOR_CORNERS)
    {
	if (0 == i)
	{
	    bl = osg::Vec3(corners[i + 0], corners[i +  1], corners[i +  2]);
	    br = osg::Vec3(corners[i + 3], corners[i +  4], corners[i +  5]);
	    tr = osg::Vec3(corners[i + 6], corners[i +  7], corners[i +  8]);
	    tl = osg::Vec3(corners[i + 9], corners[i + 10], corners[i + 11]);
	}
	else // check for extremes
	{
	    if (corners[i + 0] < bl.x() || corners[i +  2] < bl.z())
		bl = osg::Vec3(corners[i + 0], corners[i +  1],
			corners[i +  2]);

	    if (corners[i + 3] > br.x() || corners[i + 5] < br.z())
		br = osg::Vec3(corners[i + 3], corners[i +  4],
			corners[i +  5]);

	    if (corners[i + 6] > tr.x() || corners[i + 8] > tr.z())
		tr = osg::Vec3(corners[i + 6], corners[i +  7],
			corners[i +  8]);

	    if (corners[i + 9] < tl.x() || corners[i + 11] > tl.z())
		tl = osg::Vec3(corners[i + 9], corners[i + 10],
			corners[i + 11]);
	}
    }

    corners.clear();

    corners.push_back(bl.x());
    corners.push_back(bl.y());
    corners.push_back(bl.z());
    corners.push_back(br.x());
    corners.push_back(br.y());
    corners.push_back(br.z());
    corners.push_back(tr.x());
    corners.push_back(tr.y());
    corners.push_back(tr.z());
    corners.push_back(tl.x());
    corners.push_back(tl.y());
    corners.push_back(tl.z());
}

void SceneManager::getNodeWorldCorners(std::vector<float>& corners)
{
    int num_screens = ScreenConfig::instance()->getNumScreens();
    for (int i = 0; i < num_screens; ++i)
    {
	//        osg::Vec3 center = cvr::PluginHelper::getScreenInfo(i)->xyz;
	float width  = ScreenConfig::instance()->getScreenInfo(i)->width;
	float height = ScreenConfig::instance()->getScreenInfo(i)->height;
	osg::Matrix screen_to_world_xfrm =
	    ScreenConfig::instance()->getScreenInfo(i)->transform;

	//        std::cerr << "WxH = " << width << " x " << height << std::endl;

	osg::Vec3 tl, bl, tr, br;
	bl = osg::Vec3( -width/2.f,  0.f, -height/2.f ) * screen_to_world_xfrm;
	br = osg::Vec3(  width/2.f,  0.f, -height/2.f ) * screen_to_world_xfrm;
	tr = osg::Vec3(  width/2.f,  0.f,  height/2.f ) * screen_to_world_xfrm;
	tl = osg::Vec3( -width/2.f,  0.f,  height/2.f ) * screen_to_world_xfrm;

	corners.push_back(bl.x());
	corners.push_back(bl.y());
	corners.push_back(bl.z());
	corners.push_back(br.x());
	corners.push_back(br.y());
	corners.push_back(br.z());
	corners.push_back(tr.x());
	corners.push_back(tr.y());
	corners.push_back(tr.z());
	corners.push_back(tl.x());
	corners.push_back(tl.y());
	corners.push_back(tl.z());
    }

    updateToExtremeCorners(corners);
}

void SceneManager::updateActiveObject()
{
    osg::Vec3 start, end;

    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
        int hand;
        osg::Matrix handMatrix;
        hand = i;
        handMatrix = TrackingManager::instance()->getHandMat(i);

        if(_activeObjects[hand])
        {
            if(_activeObjects[hand]->getEventActive()
                    && _activeObjects[hand]->getActiveHand() == hand)
            {
                // recalc parent bounding box during movement;
                if(_activeObjects[hand]->_moving)
                {
                    SceneObject * root = _activeObjects[hand];
                    while(root->_parent)
                    {
                        root = root->_parent;
                    }
                    root->getOrComputeBoundingBox();
                }
                continue;
            }
        }

        start = osg::Vec3(0,0,0);
        end = osg::Vec3(0,10000,0);
        start = start * handMatrix;
        end = end * handMatrix;

        std::list<SceneObject*> hitList;

        // Find list of all ojects that pass bounding sphere intersection
        for(std::map<std::string,std::vector<SceneObject*> >::iterator it =
                _pluginObjectMap.begin(); it != _pluginObjectMap.end(); it++)
        {
            for(int j = 0; j < it->second.size(); j++)
            {
                if(it->second[j]->intersectsFast(start,end))
                {
                    hitList.push_back(it->second[j]);
                }
            }
        }

        if(TrackingManager::instance()->getHandTrackerType(hand)
                == TrackerBase::MOUSE)
        {
            if(!InteractionManager::instance()->mouseActive())
            {
                continue;
            }
        }

        //std::cerr << "hand: " << hand << " listsize: " << hitList.size() << std::endl;

        osg::Vec3 isec1, isec2;
        bool neg1, neg2;
        std::priority_queue<std::pair<float,SceneObject*>
                , std::vector<std::pair<float,SceneObject*> >, PrioritySort> sortQueue;

        // find points of bounding box intersection
        for(std::list<SceneObject*>::iterator objit = hitList.begin();
                objit != hitList.end(); objit++)
        {
            //std::cerr << "Object " << (*objit)->getName() << std::endl;
            if((*objit)->intersects(start,end,isec1,neg1,isec2,neg2))
            {
                if(neg1)
                {
                    //std::cerr << "n1: " << -(isec1-start).length() << std::endl;
                    sortQueue.push(
                            std::pair<float,SceneObject*>(
                                    -(isec1 - start).length(),(*objit)));
                }
                else
                {
                    //std::cerr << "1: " << (isec1-start).length() << std::endl;
                    sortQueue.push(
                            std::pair<float,SceneObject*>(
                                    (isec1 - start).length(),(*objit)));
                }

                if(neg2)
                {
                    //std::cerr << "n2: " << -(isec2-start).length() << std::endl;
                    sortQueue.push(
                            std::pair<float,SceneObject*>(
                                    -(isec2 - start).length(),(*objit)));
                }
                else
                {
                    //std::cerr << "2: " << (isec2-start).length() << std::endl;
                    sortQueue.push(
                            std::pair<float,SceneObject*>(
                                    (isec2 - start).length(),(*objit)));
                }
            }
        }

        //std::cerr << "sortqueue size: " << sortQueue.size() << std::endl;

        std::map<SceneObject*,int> countMap;
        SceneObject * currentObject = NULL;
        while(sortQueue.size())
        {
            //std::cerr << "dist: " << sortQueue.top().first << std::endl;
            currentObject = sortQueue.top().second;
            countMap[currentObject]++;
            if(countMap[currentObject] == 2)
            {
                break;
            }
            sortQueue.pop();
        }

        if(currentObject)
        {
            currentObject = findChildActiveObject(currentObject,start,end);
            if(_activeObjects[hand] != currentObject)
            {
		std::list<SceneObject*> lastObjList;
		std::list<SceneObject*> currentObjList;
		SceneObject * object = _activeObjects[hand];
		while(object)
		{
		    lastObjList.push_front(object);
		    object = object->_parent;
		}
		object = currentObject;
		while(object)
		{
		    currentObjList.push_front(object);
		    object = object->_parent;
		}

		std::list<SceneObject*>::iterator lastIt = lastObjList.begin();
		std::list<SceneObject*>::iterator curIt = currentObjList.begin();
		while(lastIt != lastObjList.end() && curIt != currentObjList.end())
		{
		    if(*lastIt != *curIt)
		    {
			break;
		    }
		    
		    lastIt++;
		    curIt++;    
		}

		if(lastIt != lastObjList.end())
		{
		    for(std::list<SceneObject*>::reverse_iterator it = lastObjList.rbegin(); ; it++)
		    {
			(*it)->leaveCallback(hand);
			if(*it == *lastIt)
			{
			    break;
			}
		    }
		}

		if(curIt != currentObjList.end())
		{
		    for(std::list<SceneObject*>::iterator it = curIt; it != currentObjList.end() ; it++)
		    {
			(*it)->enterCallback(hand,handMatrix);
		    }
		}

                if(_activeObjects[hand])
                {
                    _activeObjects[hand]->interactionCountDec();
                }
                _activeObjects[hand] = currentObject;
                currentObject->interactionCountInc();
            }
        }
        else if(_activeObjects[hand])
        {
            _activeObjects[hand]->interactionCountDec();
	    SceneObject * object = _activeObjects[hand];
	    while(object)
	    {
		object->leaveCallback(hand);
		object = object->_parent;
	    }
            _activeObjects[hand] = NULL;
        }
    }

    _uniqueActiveObjects.clear();
    for(std::map<int,SceneObject*>::iterator it = _activeObjects.begin();
            it != _activeObjects.end(); it++)
    {
        if(it->second)
        {
            _uniqueActiveObjects[it->second]++;
        }
    }
}

SceneObject * SceneManager::findChildActiveObject(SceneObject * object,
        osg::Vec3 & start, osg::Vec3 & end)
{
    std::list<SceneObject*> hitList;

    for(int i = 0; i < object->getNumChildObjects(); i++)
    {
        if(object->getChildObject(i)->intersectsFast(start,end))
        {
            hitList.push_back(object->getChildObject(i));
        }
    }

    //std::cerr << "Nested: hitlist size: " << hitList.size() << std::endl;

    osg::Vec3 isec1, isec2;
    bool neg1, neg2;
    std::priority_queue<std::pair<float,SceneObject*>
            , std::vector<std::pair<float,SceneObject*> >, PrioritySort> sortQueue;

    // find points of bounding box intersection
    for(std::list<SceneObject*>::iterator objit = hitList.begin();
            objit != hitList.end(); objit++)
    {
        if((*objit)->intersects(start,end,isec1,neg1,isec2,neg2))
        {
            if(neg1)
            {
                sortQueue.push(
                        std::pair<float,SceneObject*>(-(isec1 - start).length(),
                                (*objit)));
            }
            else
            {
                sortQueue.push(
                        std::pair<float,SceneObject*>((isec1 - start).length(),
                                (*objit)));
            }

            if(neg2)
            {
                sortQueue.push(
                        std::pair<float,SceneObject*>(-(isec2 - start).length(),
                                (*objit)));
            }
            else
            {
                sortQueue.push(
                        std::pair<float,SceneObject*>((isec2 - start).length(),
                                (*objit)));
            }
        }
    }

    std::map<SceneObject*,int> countMap;
    SceneObject * currentObject = NULL;
    while(sortQueue.size())
    {
        currentObject = sortQueue.top().second;
        countMap[currentObject]++;
        if(countMap[currentObject] == 2)
        {
            break;
        }
        sortQueue.pop();
    }

    if(currentObject)
    {
        return findChildActiveObject(currentObject,start,end);
    }

    return object;
}

void SceneManager::removeNestedObject(SceneObject * object)
{
    for(std::map<int,SceneObject*>::iterator it = _activeObjects.begin(); it != _activeObjects.end(); ++it)
    {
	SceneObject * so = it->second;
	while(so)
	{
	    if(so == object)
	    {
		it->second = so->_parent;
		break;
	    }
	    so = so->_parent;
	}
    }

    if(_uniqueActiveObjects.find(object) != _uniqueActiveObjects.end())
    {
	if(!_uniqueMapInUse)
	{
	    _uniqueActiveObjects.erase(object);
	}
	else
	{
	    _uniqueBlacklistMap[object] = true;
	}
    }

    // close menu if this node is in its path
    SceneObject * menuSO = _menuOpenObject;
    while(menuSO)
    {
	if(menuSO == object)
	{
	    closeOpenObjectMenu();
	}

	menuSO = menuSO->_parent;
    }
}

void SceneManager::removePluginObjects(CVRPlugin * plugin)
{
    std::string pluginName = PluginManager::instance()->getPluginName(plugin);
    if(pluginName.empty())
    {
	return;
    }

    // TODO: finish this when I have a use case
}

void SceneManager::preDraw()
{
    if(getDepthPartitionActive())
    {
	osgViewer::Viewer::Cameras camList;
	CVRViewer::instance()->getCameras(camList);
	for(int i = 0; i < camList.size(); i++)
	{
	    _callbackMap[camList[i]].initialDraw = camList[i]->getInitialDrawCallback();
	    _callbackMap[camList[i]].preDraw = camList[i]->getPreDrawCallback();
	    _callbackMap[camList[i]].postDraw = camList[i]->getPostDrawCallback();
	    _callbackMap[camList[i]].finalDraw = camList[i]->getFinalDrawCallback();

	    camList[i]->setInitialDrawCallback(NULL);
	    camList[i]->setPreDrawCallback(NULL);
	    camList[i]->setPostDrawCallback(NULL);
	    camList[i]->setFinalDrawCallback(NULL);
	}
    }
}

void SceneManager::postDraw()
{
    if(getDepthPartitionActive())
    {
	for(std::map<osg::Camera*,CameraCallbacks>::iterator it = _callbackMap.begin(); it != _callbackMap.end(); it++)
	{
	    it->first->setInitialDrawCallback(it->second.initialDraw);
	    it->first->setPreDrawCallback(it->second.preDraw);
	    it->first->setPostDrawCallback(it->second.postDraw);
	    it->first->setFinalDrawCallback(it->second.finalDraw);
	}
	_callbackMap.clear();
        _depthPartitionLeft->removeNodesFromCameras();
        _depthPartitionRight->removeNodesFromCameras();
    }
}
