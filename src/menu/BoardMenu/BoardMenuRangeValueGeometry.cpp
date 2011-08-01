#include <menu/BoardMenu/BoardMenuRangeValueGeometry.h>
#include <menu/MenuRangeValue.h>
#include <input/TrackingManager.h>
#include <kernel/InteractionManager.h>

#include <osg/Geometry>

#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace cvr;

BoardMenuRangeValueGeometry::BoardMenuRangeValueGeometry() :
    BoardMenuGeometry()
{

}

BoardMenuRangeValueGeometry::~BoardMenuRangeValueGeometry()
{

}

void BoardMenuRangeValueGeometry::selectItem(bool on)
{
    //std::cerr << "Select Item called " << on << std::endl;
    _node->removeChildren(0, _node->getNumChildren());
    if(on)
    {
        _node->addChild(_groupSelected);
    }
    else
    {
        _node->addChild(_group);
    }
}

void BoardMenuRangeValueGeometry::createGeometry(MenuItem * item)
{
    _sign = false;
    _item = item;
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();

    _group = new osg::Group();
    _groupSelected = new osg::Group();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _geodeIcon = new osg::Geode();
    _geodeForwardIcon = new osg::Geode();
    _geodeBackIcon = new osg::Geode();

    _group->addChild(_geode);
    _group->addChild(_geodeIcon);
    _group->addChild(_geodeForwardIcon);
    _group->addChild(_geodeBackIcon);
    _group->addChild(_intersect);

    _groupSelected->addChild(_geodeSelected);
    _groupSelected->addChild(_geodeIcon);
    _groupSelected->addChild(_geodeForwardIcon);
    _groupSelected->addChild(_geodeBackIcon);
    _groupSelected->addChild(_intersect);

    _node->addChild(_group);

    osg::Geometry * geo = makeQuad(_iconHeight, -_iconHeight, osg::Vec4(1.0,
                                                                        1.0,
                                                                        1.0,
                                                                        1.0),
                                   osg::Vec3(0, -2, 0));
    _geodeIcon->addDrawable(geo);

    MenuRangeValue * mrv = (MenuRangeValue*)item;

    const char * printstr;
    if((mrv->getMin() >= 0 && mrv->getMax() >= 0) || (mrv->getMin() < 0
            && mrv->getMax() < 0))
    {
        printstr = "%6f";
        _sign = false;
    }
    else
    {
        printstr = "% 6f";
        _sign = true;
    }

    _label = makeText(mrv->getLabel(), _textSize, osg::Vec3(_iconHeight
            + _boarder, -2, -_iconHeight / 2.0), _textColor);

    _geode->addDrawable(_label.get());

    _geodeSelected->addDrawable(makeText(mrv->getLabel(), _textSize,
                                         osg::Vec3(_iconHeight + _boarder, -2,
                                                   -_iconHeight / 2.0),
                                         _textColorSelected));

    char buffer[7];
    snprintf(buffer, 7, "%6f", mrv->getMin());
    _minValue = makeText(buffer, _textSize * 0.75, osg::Vec3(0, -2, -3.0
            * _iconHeight / 2.0 - _boarder), _textColor);

    _geode->addDrawable(_minValue.get());
    _geodeSelected->addDrawable(_minValue.get());

    float width2;

    osg::BoundingBox bbmin = _minValue->getBound();

    width2 = bbmin.xMax() - bbmin.xMin() + _boarder;

    geo = makeQuad(_iconHeight, -_iconHeight, osg::Vec4(1.0, 1.0, 1.0, 1.0),
                   osg::Vec3(width2, -2, -_iconHeight - _boarder));
    _geodeBackIcon->addDrawable(geo);

    _backIcon = loadIcon("less.rgb");

    if(_backIcon)
    {
	osg::StateSet * stateset = _geodeBackIcon->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, _backIcon,
                                              osg::StateAttribute::ON);
    }

    width2 += _iconHeight + _boarder;

    snprintf(buffer, 7, printstr, mrv->getValue());
    _currentValue = makeText(buffer, _textSize, osg::Vec3(width2, -2, -3.0
            * _iconHeight / 2.0 - _boarder), _textColor);

    _geode->addDrawable(_currentValue.get());
    _geodeSelected->addDrawable(_currentValue.get());

    bbmin = _currentValue->getBound();

    width2 += bbmin.xMax() - bbmin.xMin() + _boarder;

    geo = makeQuad(_iconHeight, -_iconHeight, osg::Vec4(1.0, 1.0, 1.0, 1.0),
                   osg::Vec3(width2, -2, -_iconHeight - _boarder));
    _geodeForwardIcon->addDrawable(geo);

    _forwardIcon = loadIcon("greater.rgb");

    if(_forwardIcon)
    {
	osg::StateSet * stateset = _geodeForwardIcon->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, _forwardIcon,
                                              osg::StateAttribute::ON);
    }

    width2 += _iconHeight + _boarder;

    snprintf(buffer, 7, "%6f", mrv->getMax());
    _maxValue = makeText(buffer, _textSize * 0.75, osg::Vec3(width2, -2, -3.0
            * _iconHeight / 2.0 - _boarder), _textColor);

    _geode->addDrawable(_maxValue.get());
    _geodeSelected->addDrawable(_maxValue.get());

    bbmin = _maxValue->getBound();
    width2 += bbmin.xMax() - bbmin.xMin();

    _height = _iconHeight * 2.0 + _boarder;

    osg::BoundingBox bb = _label->getBound();
    _width = std::max(bb.xMax() - bb.xMin() + _iconHeight + _boarder, width2);
}

void BoardMenuRangeValueGeometry::updateGeometry()
{
    _geode->removeDrawable(_currentValue.get());
    _geodeSelected->removeDrawable(_currentValue.get());

    float width1 = 0.0;

    osg::BoundingBox bb = _label->getBound();
    width1 = bb.xMax() - bb.xMin() + _iconHeight + _boarder;

    float width2 = 0.0;
    bb = _minValue->getBound();
    width2 = bb.xMax() - bb.xMin() + _boarder + _iconHeight + _boarder;

    char buffer[7];
    const char * printstr;
    if(_sign)
    {
        printstr = "% 6f";
    }
    else
    {
        printstr = "%6f";
    }

    MenuRangeValue * mrv = (MenuRangeValue*)_item;

    snprintf(buffer, 7, printstr, mrv->getValue());
    _currentValue = makeText(buffer, _textSize, osg::Vec3(width2, -2, -3.0
            * _iconHeight / 2.0 - _boarder), _textColor);

    _geode->addDrawable(_currentValue.get());
    _geodeSelected->addDrawable(_currentValue.get());

    bb = _currentValue->getBound();
    width2 += bb.xMax() - bb.xMin() + _boarder + _iconHeight + _boarder;

    bb = _maxValue->getBound();
    width2 += bb.xMax() - bb.xMin();

    _width = std::max(width1, width2);
}

void BoardMenuRangeValueGeometry::processEvent(InteractionEvent * event)
{
    if(event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK
	|| (TrackingManager::instance()->getUsingMouseTracker() && 
	(event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK)))
    {
	int x,y;

	if(event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK)
	{
	    MouseInteractionEvent* mie = (MouseInteractionEvent*)event;
	    x = mie->x;
	    y = mie->y;
	}
	else
	{
	    x = InteractionManager::instance()->getMouseX();
	    y = InteractionManager::instance()->getMouseY();
	}

        _lastMouseX = x;
        _lastMouseY = y;
        return;
    }
    if(event->type == MOUSE_DRAG || event->type == MOUSE_BUTTON_UP
	|| (TrackingManager::instance()->getUsingMouseTracker() &&
	(event->type == BUTTON_DRAG || event->type == BUTTON_UP)))
    {
	int x,y;
	if(event->type == MOUSE_DRAG || event->type == MOUSE_BUTTON_UP)
	{
	    MouseInteractionEvent* mie = (MouseInteractionEvent*)event;
	    x = mie->x;
	    y = mie->y;
	}
	else
	{
	    x = InteractionManager::instance()->getMouseX();
	    y = InteractionManager::instance()->getMouseY();
	    if(x == _lastMouseX && y == _lastMouseY)
	    {
		return;
	    }
	}

        MenuRangeValue * mrv = (MenuRangeValue*)_item;
        float pixelRange = 400;

        bool valueUpdated = false;
        if(y > _lastMouseY)
        {
            if(mrv->getValue() != mrv->getMax())
            {
                float change = (y - _lastMouseY) * (mrv->getMax()
                        - mrv->getMin()) / pixelRange;
                float newValue = std::max(mrv->getValue() + change,
                                          mrv->getMin());
                mrv->setValue(newValue);
                valueUpdated = true;
            }
        }
        else if(y < _lastMouseY)
        {
            if(mrv->getValue() != mrv->getMin())
            {
                float change = (y - _lastMouseY) * (mrv->getMax()
                        - mrv->getMin()) / pixelRange;
                float newValue = std::min(mrv->getValue() + change,
                                          mrv->getMax());
                mrv->setValue(newValue);
                valueUpdated = true;
            }
        }

        if(valueUpdated)
        {
            if(mrv->getCallback())
            {
                mrv->getCallback()->menuCallback(_item);
            }
        }

        _lastMouseY = y;
        _lastMouseX = x;
        return;
    }
    if(event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK)
    {
        TrackingInteractionEvent * tie = (TrackingInteractionEvent*)event;
        osg::Matrix m = tie2mat(tie);
        _point = m.getTrans();
        osg::Vec3 forward = osg::Vec3(0, 1.0, 0) * m;
        forward = forward - _point;
        _normal = forward ^ osg::Vec3(0, 0, 1.0);
        _normal.normalize();
        _lastDistance = 0.0;
        return;
    }
    if(event->type == BUTTON_DRAG || event->type == BUTTON_UP)
    {
        TrackingInteractionEvent * tie = (TrackingInteractionEvent*)event;
        MenuRangeValue * mrv = (MenuRangeValue*)_item;
        osg::Vec3 vec(tie->xyz[0], tie->xyz[1], tie->xyz[2]);
        vec = vec - _point;
        float newDistance = vec * _normal;

        float range = 600;

        bool valueUpdated = false;
        if(newDistance < _lastDistance)
        {
            if(mrv->getValue() != mrv->getMin())
            {
                float change =  (newDistance - _lastDistance) * (mrv->getMax()
                        - mrv->getMin()) / range;
                float newValue = std::max(mrv->getValue() + change,
                                          mrv->getMin());
                mrv->setValue(newValue);
                valueUpdated = true;
            }
        }
        else if(newDistance > _lastDistance)
        {
            if(mrv->getValue() != mrv->getMax())
            {
                float change = (newDistance - _lastDistance) * (mrv->getMax()
                        - mrv->getMin()) / range;
                float newValue = std::min(mrv->getValue() + change,
                                          mrv->getMax());
                mrv->setValue(newValue);
                valueUpdated = true;
            }
        }

        if(valueUpdated)
        {
            if(mrv->getCallback())
            {
                mrv->getCallback()->menuCallback(_item);
            }
        }

        _lastDistance = newDistance;

        return;
    }
}
