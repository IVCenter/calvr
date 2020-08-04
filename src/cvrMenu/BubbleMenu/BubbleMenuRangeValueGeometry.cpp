#include <cvrMenu/BubbleMenu/BubbleMenuRangeValueGeometry.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrUtil/Bounds.h>

#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/PolygonMode>

#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace cvr;

BubbleMenuRangeValueGeometry::BubbleMenuRangeValueGeometry() :
        BubbleMenuGeometry()
{

}

BubbleMenuRangeValueGeometry::~BubbleMenuRangeValueGeometry()
{

}

void BubbleMenuRangeValueGeometry::selectItem(bool on)
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

void BubbleMenuRangeValueGeometry::createGeometry(MenuItem * item)
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

    // Unselected sphere
    osg::Geometry * sphereGeom = makeSphere(osg::Vec3(_radius / 2,0,0),_radius,
            osg::Vec4(0,1,0,1));
    osg::PolygonMode * polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
            osg::PolygonMode::LINE);
    sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);

    _geode->addDrawable(sphereGeom);

    // Selected sphere
    osg::Geometry * sphereSelectedGeom = makeSphere(osg::Vec3(_radius / 2,0,0),
            _radius,osg::Vec4(0,1,0,1));
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);

    osg::LineWidth * lineWidth = new osg::LineWidth();
    lineWidth->setWidth(3);
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(lineWidth,
            osg::StateAttribute::ON);

    _geodeSelected->addDrawable(sphereSelectedGeom);

    // Text on unselected
    const char * printstr;
    MenuRangeValue * mrv = (MenuRangeValue*)item;
    char buffer[7];
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

    _label = make3DText(mrv->getLabel(),_textSize,osg::Vec3(0,0,0),_textColor);

    osg::BoundingBox bbb = getBound(_label);
    float width = bbb.xMax() - bbb.xMin();
    _label->setPosition(osg::Vec3(_radius / 2,0,0));

    _geode->addDrawable(_label.get());

    // Current value on selected
    snprintf(buffer,7,printstr,mrv->getValue());
    _currentValue = make3DText(buffer,_textSize,osg::Vec3(0,0,0),_textColor);
    bbb = getBound(_currentValue);
    width = bbb.xMax() - bbb.xMin();
    _currentValue->setPosition(osg::Vec3(_radius / 2,0,0));

    _geodeSelected->addDrawable(_currentValue.get());

    osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
    //_geodeIcon->addDrawable(geo);
    //
    //_geodeSelected->addDrawable(makeText(mrv->getLabel(),_textSize,
    //    osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),_textColorSelected));

    snprintf(buffer,7,"%6f",mrv->getMin());
    _minValue = makeText(buffer,_textSize * 0.75,
            osg::Vec3(0,-2,-3.0 * _iconHeight / 2.0 - _border),_textColor);

    //_geode->addDrawable(_minValue.get());
    //_geodeSelected->addDrawable(_minValue.get());

    float width2;

    osg::BoundingBox bbmin = getBound(_minValue);

    width2 = bbmin.xMax() - bbmin.xMin() + _border;

    geo = makeQuad(_iconHeight,-_iconHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(width2,-2,-_iconHeight - _border));
    //_geodeBackIcon->addDrawable(geo);

    _backIcon = loadIcon("less.rgb");

    if(_backIcon)
    {
        osg::StateSet * stateset = _geodeBackIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_backIcon,
                osg::StateAttribute::ON);
    }

    width2 += _iconHeight + _border;

    bbmin = getBound(_currentValue);

    width2 += bbmin.xMax() - bbmin.xMin() + _border;

    geo = makeQuad(_iconHeight,-_iconHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(width2,-2,-_iconHeight - _border));
    //_geodeForwardIcon->addDrawable(geo);

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

    //_geode->addDrawable(_maxValue.get());
    //_geodeSelected->addDrawable(_maxValue.get());

    bbmin = getBound(_maxValue);
    width2 += bbmin.xMax() - bbmin.xMin();

    _height = _iconHeight * 2.0 + _border;

    osg::BoundingBox bb = getBound(_label);
    _width = std::max(bb.xMax() - bb.xMin() + _iconHeight + _border,width2);

    // Hover text
    _textGeode = new osg::Geode();
    osgText::Text * hoverTextNode = makeText(item->getHoverText(),_textSize,
//        osg::Vec3(0,0,_radius), osg::Vec4(1,1,1,1), osgText::Text::LEFT_CENTER);
            osg::Vec3(_radius / 2,0,1.5 * _radius),osg::Vec4(1,1,1,1),
            osgText::Text::CENTER_CENTER);

    osg::StateSet * ss = _textGeode->getOrCreateStateSet();
    ss->setMode(GL_BLEND,osg::StateAttribute::ON);
    ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setRenderBinDetails(11,"Render Bin");

    hoverTextNode->setFont(_font);
    _textGeode->addDrawable(hoverTextNode);
}

void BubbleMenuRangeValueGeometry::updateGeometry()
{
    //_geode->removeDrawable(_currentValue.get());
    _geodeSelected->removeDrawable(_currentValue.get());

    float width1 = 0.0;

    osg::BoundingBox bb = getBound(_label);
    width1 = bb.xMax() - bb.xMin() + _iconHeight + _border;

    float width2 = 0.0;
    bb = getBound(_minValue);
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

    _currentValue = make3DText(buffer,_textSize,osg::Vec3(0,0,0),_textColor);
    _currentValue->setPosition(osg::Vec3(_radius / 2,0,0));

    //_geode->addDrawable(_currentValue.get());
    _geodeSelected->addDrawable(_currentValue.get());

    bb = getBound(_currentValue);
    width2 += bb.xMax() - bb.xMin() + _border + _iconHeight + _border;

    bb = getBound(_maxValue);
    width2 += bb.xMax() - bb.xMin();

    _width = std::max(width1,width2);
}

void BubbleMenuRangeValueGeometry::processEvent(InteractionEvent * event)
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
                if(mrv->getCallback())
                {
                    mrv->getCallback()->menuCallback(_item);
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
            _point = tie->getTransform().getTrans();
            osg::Vec3 forward = osg::Vec3(0,1.0,0) * tie->getTransform();
            forward = forward - _point;
            _normal = forward ^ osg::Vec3(0,0,1.0);
            _normal.normalize();
            _lastDistance = 0.0;
            return;
        }
        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
            MenuRangeValue * mrv = (MenuRangeValue*)_item;
            osg::Vec3 vec = tie->getTransform().getTrans();
            vec = vec - _point;
            float newDistance = vec * _normal;

            float range = 600;

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
                if(mrv->getCallback())
                {
                    mrv->getCallback()->menuCallback(_item);
                }
            }

            _lastDistance = newDistance;

            return;
        }
    }
}

void BubbleMenuRangeValueGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuRangeValueGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}
