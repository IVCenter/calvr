#include <kernel/SceneObject.h>
#include <kernel/PluginHelper.h>
#include <util/LocalToWorldVisitor.h>
#include <util/ComputeBoundingBoxVisitor.h>

#include <osg/ShapeDrawable>
#include <osg/PolygonMode>

#include <iostream>

using namespace cvr;

SceneObject::SceneObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) :
_name(name), _navigation(navigation), _movable(movable), _clip(clip), _contextMenu(contextMenu), _showBounds(showBounds)
{
    _registered = false;
    _attached = false;
    _eventActive = false;

    _bb.init();

    _root = new osg::MatrixTransform();
    _clipRoot = new osg::ClipNode();
    _boundsTransform = new osg::MatrixTransform();
    _boundsGeode = new osg::Geode();

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

    //TODO: move default values to config file and and api to change them
    _moveButton = 0;
    _menuButton = 1;
    _moveMouseButton = 0;
    _menuMouseButton = 2;

    _activeHand = -2;
}

SceneObject::~SceneObject()
{
    if(_attached)
    {
	detachFromScene();
    }

    //TODO: unregister
    //TODO: cleanup children objects
}

void SceneObject::setNavigationOn(bool nav)
{
}

void SceneObject::setMovable(bool mov)
{
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
    updateMatrices();
}

float SceneObject::getScale()
{
    return _scaleMat.getScale().x();
}

void SceneObject::setScale(float scale)
{
    _scaleMat.makeScale(osg::Vec3(scale,scale,scale));
    updateMatrices();
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
    updateBoundsGeometry();
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
    updateBoundsGeometry();
}

void SceneObject::addMenuItem(MenuItem * item)
{
    //TODO: add context menu
}

void SceneObject::removeMenuItem(MenuItem * item)
{
    //TODO: add context menu
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
		_lastobj2world = _obj2world;
		_eventActive = true;
		_activeHand = -1;
		return true;
	    }
	    else if(mie->type == MOUSE_DRAG || mie->type == MOUSE_BUTTON_UP)
	    {
		processMove(mie->transform);
		if(mie->type == MOUSE_BUTTON_UP)
		{
		    _eventActive = false;
		    _activeHand = -2;
		}
		return true;
	    }
	}

	if(_contextMenu && mie->button == _menuMouseButton)
	{
	    if(mie->type == MOUSE_BUTTON_DOWN)
	    {
		//open/close menu
		_eventActive = true;
		_activeHand = -1;
		return true;
	    }
	    else if(mie->type == MOUSE_DRAG)
	    {
		// why not?
		return true;
	    }
	    else if(mie->type == MOUSE_BUTTON_UP)
	    {
		_eventActive = false;
		_activeHand = -2;
		return true;
	    }
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
		_lastobj2world = _obj2world;
		_eventActive = true;
		_activeHand = tie->hand;
		return true;
	    }
	    else if(tie->type == BUTTON_DRAG || tie->type == BUTTON_UP)
	    {
		processMove(transform);
		if(tie->type == BUTTON_UP)
		{
		    _eventActive = false;
		    _activeHand = -2;
		}
		return true;
	    }
	}

	if(_contextMenu && tie->button == _menuButton)
	{
	    if(tie->type == BUTTON_DOWN)
	    {
		//open/close menu
		_eventActive = true;
		_activeHand = tie->hand;
		return true;
	    }
	    else if(tie->type == BUTTON_DRAG)
	    {
		// why not?
		return true;
	    }
	    else if(tie->type == BUTTON_UP)
	    {
		_eventActive = false;
		_activeHand = -2;
		return true;
	    }
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
    //TODO: add context menu
}

void SceneObject::setBoundingBox(osg::BoundingBox bb)
{
    _bb = bb;
    updateBoundsGeometry();
}

void SceneObject::computeBoundingBox()
{
    _bb.init();

    ComputeBoundingBoxVisitor cbbv;

    if(_clip)
    {
	_clipRoot->accept(cbbv);
	_bb = cbbv.getBound();
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
    }

    updateBoundsGeometry();
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
    //TODO: nested objects
}

void SceneObject::processMove(osg::Matrix & mat)
{
    //TODO: mod for nested
    osg::Matrix m;
    if(_navigation)
    {
	m = PluginHelper::getWorldToObjectTransform();
    }
    _root->setMatrix(_lastobj2world * _lastHandInv * mat * m);

    splitMatrix();
};

void SceneObject::moveCleanup()
{
    //TODO: mod for nested
    // cleanup nav happening last in the event process
    if(_navigation && _movable)
    {
	processMove(_lastHandMat);
    }
}

bool SceneObject::intersectsFast(osg::Vec3 & start, osg::Vec3 & end)
{
    osg::Vec3 startlocal = start * _world2obj;
    osg::Vec3 endlocal = end * _world2obj;
    osg::Vec3 center = _bb.center();
    osg::Vec3 normal = endlocal - startlocal;
    normal.normalize();

    float radius = _bb.radius();

    std::cerr << "radius: " << radius << std::endl;

    // see if bounding sphere is more then a radius behind the pointer
    float dist = (center - startlocal) * normal;
    std::cerr << "dist to plane: " << dist << std::endl;
    if(dist < 0 && fabs(dist) > radius)
    {
	return false;
    }

    // see if the bounding sphere is more then a radius away from the pointer
    dist = ((center - startlocal) ^ (center - endlocal)).length() / (endlocal - startlocal).length();
    std::cerr << "dist to line: " << dist << std::endl;
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

    startlocal = start * _world2obj;
    endlocal = end * _world2obj;

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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
		intersect1 = planepoint;
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
		intersect2 = planepoint;
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
    osg::Box * box = new osg::Box(osg::Vec3(0,0,0),1.0,1.0,1.0);
    osg::ShapeDrawable * sd = new osg::ShapeDrawable(box);
    _boundsGeode->addDrawable(sd);

    osg::StateSet * stateset = sd->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    osg::PolygonMode * pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(pm,osg::StateAttribute::ON);
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
    _root->setMatrix(_scaleMat * _transMat);
    
    _obj2world = getLocalToWorldMatrix(_root.get());
    _world2obj = osg::Matrix::inverse(_obj2world);
}

void SceneObject::splitMatrix()
{
    osg::Vec3 trans, scale;
    osg::Quat rot;

    trans = _root->getMatrix().getTrans();
    scale = _root->getMatrix().getScale();
    rot = _root->getMatrix().getRotate();

    _transMat = osg::Matrix::rotate(rot) * osg::Matrix::translate(trans);
    _scaleMat = osg::Matrix::scale(scale);

    updateMatrices();
}
