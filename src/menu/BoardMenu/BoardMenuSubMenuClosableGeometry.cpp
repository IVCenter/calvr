#include <menu/BoardMenu/BoardMenuSubMenuClosableGeometry.h>
#include <util/Intersection.h>
#include <kernel/NodeMask.h>
#include <kernel/SceneManager.h>

#include <osg/Geometry>

#include <iostream>

using namespace cvr;

BoardMenuSubMenuClosableGeometry::BoardMenuSubMenuClosableGeometry(bool head) :
    BoardMenuSubMenuGeometry(head)
{
    //std::cerr << "Making closable sub menu." << std::endl;
    _overX = false;
}

BoardMenuSubMenuClosableGeometry::~BoardMenuSubMenuClosableGeometry()
{
}

void BoardMenuSubMenuClosableGeometry::createGeometry(MenuItem * item)
{
    BoardMenuSubMenuGeometry::createGeometry(item);

    if(_head)
    {
	_xGeode = new osg::Geode();
	_xTransform = new osg::MatrixTransform();

	_xTransform->addChild(_xGeode);
	_node->addChild(_xTransform);

	osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
		osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
	_xGeode->addDrawable(geo);

	_xIcon = loadIcon("whiteX.rgb");
	_xSelectedIcon = loadIcon("greenX.rgb");

	if(_xIcon)
	{
	    osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
	    stateset->setTextureAttributeAndModes(0,_xIcon);
	}

	_width += _border + _iconHeight;
    }
}

void BoardMenuSubMenuClosableGeometry::selectItem(bool on)
{
    if(_head && !on && _overX && _xIcon)
    {
	osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0,_xIcon);
	_overX = false;
    }
    BoardMenuSubMenuGeometry::selectItem(on);
}

void BoardMenuSubMenuClosableGeometry::processEvent(InteractionEvent * event)
{
    if(_head)
    {
	if(_overX && event->getInteraction() == BUTTON_DOWN)
	{
	    if(_item->getCallback())
	    {
		_item->getCallback()->menuCallback(_item);
	    }
	}
    }
}

void BoardMenuSubMenuClosableGeometry::resetIntersect(float width)
{
    BoardMenuGeometry::resetIntersect(width);

    if(_head)
    {
	osg::Matrix m;
	m.makeTranslate(osg::Vec3(width - _iconHeight,0,0));
	_xTransform->setMatrix(m);
    }
}

void BoardMenuSubMenuClosableGeometry::update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd)
{
    if(_head)
    {
	std::vector<IsectInfo> isecvec;
	isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
		pointerStart,pointerEnd);

	bool hit = false;
	for(int i = 0; i < isecvec.size(); i++)
	{
	    if(isecvec[i].geode == _xGeode.get())
	    {
		hit = true;
		break;
	    }
	}

	if(hit && !_overX && _xSelectedIcon)
	{
	    osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
	    stateset->setTextureAttributeAndModes(0,_xSelectedIcon);
	    _overX = true;
	}
	else if(!hit && _overX && _xIcon)
	{
	    osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
	    stateset->setTextureAttributeAndModes(0,_xIcon);
	    _overX = false;
	}
    }
}
