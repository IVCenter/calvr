#include <cvrMenu/BoardMenu/BoardMenuRangeValueGeometry.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>

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
    _node->removeChildren(0,_node->getNumChildren());
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

    osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
    _geodeIcon->addDrawable(geo);

    _rvIcon = loadIcon("left-right.rgb");

    if(_rvIcon)
    {
        osg::StateSet * stateset = geo->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_rvIcon,
                osg::StateAttribute::ON);
    }

    MenuRangeValue * mrv = (MenuRangeValue*)item;

    const char * printstr;
    if((mrv->getMin() >= 0 && mrv->getMax() >= 0)
            || (mrv->getMin() < 0 && mrv->getMax() < 0))
    {
        printstr = "%6f";
        _sign = false;
    }
    else
    {
        printstr = "% 6f";
        _sign = true;
    }

    _label = makeText(mrv->getLabel(),_textSize,
            osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),_textColor);

    _geode->addDrawable(_label.get());

    _geodeSelected->addDrawable(
            makeText(mrv->getLabel(),_textSize,
                    osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),
                    _textColorSelected));

    char buffer[7];
    snprintf(buffer,7,"%6f",mrv->getMin());
    _minValue = makeText(buffer,_textSize * 0.75,
            osg::Vec3(0,-2,-3.0 * _iconHeight / 2.0 - _border),_textColor);

    _geode->addDrawable(_minValue.get());
    _geodeSelected->addDrawable(_minValue.get());

    float width2;

    osg::BoundingBox bbmin = _minValue->getBound();

    width2 = bbmin.xMax() - bbmin.xMin() + _border;

    geo = makeQuad(_iconHeight,-_iconHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(width2,-2,-_iconHeight - _border));
    _geodeBackIcon->addDrawable(geo);

    _backIcon = loadIcon("less.rgb");

    if(_backIcon)
    {
        osg::StateSet * stateset = _geodeBackIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_backIcon,
                osg::StateAttribute::ON);
    }

    width2 += _iconHeight + _border;

    snprintf(buffer,7,printstr,mrv->getValue());
    _currentValue = makeText(buffer,_textSize,
            osg::Vec3(width2,-2,-3.0 * _iconHeight / 2.0 - _border),_textColor);

    _geode->addDrawable(_currentValue.get());
    _geodeSelected->addDrawable(_currentValue.get());

    bbmin = _currentValue->getBound();

    width2 += bbmin.xMax() - bbmin.xMin() + _border;

    geo = makeQuad(_iconHeight,-_iconHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(width2,-2,-_iconHeight - _border));
    _geodeForwardIcon->addDrawable(geo);

    _forwardIcon = loadIcon("greater.rgb");

    if(_forwardIcon)
    {
        osg::StateSet * stateset = _geodeForwardIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_forwardIcon,
                osg::StateAttribute::ON);
    }

    width2 += _iconHeight + _border;

    snprintf(buffer,7,"%6f",mrv->getMax());
    _maxValue = makeText(buffer,_textSize * 0.75,
            osg::Vec3(width2,-2,-3.0 * _iconHeight / 2.0 - _border),_textColor);

    _geode->addDrawable(_maxValue.get());
    _geodeSelected->addDrawable(_maxValue.get());

    bbmin = _maxValue->getBound();
    width2 += bbmin.xMax() - bbmin.xMin();

    _height = _iconHeight * 2.0 + _border;

    osg::BoundingBox bb = _label->getBound();
    _width = std::max(bb.xMax() - bb.xMin() + _iconHeight + _border,width2);
}

void BoardMenuRangeValueGeometry::updateGeometry()
{
    _geode->removeDrawable(_currentValue.get());
    _geodeSelected->removeDrawable(_currentValue.get());

    float width1 = 0.0;

    osg::BoundingBox bb = _label->getBound();
    width1 = bb.xMax() - bb.xMin() + _iconHeight + _border;

    float width2 = 0.0;
    bb = _minValue->getBound();
    width2 = bb.xMax() - bb.xMin() + _border + _iconHeight + _border;

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

    snprintf(buffer,7,printstr,mrv->getValue());
    _currentValue = makeText(buffer,_textSize,
            osg::Vec3(width2,-2,-3.0 * _iconHeight / 2.0 - _border),_textColor);

    _geode->addDrawable(_currentValue.get());
    _geodeSelected->addDrawable(_currentValue.get());

    bb = _currentValue->getBound();
    width2 += bb.xMax() - bb.xMin() + _border + _iconHeight + _border;

    bb = _maxValue->getBound();
    width2 += bb.xMax() - bb.xMin();

    _width = std::max(width1,width2);
}

void BoardMenuRangeValueGeometry::processEvent(InteractionEvent * event)
{
    if(event->asMouseEvent())
    {
        MouseInteractionEvent * mie = event->asMouseEvent();
        if(event->getInteraction() == BUTTON_DOWN
                || event->getInteraction() == BUTTON_DOUBLE_CLICK)
        {
            int x, y;

            x = mie->getX();
            y = mie->getY();

            _lastMouseX = x;
            _lastMouseY = y;
            return;
        }
        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
            int x, y;
            x = mie->getX();
            y = mie->getY();

            MenuRangeValue * mrv = (MenuRangeValue*)_item;
            float pixelRange = 400;

            bool valueUpdated = false;
            if(x > _lastMouseX)
            {
                if(mrv->getValue() != mrv->getMax())
                {
                    float change = (x - _lastMouseX)
                            * (mrv->getMax() - mrv->getMin()) / pixelRange;
                    float newValue = std::max(mrv->getValue() + change,
                            mrv->getMin());
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }
            else if(x < _lastMouseX)
            {
                if(mrv->getValue() != mrv->getMin())
                {
                    float change = (x - _lastMouseX)
                            * (mrv->getMax() - mrv->getMin()) / pixelRange;
                    float newValue = std::min(mrv->getValue() + change,
                            mrv->getMax());
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
		if(mrv->getCallbackType() != MenuRangeValue::ON_RELEASE || event->getInteraction() == BUTTON_UP)
		{
		    if(mrv->getCallback())
		    {
			mrv->getCallback()->menuCallback(_item, event->asHandEvent() ? event->asHandEvent()->getHand() : 0);
		    }
		}
            }
	    else if(mrv->getCallbackType() == MenuRangeValue::ON_RELEASE && event->getInteraction() == BUTTON_UP)
	    {
		if(mrv->getCallback())
		{
		    mrv->getCallback()->menuCallback(_item, event->asHandEvent() ? event->asHandEvent()->getHand() : 0);
		}
	    }

            _lastMouseY = y;
            _lastMouseX = x;
            return;
        }
    }
    else if(event->asTrackedButtonEvent())
    {
        TrackedButtonInteractionEvent * tie = event->asTrackedButtonEvent();
        if(event->getInteraction() == BUTTON_DOWN
                || event->getInteraction() == BUTTON_DOUBLE_CLICK)
        {
	    if(event->asPointerEvent())
	    {
		SceneManager::instance()->getPointOnTiledWall(tie->getTransform(),_point);
	    }
	    else
	    {
		_point = tie->getTransform().getTrans();
		osg::Vec3 forward = osg::Vec3(0,1.0,0) * tie->getTransform();
		forward = forward - _point;
		_normal = forward ^ osg::Vec3(0,0,1.0);
		_normal.normalize();
	    }
            _lastDistance = 0.0;
            return;
        }
        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
	    float newDistance;
	    float range;

            MenuRangeValue * mrv = (MenuRangeValue*)_item;

	    if(tie->asPointerEvent())
	    {
		osg::Vec3 newPoint;
		SceneManager::instance()->getPointOnTiledWall(tie->getTransform(),newPoint);
		newDistance = newPoint.z() - _point.z();
		range = SceneManager::instance()->getTiledWallHeight() * 0.6;
	    }
	    else
	    {
		osg::Vec3 vec = tie->getTransform().getTrans();
		vec = vec - _point;
		newDistance = vec * _normal;
		range = 600;
	    }

            bool valueUpdated = false;
            if(newDistance < _lastDistance)
            {
                if(mrv->getValue() != mrv->getMin())
                {
                    float change = (newDistance - _lastDistance)
                            * (mrv->getMax() - mrv->getMin()) / range;
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
                    float change = (newDistance - _lastDistance)
                            * (mrv->getMax() - mrv->getMin()) / range;
                    float newValue = std::min(mrv->getValue() + change,
                            mrv->getMax());
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
		if(mrv->getCallbackType() != MenuRangeValue::ON_RELEASE || event->getInteraction() == BUTTON_UP)
		{
		    if(mrv->getCallback())
		    {
			mrv->getCallback()->menuCallback(_item, event->asHandEvent() ? event->asHandEvent()->getHand() : 0);
		    }
		}
            }
	    else if(mrv->getCallbackType() == MenuRangeValue::ON_RELEASE && event->getInteraction() == BUTTON_UP)
	    {
		if(mrv->getCallback())
		{
		    mrv->getCallback()->menuCallback(_item, event->asHandEvent() ? event->asHandEvent()->getHand() : 0);
		}
	    }

            _lastDistance = newDistance;

            return;
        }
    }
}
