#include <kernel/SceneObject.h>
#include <kernel/PluginHelper.h>
#include <util/LocalToWorldVisitor.h>
#include <util/ComputeBoundingBoxVisitor.h>
#include <menu/MenuCheckbox.h>
#include <menu/MenuRangeValue.h>

#include <osg/ShapeDrawable>
#include <osg/PolygonMode>
#include <osg/Geometry>

#include <iostream>

using namespace cvr;

SceneObject::SceneObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) :
_name(name), _navigation(navigation), _movable(movable), _clip(clip), _contextMenu(contextMenu), _showBounds(showBounds)
{
    _registered = false;
    _attached = false;
    _eventActive = false;
    _moving = false;
    _parent = NULL;
    _boundsDirty = false;
    _boundsCalcMode = AUTO;
    _interactionCount = 0;

    _bb.init();
    _bbLocal.init();

    if(_contextMenu)
    {
	_myMenu = new PopupMenu(_name);
    }
    else
    {
	_myMenu = NULL;
    }

    _moveMenuItem = NULL;
    _navMenuItem = NULL;
    _scaleMenuItem = NULL;

    _root = new osg::MatrixTransform();
    _clipRoot = new osg::ClipNode();
    _boundsTransform = new osg::MatrixTransform();
    _boundsGeode = new osg::Geode();
    _boundsGeodeActive = new osg::Geode();

    _boundsTransform->addChild(_boundsGeode);
    createBoundsGeometry();

    if(_clip)
    {
	_root->addChild(_clipRoot);
    }
    if(_showBounds)
    {
	_root->addChild(_boundsTransform);
    }

    _moveButton = 0;
    _menuButton = SceneManager::instance()->_menuDefaultOpenButton;
    _moveMouseButton = 0;
    _menuMouseButton = SceneManager::instance()->_menuDefaultOpenButtonMouse;

    _activeHand = -2;
}

SceneObject::~SceneObject()
{
    if(_attached)
    {
	detachFromScene();
    }

    if(_registered)
    {
	SceneManager::instance()->unregisterSceneObject(this);
    }

    if(_parent)
    {
	_parent->removeChild(this);
    }
}

bool SceneObject::getNavigationOn()
{
    if(!_parent)
    {
	return _navigation;
    }
    else
    {
	return _parent->getNavigationOn();
    }
}

void SceneObject::setNavigationOn(bool nav)
{
    if(nav == _navigation)
    {
	return;
    }

    if(_attached)
    {
	if(nav)
	{
	    SceneManager::instance()->getScene()->removeChild(_root);
	    SceneManager::instance()->getObjectsRoot()->addChild(_root);
	    _root->setMatrix(_root->getMatrix() * PluginHelper::getWorldToObjectTransform());
	}
	else
	{
	    SceneManager::instance()->getObjectsRoot()->removeChild(_root);
	    SceneManager::instance()->getScene()->addChild(_root);
	    _root->setMatrix(_root->getMatrix() * PluginHelper::getObjectToWorldTransform());
	}
	splitMatrix();
    }

    _navigation = nav;
    if(_navMenuItem)
    {
	_navMenuItem->setValue(_navigation);
    }
}

void SceneObject::setMovable(bool mov)
{
    if(mov == _movable)
    {
	return;
    }

    if(!mov)
    {
	if(_moving)
	{
	    _moving = false;
	    _eventActive = false;
	}
    }

    _movable = mov;
    if(_moveMenuItem)
    {
	_moveMenuItem->setValue(_movable);
    }
}

void SceneObject::setClipOn(bool clip)
{
    if(_clip == clip)
    {
	return;
    }

    if(clip)
    {
	for(int i = 0; i < _root->getNumChildren(); i++)
	{
	    _clipRoot->addChild(_root->getChild(i));
	}
	_root->removeChildren(0,_root->getNumChildren());
	_root->addChild(_clipRoot);
	//TODO: request clip plane update
    }
    else
    {
	for(int i = 0; i < _clipRoot->getNumChildren(); i++)
	{
	    _root->addChild(_clipRoot->getChild(i));
	}
	_clipRoot->removeChildren(0,_clipRoot->getNumChildren());
	_root->removeChild(_clipRoot);
    }
    _clip = clip;
}

void SceneObject::setShowBounds(bool bounds)
{
    if(_showBounds == bounds)
    {
	return;
    }

    if(bounds)
    {
	_root->addChild(_boundsTransform);
    }
    else
    {
	_root->removeChild(_boundsTransform);
    }
    _showBounds = bounds;
}

osg::Vec3 SceneObject::getPosition()
{
    return _transMat.getTrans();
}

void SceneObject::setPosition(osg::Vec3 pos)
{
    _transMat.setTrans(pos);
    updateMatrices();
}

osg::Quat SceneObject::getRotation()
{
    return _transMat.getRotate();
}

void SceneObject::setRotation(osg::Quat rot)
{
    _transMat.setRotate(rot);
    updateMatrices();
}

osg::Matrix SceneObject::getTransform()
{
    return _transMat;
}

void SceneObject::setTransform(osg::Matrix m)
{
    _transMat = m;
    splitMatrix();
}

float SceneObject::getScale()
{
    return _scaleMat.getScale().x();
}

void SceneObject::setScale(float scale)
{
    _scaleMat.makeScale(osg::Vec3(scale,scale,scale));
    updateMatrices();
    if(_scaleMenuItem)
    {
	_scaleMenuItem->setValue(scale);
    }
}

void SceneObject::attachToScene()
{
    if(_attached)
    {
	return;
    }

    if(!_registered)
    {
	std::cerr << "Scene Object: " << _name << " must be registered before it is attached." << std::endl;
	return;
    }

    if(_parent)
    {
	std::cerr << "Scene Object: attachToScene: error, " << _name << " is a child object." << std::endl;
	return;
    }

    if(_navigation)
    {
	SceneManager::instance()->getObjectsRoot()->addChild(_root);
    }
    else
    {
	SceneManager::instance()->getScene()->addChild(_root);
    }

    updateMatrices();

    _attached = true;
}

void SceneObject::detachFromScene()
{
    if(!_attached)
    {
	return;
    }

    if(SceneManager::instance()->getMenuOpenObject() == this)
    {
	SceneManager::instance()->closeOpenObjectMenu();
    }

    if(_navigation)
    {
	SceneManager::instance()->getObjectsRoot()->removeChild(_root);
    }
    else
    {
	SceneManager::instance()->getScene()->removeChild(_root);
    }

    _attached = false;
}

void SceneObject::addChild(osg::Node * node)
{
    if(_clip)
    {
	_clipRoot->addChild(node);
    }
    else
    {
	_root->addChild(node);
    }

    _childrenNodes.push_back(node);

    //updateBoundsGeometry();
    dirtyBounds();
}

void SceneObject::removeChild(osg::Node * node)
{
    if(_clip)
    {
	_clipRoot->removeChild(node);
    }
    else
    {
	_root->removeChild(node);
    }

    for(std::vector<osg::ref_ptr<osg::Node> >::iterator it = _childrenNodes.begin(); it != _childrenNodes.end(); it++)
    {
	if(it->get() == node)
	{
	    _childrenNodes.erase(it);
	    break;
	}
    }

    dirtyBounds();
    //updateBoundsGeometry();
}

void SceneObject::addChild(SceneObject * so)
{
    if(so->_registered)
    {
	if(so->_attached)
	{
	    so->detachFromScene();
	}
	SceneManager::instance()->unregisterSceneObject(so);
    }

    if(_clip)
    {
	_clipRoot->addChild(so->_root);
    }
    else
    {
	_root->addChild(so->_root);
    }

    so->_parent = this;
    _childrenObjects.push_back(so);
    so->updateMatrices();

    //updateBoundsGeometry();
}

void SceneObject::removeChild(SceneObject * so)
{
    if(_clip)
    {
	_clipRoot->removeChild(so->_root);
    }
    else
    {
	_root->removeChild(so->_root);
    }

    for(std::vector<SceneObject*>::iterator it = _childrenObjects.begin(); it != _childrenObjects.end(); it++)
    {
	if((*it) == so)
	{
	    (*it)->_parent = NULL;
	    _childrenObjects.erase(it);
	    break;
	}
    }

    //updateBoundsGeometry();
}

osg::Node * SceneObject::getChildNode(int node)
{
    if(node < 0 || node >= _childrenNodes.size())
    {
	return NULL;
    }

    return _childrenNodes[node];
}

SceneObject * SceneObject::getChildObject(int obj)
{
    if(obj < 0 || obj >= _childrenObjects.size())
    {
	return NULL;
    }

    return _childrenObjects[obj];
}

void SceneObject::addMenuItem(MenuItem * item)
{
    if(_myMenu)
    {
	_myMenu->addMenuItem(item);
    }
}

void SceneObject::removeMenuItem(MenuItem * item)
{
    if(_myMenu)
    {
	_myMenu->removeMenuItem(item);
    }
}

void SceneObject::addMoveMenuItem(std::string label)
{
    if(_myMenu)
    {
	if(!_moveMenuItem)
	{
	    _moveMenuItem = new MenuCheckbox(label,_movable);
	    _moveMenuItem->setCallback(this);
	}
	_moveMenuItem->setValue(_movable);
	_myMenu->removeMenuItem(_moveMenuItem);
	_myMenu->addMenuItem(_moveMenuItem);
    }
}

void SceneObject::removeMoveMenuItem()
{
    if(_myMenu && _moveMenuItem)
    {
	_myMenu->removeMenuItem(_moveMenuItem);
    }
}

void SceneObject::addNavigationMenuItem(std::string label)
{
    if(_myMenu)
    {
	if(!_navMenuItem)
	{
	    _navMenuItem = new MenuCheckbox(label,_navigation);
	    _navMenuItem->setCallback(this);
	}
	_navMenuItem->setValue(_navigation);
	_myMenu->removeMenuItem(_navMenuItem);
	_myMenu->addMenuItem(_navMenuItem);
    }
}

void SceneObject::removeNavigationMenuItem()
{
    if(_myMenu && _navMenuItem)
    {
	_myMenu->removeMenuItem(_navMenuItem);
    }
}

void SceneObject::addScaleMenuItem(std::string label, float min, float max, float value)
{
    if(_myMenu)
    {
	if(_scaleMenuItem)
	{
	    _myMenu->removeMenuItem(_scaleMenuItem);
	    delete _scaleMenuItem;
	}
	_scaleMenuItem = new MenuRangeValue(label,min,max,value);
	_scaleMenuItem->setCallback(this);
	_myMenu->addMenuItem(_scaleMenuItem);
	menuCallback(_scaleMenuItem);
    }
}

void SceneObject::removeScaleMenuItem()
{
    if(_myMenu && _scaleMenuItem)
    {
	_myMenu->removeMenuItem(_scaleMenuItem);
    }
}

osg::Matrix SceneObject::getObjectToWorldMatrix()
{
    if(getNavigationOn())
    {
	return _root->getMatrix() * _obj2root * PluginHelper::getObjectToWorldTransform();
    }
    else
    {
	return _root->getMatrix() * _obj2root;
    }
}

osg::Matrix SceneObject::getWorldToObjectMatrix()
{
    if(getNavigationOn())
    {
	return PluginHelper::getWorldToObjectTransform() * _root2obj * _invTransform;
    }
    else
    {
	return _root2obj * _invTransform;
    }
}

bool SceneObject::processEvent(InteractionEvent * ie)
{
    MouseInteractionEvent * mie = NULL;// = dynamic_cast<MouseInteractionEvent*>(ie);

    if(ie->type == MOUSE_BUTTON_DOWN || ie->type == MOUSE_DOUBLE_CLICK || ie->type == MOUSE_BUTTON_UP || ie->type == MOUSE_DRAG)
    {
	mie = (MouseInteractionEvent*)ie;
    }

    if(mie)
    {
	if(_eventActive && _activeHand != -1)
	{
	    return false;
	}

	if(_movable && mie->button == _moveMouseButton)
	{
	    if(mie->type == MOUSE_BUTTON_DOWN)
	    {
		_lastHandInv = osg::Matrix::inverse(mie->transform);
		_lastHandMat = mie->transform;
		_lastobj2world = getObjectToWorldMatrix();
		_eventActive = true;
		_moving = true;
		_activeHand = -1;
		return true;
	    }
	    else if(_moving && (mie->type == MOUSE_DRAG || mie->type == MOUSE_BUTTON_UP))
	    {
		processMove(mie->transform);
		if(mie->type == MOUSE_BUTTON_UP)
		{
		    _eventActive = false;
		    _moving = false;
		    _activeHand = -2;
		}
		return true;
	    }
	}

	if(_contextMenu && mie->button == _menuMouseButton)
	{
	    if(mie->type == MOUSE_BUTTON_DOWN)
	    {
		if(!_myMenu->isVisible())
		{
		    _myMenu->setVisible(true);
		    osg::Vec3 start(0,0,0), end(0,1000,0);
		    start = start * mie->transform;
		    end = end * mie->transform;

		    osg::Vec3 p1,p2;
		    bool n1,n2;
		    float dist = 0;

		    if(intersects(start,end,p1,n1,p2,n2))
		    {
			p1 = p1 * getObjectToWorldMatrix();
			p2 = p2 * getObjectToWorldMatrix();
			float d1 = (p1 - start).length();
			if(n1)
			{
			    d1 = -d1;
			}

			float d2 = (p2 - start).length();
			if(n2)
			{
			    d2 = -d2;
			}

			if(d1 < d2)
			{
			    dist = d1;
			}
			else
			{
			    dist = d2;
			}
		    }

		    dist = std::min(dist,SceneManager::instance()->_menuMaxDistanceMouse);
		    dist = std::max(dist,SceneManager::instance()->_menuMinDistanceMouse);

		    osg::Vec3 menuPoint(0,dist,0);
		    menuPoint = menuPoint * mie->transform;
		    osg::Matrix m;
		    m.makeTranslate(menuPoint);
		    _myMenu->setTransform(m);

		    _myMenu->setScale(SceneManager::instance()->_menuScaleMouse);

		    SceneManager::instance()->setMenuOpenObject(this);
		}
		else
		{
		    SceneManager::instance()->closeOpenObjectMenu();
		}
		//_eventActive = true;
		//_activeHand = -1;
		return true;
	    }
	    /*else if(mie->type == MOUSE_DRAG)
	    {
		return true;
	    }
	    else if(mie->type == MOUSE_BUTTON_UP)
	    {
		//_eventActive = false;
		//_activeHand = -2;
		return true;
	    }*/
	}

	//TODO: replace button down/up active check with mask of buttons to
	// handle multiple buttons down
	bool retValue;
	retValue = eventCallback(mie->type, -1, mie->button, mie->transform);
	if(retValue && mie->type == MOUSE_BUTTON_DOWN)
	{
	    _activeButton = mie->button;
	    _eventActive = true;
	    _activeHand = -1;
	}
	else if(mie->type == MOUSE_BUTTON_UP && mie->button == _activeButton)
	{
	    _eventActive = false;
	    _activeHand = -2;
	}
	return retValue;
    }

    TrackingInteractionEvent * tie = NULL; // dynamic_cast<TrackingInteractionEvent*>(ie);

    if(ie->type == BUTTON_DOWN || ie->type == BUTTON_DOUBLE_CLICK || ie->type == BUTTON_UP || ie->type == BUTTON_DRAG)
    {
	tie = (TrackingInteractionEvent*)ie;
    }

    if(tie)
    {
	if(_eventActive && _activeHand != tie->hand)
	{
	    return false;
	}

	osg::Matrix transform = tie2mat(tie);

	if(_movable && tie->button == _moveButton)
	{
	    if(tie->type == BUTTON_DOWN)
	    {
		_lastHandInv = osg::Matrix::inverse(transform);
		_lastHandMat = transform;
		_lastobj2world = getObjectToWorldMatrix();
		_eventActive = true;
		_moving = true;
		_activeHand = tie->hand;
		return true;
	    }
	    else if(_moving && (tie->type == BUTTON_DRAG || tie->type == BUTTON_UP))
	    {
		processMove(transform);
		if(tie->type == BUTTON_UP)
		{
		    _eventActive = false;
		    _moving = false;
		    _activeHand = -2;
		}
		return true;
	    }
	}

	if(_contextMenu && tie->button == _menuButton)
	{
	    if(tie->type == BUTTON_DOWN)
	    {
		if(!_myMenu->isVisible())
		{
		    _myMenu->setVisible(true);
		    osg::Vec3 start(0,0,0), end(0,1000,0);
		    start = start * transform;
		    end = end * transform;

		    osg::Vec3 p1,p2;
		    bool n1,n2;
		    float dist = 0;

		    if(intersects(start,end,p1,n1,p2,n2))
		    {
			p1 = p1 * getObjectToWorldMatrix();
			p2 = p2 * getObjectToWorldMatrix();
			float d1 = (p1 - start).length();
			if(n1)
			{
			    d1 = -d1;
			}

			float d2 = (p2 - start).length();
			if(n2)
			{
			    d2 = -d2;
			}

			if(d1 < d2)
			{
			    dist = d1;
			}
			else
			{
			    dist = d2;
			}
		    }

		    dist = std::min(dist,SceneManager::instance()->_menuMaxDistance);
		    dist = std::max(dist,SceneManager::instance()->_menuMinDistance);

		    osg::Vec3 menuPoint(0,dist,0);
		    menuPoint = menuPoint * transform;

		    osg::Vec3 viewerPoint = TrackingManager::instance()->getHeadMat(0).getTrans();
		    osg::Vec3 viewerDir = viewerPoint - menuPoint;
		    viewerDir.z() = 0.0;

		    osg::Matrix menuRot;
		    menuRot.makeRotate(osg::Vec3(0,-1,0),viewerDir);

		    osg::Matrix m;
		    m.makeTranslate(menuPoint);
		    _myMenu->setTransform(menuRot * m);

		    _myMenu->setScale(SceneManager::instance()->_menuScale);

		    SceneManager::instance()->setMenuOpenObject(this);
		}
		else
		{
		    SceneManager::instance()->closeOpenObjectMenu();
		}
		//_eventActive = true;
		//_activeHand = tie->hand;
		return true;
	    }
	    /*else if(tie->type == BUTTON_DRAG)
	    {
		return true;
	    }
	    else if(tie->type == BUTTON_UP)
	    {
		//_eventActive = false;
		//_activeHand = -2;
		return true;
	    }*/
	}

	//TODO: replace button down/up active check with mask of buttons to
	// handle multiple buttons down
	bool retValue;
	retValue = eventCallback(tie->type, tie->hand, tie->button, transform);
	if(retValue && tie->type == BUTTON_DOWN)
	{
	    _activeButton = tie->button;
	    _eventActive = true;
	    _activeHand = tie->hand;
	}
	else if(tie->type == BUTTON_UP && tie->button == _activeButton)
	{
	    _eventActive = false;
	    _activeHand = -2;
	}
	return retValue;
    }

    return false;
}

void SceneObject::menuCallback(MenuItem * item)
{
    if(item == _moveMenuItem)
    {
	if(_moveMenuItem->getValue() != _movable)
	{
	    setMovable(_moveMenuItem->getValue());
	}
    }
    else if(item == _navMenuItem)
    {
	if(_navMenuItem->getValue() != _navigation)
	{
	    setNavigationOn(_navMenuItem->getValue());
	}
    }
    else if(item == _scaleMenuItem)
    {
	setScale(_scaleMenuItem->getValue());
    }
}

void SceneObject::setBoundingBox(osg::BoundingBox bb)
{
    if(_boundsCalcMode == MANUAL)
    {
	_bb = bb;
	updateBoundsGeometry();
    }
}

const osg::BoundingBox & SceneObject::getOrComputeBoundingBox()
{
    if(_boundsCalcMode == MANUAL)
    {
	return _bb;
    }
    else
    {
	//TODO: try to do an automatic check if local nodes have changed
	// maybe check for a change in the bounding sphere?
	if(_boundsDirty)
	{
	    computeBoundingBox();
	    _boundsDirty = false;
	}

	_bb = _bbLocal;

	osg::BoundingBox tbb;
	for(int i = 0; i < _childrenObjects.size(); i++)
	{
	    tbb = _childrenObjects[i]->getOrComputeBoundingBox();
	    for(int j = 0; j < 8; j++)
	    {
		_bb.expandBy(tbb.corner(j) * _childrenObjects[i]->_root->getMatrix());
	    }
	}

	updateBoundsGeometry();

	return _bb;
    }
}

void SceneObject::closeMenu()
{
    if(_myMenu)
    {
	_myMenu->setVisible(false);
    }
}

void SceneObject::computeBoundingBox()
{
    _bbLocal.init();

    ComputeBoundingBoxVisitor cbbv;

    for(int i = 0; i < _childrenNodes.size(); i++)
    {
	cbbv = ComputeBoundingBoxVisitor();
	cbbv.setBound(_bbLocal);
	_childrenNodes[i]->accept(cbbv);
	_bbLocal = cbbv.getBound();
    }

    /*if(_clip)
    {
	_clipRoot->accept(cbbv);
	_bbLocal = cbbv.getBound();
    }
    else
    {
	for(int i = 0; i < _root->getNumChildren(); i++)
	{
	    if(_root->getChild(i) != _boundsTransform.get())
	    {
		cbbv = ComputeBoundingBoxVisitor();
		cbbv.setBound(_bb);
		_root->getChild(i)->accept(cbbv);
		_bb = cbbv.getBound();
	    }
	}
    }*/

    //updateBoundsGeometry();
}

void SceneObject::setRegistered(bool reg)
{
    if(reg == _registered)
    {
	return;
    }

    if(!reg && _attached)
    {
	detachFromScene();
    }
    _registered = reg;
}

void SceneObject::processMove(osg::Matrix & mat)
{
    //std::cerr << "Process move." << std::endl;
    osg::Matrix m;
    if(getNavigationOn())
    {
	m = PluginHelper::getWorldToObjectTransform();
    }
    _root->setMatrix(_lastobj2world * _lastHandInv * mat * m * _root2obj);

    /*osg::Matrix t;
    t = _root->getMatrix();
    std::cerr << "m: ";
    for(int i = 0; i < 16; i++)
    {
	std::cerr << t.ptr()[i] << " ";
    }
    std::cerr << std::endl;*/

    splitMatrix();

    /*t = _root->getMatrix();
    std::cerr << "After: ";
    for(int i = 0; i < 16; i++)
    {
	std::cerr << t.ptr()[i] << " ";
    }
    std::cerr << std::endl;*/

    _lastHandMat = mat;
    _lastHandInv = osg::Matrix::inverse(mat);
    _lastobj2world = getObjectToWorldMatrix();
};

void SceneObject::moveCleanup()
{
    //TODO: mod for nested
    // cleanup nav happening last in the event process
    if(_moving && getNavigationOn() && _movable)
    {
	processMove(_lastHandMat);
    }
}

bool SceneObject::intersectsFast(osg::Vec3 & start, osg::Vec3 & end)
{
    osg::Vec3 startlocal;
    osg::Vec3 endlocal;
    osg::BoundingBox bb = getOrComputeBoundingBox();

    osg::Vec3 center = bb.center();

    startlocal = start * getWorldToObjectMatrix();
    endlocal = end * getWorldToObjectMatrix();

    /*if(_navigation)
    {
	startlocal = start * PluginHelper::getWorldToObjectTransform() * _root2obj;
	endlocal = end * PluginHelper::getWorldToObjectTransform() * _root2obj;
    }
    else
    {
	startlocal = start * _root2obj;
	endlocal = end * _root2obj;
    }*/

    //std::cerr << "Start x: " << startlocal.x() << " y: " << startlocal.y() << " z: " << startlocal.z() << std::endl;
    //std::cerr << "End x: " << endlocal.x() << " y: " << endlocal.y() << " z: " << endlocal.z() << std::endl;
	
    osg::Vec3 normal = endlocal - startlocal;
    normal.normalize();

    float radius = bb.radius();

    //std::cerr << "radius: " << radius << std::endl;
    //std::cerr << "Center x: " << center.x() << " y: " << center.y() << " z: " << center.z() << std::endl;

    // see if bounding sphere is more then a radius behind the pointer
    float dist = (center - startlocal) * normal;
    //std::cerr << "dist to plane: " << dist << std::endl;
    if(dist < 0 && fabs(dist) > radius)
    {
	return false;
    }

    // see if the bounding sphere is more then a radius away from the pointer
    dist = ((center - startlocal) ^ (center - endlocal)).length() / (endlocal - startlocal).length();
    //dist = ((endlocal - startlocal) ^ (startlocal - center)).length() / (endlocal - startlocal).length();
    //std::cerr << "dist to line: " << dist << std::endl;
    if(dist > radius)
    {
	return false;
    }

    return true; 
}

bool SceneObject::intersects(osg::Vec3 & start, osg::Vec3 & end, osg::Vec3 & intersect1, bool & neg1, osg::Vec3 & intersect2, bool & neg2)
{
    osg::Vec3 linenorm;
    osg::Vec3 normal;
    osg::Vec3 startlocal,endlocal;
    osg::Vec3 planepoint;
    int isecnum = 1;
    float dnom;
    float d;

    startlocal = start * getWorldToObjectMatrix();
    endlocal = end * getWorldToObjectMatrix();

    osg::Matrix obj2world = getObjectToWorldMatrix();

    /*if(_navigation)
    {
	startlocal = start * PluginHelper::getWorldToObjectTransform() * _root2obj;
	endlocal = end * PluginHelper::getWorldToObjectTransform() * _root2obj;
    }
    else
    {
	startlocal = start * _root2obj;
	endlocal = end * _root2obj;
    }*/

    linenorm = endlocal - startlocal;
    linenorm.normalize();

    planepoint = osg::Vec3(_bb.xMin(),0,0);
    normal = osg::Vec3(1,0,0);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.y() <= _bb.yMax() && planepoint.y() >= _bb.yMin() && planepoint.z() <= _bb.zMax() && planepoint.z() >= _bb.zMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    planepoint = osg::Vec3(_bb.xMax(),0,0);
    normal = osg::Vec3(1,0,0);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.y() <= _bb.yMax() && planepoint.y() >= _bb.yMin() && planepoint.z() <= _bb.zMax() && planepoint.z() >= _bb.zMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    planepoint = osg::Vec3(0,_bb.yMin(),0);
    normal = osg::Vec3(0,1,0);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.x() <= _bb.xMax() && planepoint.x() >= _bb.xMin() && planepoint.z() <= _bb.zMax() && planepoint.z() >= _bb.zMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    planepoint = osg::Vec3(0,_bb.yMax(),0);
    normal = osg::Vec3(0,1,0);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.x() <= _bb.xMax() && planepoint.x() >= _bb.xMin() && planepoint.z() <= _bb.zMax() && planepoint.z() >= _bb.zMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    planepoint = osg::Vec3(0,0,_bb.zMin());
    normal = osg::Vec3(0,0,1);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.x() <= _bb.xMax() && planepoint.x() >= _bb.xMin() && planepoint.y() <= _bb.yMax() && planepoint.y() >= _bb.yMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    planepoint = osg::Vec3(0,0,_bb.zMax());
    normal = osg::Vec3(0,0,1);
    dnom = linenorm * normal;
    if(dnom != 0.0)
    {
	d = ((planepoint - startlocal) * normal) / dnom;
	planepoint = linenorm * d + startlocal;
	if(planepoint.x() <= _bb.xMax() && planepoint.x() >= _bb.xMin() && planepoint.y() <= _bb.yMax() && planepoint.y() >= _bb.yMin())
	{
	    if(isecnum == 1)
	    {
		intersect1 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg1 = true;
		}
		else
		{
		    neg1 = false;
		}
		isecnum = 2;
	    }
	    else
	    {
		intersect2 = planepoint * obj2world;
		if(d < 0.0)
		{
		    neg2 = true;
		}
		else
		{
		    neg2 = false;
		}
		return true;
	    }
	}
    }

    return false;
}

void SceneObject::createBoundsGeometry()
{
    /*osg::Box * box = new osg::Box(osg::Vec3(0,0,0),1.0,1.0,1.0);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(box);
    _boundsGeode->addDrawable(sd);

    osg::StateSet * stateset = sd->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    osg::PolygonMode * pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(pm,osg::StateAttribute::ON);*/

    osg::Geometry * geometry = new osg::Geometry();
    osg::Vec3Array * verts = new osg::Vec3Array(0);
    osg::Vec4Array * colors = new osg::Vec4Array(1);
    osg::DrawArrays * primitive = new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 0);
    geometry->setVertexArray(verts);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->addPrimitiveSet(primitive);
    (*colors)[0] = osg::Vec4(1.0,1.0,1.0,1.0);
    primitive->setCount(24);

    verts->push_back(osg::Vec3(-0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,0.5,0.5));
    verts->push_back(osg::Vec3(0.5,0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(-0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,-0.5));
    verts->push_back(osg::Vec3(0.5,0.5,-0.5));
    verts->push_back(osg::Vec3(-0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(-0.5,0.5,0.5));
    verts->push_back(osg::Vec3(0.5,-0.5,0.5));
    verts->push_back(osg::Vec3(0.5,0.5,0.5));

    _boundsGeode->addDrawable(geometry);

    geometry = new osg::Geometry();
    colors = new osg::Vec4Array(1);
    geometry->setVertexArray(verts);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->addPrimitiveSet(primitive);
    (*colors)[0] = osg::Vec4(1.0,0.0,0.0,1.0);
    _boundsGeodeActive->addDrawable(geometry);

    osg::StateSet * stateset = _boundsTransform->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
}

void SceneObject::updateBoundsGeometry()
{
    osg::Vec3 scale(_bb.xMax() - _bb.xMin(), _bb.yMax() - _bb.yMin(), _bb.zMax() - _bb.zMin());
    osg::Matrix s,t;
    s.makeScale(scale);
    t.makeTranslate(_bb.center());
    _boundsTransform->setMatrix(s * t);
}

void SceneObject::updateMatrices()
{
    //std::cerr << "UpdateMatrices" << std::endl;
    _root->setMatrix(_scaleMat * _transMat);
    _invTransform = osg::Matrix::inverse(_root->getMatrix());

    if(!_parent)
    {
	_obj2root.makeIdentity();
	_root2obj.makeIdentity();
    }
    else
    {
	_obj2root = _parent->_root->getMatrix() * _parent->_obj2root;
	_root2obj = osg::Matrix::inverse(_obj2root);
    }

    for(int i = 0; i < _childrenObjects.size(); i++)
    {
	_childrenObjects[i]->updateMatrices();
    }

}

void SceneObject::splitMatrix()
{
    osg::Vec3 trans, scale;
    osg::Quat rot, so;

    /*trans = _root->getMatrix().getTrans();
    scale = _root->getMatrix().getScale();
    rot = _root->getMatrix().getRotate();*/
    _root->getMatrix().decompose(trans,rot,scale,so);

    //std::cerr << "Trans x: " << trans.x() << " y: " << trans.y() << " z: " << trans.z() << std::endl;

    _transMat = osg::Matrix::rotate(rot) * osg::Matrix::translate(trans);
    _scaleMat = osg::Matrix::scale(scale);

    updateMatrices();
}

void SceneObject::interactionCountInc()
{
    _interactionCount++;
    if(_interactionCount == 1)
    {
	_boundsTransform->removeChild(_boundsGeode);
	_boundsTransform->addChild(_boundsGeodeActive);
    }
}

void SceneObject::interactionCountDec()
{
    _interactionCount--;
    if(!_interactionCount)
    {
	_boundsTransform->removeChild(_boundsGeodeActive);
	_boundsTransform->addChild(_boundsGeode);
    }
}
