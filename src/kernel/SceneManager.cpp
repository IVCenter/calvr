#include <kernel/SceneManager.h>
#include <input/TrackingManager.h>
#include <config/ConfigManager.h>
#include <kernel/NodeMask.h>
#include <kernel/CVRViewer.h>
#include <kernel/ComController.h>

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osg/Material>

#include <iostream>

using namespace cvr;

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

SceneManager * SceneManager::_myPtr = NULL;

SceneManager::SceneManager()
{
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

    bool dpart = ConfigManager::getBool("value",std::string("UseDepthPartition"),false);
    _depthPartitionLeft->setActive(dpart);
    _depthPartitionRight->setActive(dpart);

    _depthPartitionLeft->setNodeMask(_depthPartitionLeft->getNodeMask() & ~(CULL_MASK_RIGHT));
    _depthPartitionRight->setNodeMask(_depthPartitionRight->getNodeMask() & ~(CULL_MASK_LEFT));
    _depthPartitionRight->setNodeMask(_depthPartitionRight->getNodeMask() & ~(CULL_MASK));

    _scale = 1.0;
    _showAxis = false;
    _hidePointer = false;

    initPointers();
    initLights();
    initSceneState();
    initAxis();

    bool b = ConfigManager::getBool("ShowAxis", false);

    setAxis(b);

    b = ConfigManager::getBool("HidePointer", false);
    setHidePointer(b);

    return true;
}

void SceneManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
	return;
    }

    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
        _handTransforms[i]->setMatrix(
                                      TrackingManager::instance()->getHandMat(i));
    }

    if(_showAxis)
    {
        for(int i = 0; i < _headAxisTransforms.size(); i++)
        {
            _headAxisTransforms[i]->setMatrix(
                                              TrackingManager::instance()->getUnfrozenHeadMat(
                                                                                      i));
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
    m.makeScale(osg::Vec3d(_scale, _scale, _scale));

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

void SceneManager::initPointers()
{
    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
        _handTransforms.push_back(new osg::MatrixTransform());
	_sceneRoot->addChild(_handTransforms[i]);
        _handTransforms[i]->setMatrix(
                                      TrackingManager::instance()->getHandMat(i));

        if(!TrackingManager::instance()->getShowWand())
        {
            continue;
        }

        // TODO: get a hand model to add
        osg::Cone * cone = new osg::Cone(osg::Vec3(0, 500, 0), 10, 2000);
        osg::Quat q = osg::Quat(-M_PI / 2.0, osg::Vec3(1.0, 0, 0));
        cone->setRotation(q);
        osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
        osg::Geode * geode = new osg::Geode();
        geode->addDrawable(sd);
        geode->setNodeMask(geode->getNodeMask() & ~INTERSECT_MASK);
        _handTransforms[i]->addChild(geode);
    }
}

void SceneManager::initLights()
{
    osg::StateSet * stateset = _sceneRoot->getOrCreateStateSet();

    //TODO: replace with lighting manager

    //read any values from config file
    osg::Vec4 diffuse,specular,ambient,position;
    osg::Vec3 direction;
    float spotexp,spotcutoff;

    diffuse = ConfigManager::getColor("Light.Diffuse",osg::Vec4(1.0, 1.0, 1.0, 1.0));
    specular = ConfigManager::getColor("Light.Specular",osg::Vec4(0, 0, 0, 1.0));
    ambient = ConfigManager::getColor("Light.Ambient",osg::Vec4(0.3, 0.3, 0.3, 1.0));
    position = ConfigManager::getVec4("Light.Position",osg::Vec4(0.0, -10000.0, 10000.0, 1.0));
    direction = ConfigManager::getVec3("Light.Direction",osg::Vec3(0, 0, -1));
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
    source->setStateSetModes(*stateset, osg::StateAttribute::ON);

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

        stateset->setAttributeAndModes(light, osg::StateAttribute::ON);
        source->setLocalStateSetModes(osg::StateAttribute::ON);
        source->setStateSetModes(*stateset, osg::StateAttribute::ON);
    }
}

void SceneManager::initSceneState()
{
    osg::StateSet * stateset = _sceneRoot->getOrCreateStateSet();

    osg::LightModel * lm = new osg::LightModel();
    lm->setTwoSided(true);
    lm->setLocalViewer(true);

    osg::Material * mat = new osg::Material();
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f,
                                                             1.0f));
    mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.2f, 0.2f, 0.2f,
                                                             1.0f));
    mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f,
                                                              1.0f));
    mat->setAlpha(osg::Material::FRONT_AND_BACK, 1.0f);
    mat->setColorMode(osg::Material::DIFFUSE);

    osg::ColorMask* rootColorMask = new osg::ColorMask;
    rootColorMask->setMask(true, true, true, true);

    stateset->setAttribute(rootColorMask);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateset->setAttributeAndModes(mat, osg::StateAttribute::ON);
    stateset->setAttributeAndModes(lm, osg::StateAttribute::ON);
    stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
}

void SceneManager::initAxis()
{
    _axisNode = new osg::Group();

    osg::Cone * cone = new osg::Cone(osg::Vec3(0, 0, 0), 15, 32);
    osg::Quat q = osg::Quat(-M_PI / 2.0, osg::Vec3(1.0, 0, 0));
    cone->setRotation(q);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
    osg::Geode * geode = new osg::Geode();
    geode->addDrawable(sd);
    osg::MatrixTransform * mt = new osg::MatrixTransform();
    osg::Matrix m;
    m.makeTranslate(osg::Vec3(0, 208, 0));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    cone = new osg::Cone(osg::Vec3(0, 0, 0), 15, 32);
    q = osg::Quat(M_PI / 2.0, osg::Vec3(0, 1.0, 0));
    cone->setRotation(q);
    sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
    geode = new osg::Geode();
    geode->addDrawable(sd);
    mt = new osg::MatrixTransform();
    m.makeTranslate(osg::Vec3(208, 0, 0));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    cone = new osg::Cone(osg::Vec3(0, 0, 0), 15, 32);
    sd = new osg::ShapeDrawable(cone);
    sd->setColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));
    geode = new osg::Geode();
    geode->addDrawable(sd);
    mt = new osg::MatrixTransform();
    m.makeTranslate(osg::Vec3(0, 0, 208));
    mt->setMatrix(m);
    mt->addChild(geode);
    _axisNode->addChild(mt);

    geode = new osg::Geode();

    osg::Cylinder * cylinder = new osg::Cylinder(osg::Vec3(0, 0, 100), 10, 200);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));
    geode->addDrawable(sd);

    cylinder = new osg::Cylinder(osg::Vec3(100, 0, 0), 10, 200);
    q = osg::Quat(M_PI / 2.0, osg::Vec3(0, 1.0, 0));
    cylinder->setRotation(q);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
    geode->addDrawable(sd);

    cylinder = new osg::Cylinder(osg::Vec3(0, 100, 0), 10, 200);
    q = osg::Quat(-M_PI / 2.0, osg::Vec3(1.0, 0, 0));
    cylinder->setRotation(q);
    sd = new osg::ShapeDrawable(cylinder);
    sd->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
    geode->addDrawable(sd);

    _axisNode->addChild(geode);

    for(int i = 0; i < TrackingManager::instance()->getNumHeads(); i++)
    {
        _headAxisTransforms.push_back(new osg::MatrixTransform());
        _headAxisTransforms[i]->addChild(_axisNode.get());
    }

    //_objectRoot->addChild(_axisNode);
}
