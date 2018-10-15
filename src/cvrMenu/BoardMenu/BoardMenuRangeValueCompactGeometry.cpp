#include <cvrMenu/BoardMenu/BoardMenuRangeValueCompactGeometry.h>
#include <cvrMenu/MenuRangeValueCompact.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrUtil/Bounds.h>

#include <osg/Geometry>

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cvrConfig/ConfigManager.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace cvr;

BoardMenuRangeValueCompactGeometry::BoardMenuRangeValueCompactGeometry() :
        BoardMenuGeometry()
{

}

BoardMenuRangeValueCompactGeometry::~BoardMenuRangeValueCompactGeometry()
{

}

void BoardMenuRangeValueCompactGeometry::selectItem(bool on)
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

void BoardMenuRangeValueCompactGeometry::createGeometry(MenuItem * item)
{
    _item = item;
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();

    _group = new osg::Group();
    _groupSelected = new osg::Group();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _geodeIcon = new osg::Geode();

    _group->addChild(_geode);
    _group->addChild(_geodeIcon);
    _group->addChild(_intersect);

    _groupSelected->addChild(_geodeSelected);
    _groupSelected->addChild(_geodeIcon);
    _groupSelected->addChild(_intersect);

    _node->addChild(_group);

    osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,ConfigManager::CONTENT_BOARD_DIST,0));
    _geodeIcon->addDrawable(geo);

    _rvIcon = loadIcon("left-right.rgb");

    if(_rvIcon)
    {
        osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_rvIcon,
                osg::StateAttribute::ON);
    }

    MenuRangeValueCompact * mrv = (MenuRangeValueCompact*)item;

    const char * printstr;
    if((mrv->getMin() >= 0 && mrv->getMax() >= 0)
            || (mrv->getMin() < 0 && mrv->getMax() < 0))
    {
        printstr = "%6f";
        _sign = true;
    }
    else
    {
        printstr = "% 6f";
        _sign = false;
    }

    _label = makeText(mrv->getLabel(),_textSize,
            osg::Vec3(_iconHeight + _border,ConfigManager::CONTENT_BOARD_DIST,-_iconHeight / 2.0),_textColor);

    _geode->addDrawable(_label.get());

    char buffer[8];
    memset(buffer,'\0',8);
    _widthLabel = _widthValue = _iconHeight + _border;

    snprintf(buffer,7,printstr,mrv->getValue());
    _currentValue = makeText(buffer,_textSize,
            osg::Vec3(_widthValue,ConfigManager::CONTENT_BOARD_DIST,-_iconHeight / 2.0),_textColorSelected);

    _geodeSelected->addDrawable(_currentValue.get());

    osg::BoundingBox bb;
    bb = cvr::getBound(_label);
    _widthLabel += bb.xMax() - bb.xMin();

    bb = cvr::getBound(_currentValue);
    _widthValue += bb.xMax() - bb.xMin();

    _height = _iconHeight;

    _width = std::max(_widthLabel,_widthValue);
}

void BoardMenuRangeValueCompactGeometry::updateGeometry()
{
    _geodeSelected->removeDrawable(_currentValue.get());

    char buffer[8];
    memset(buffer,'\0',8);
    const char * printstr;
    if(_sign)
    {
        printstr = "% 6f";
    }
    else
    {
        printstr = "%6f";
    }

    MenuRangeValueCompact * mrv = (MenuRangeValueCompact*)_item;

    snprintf(buffer,7,printstr,mrv->getValue());
    _currentValue = makeText(buffer,_textSize,
            osg::Vec3(_iconHeight + _border,ConfigManager::CONTENT_BOARD_DIST,-_iconHeight / 2.0),_textColor);

    _geodeSelected->addDrawable(_currentValue.get());

    osg::BoundingBox bb = cvr::getBound(_currentValue);
    _widthValue = bb.xMax() - bb.xMin() + _border + _iconHeight;

    _width = std::max(_widthLabel,_widthValue);
}

void BoardMenuRangeValueCompactGeometry::processEvent(InteractionEvent * event)
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

            MenuRangeValueCompact * mrv = (MenuRangeValueCompact*)_item;

            float min, max, current;

            if(mrv->getIsLog())
            {
                min = log(mrv->getMin()) / log(mrv->getLogBase());
                max = log(mrv->getMax()) / log(mrv->getLogBase());
                current = log(mrv->getValue()) / log(mrv->getLogBase());
            }
            else
            {
                min = mrv->getMin();
                max = mrv->getMax();
                current = mrv->getValue();
            }

            float pixelRange = 800;

            bool valueUpdated = false;
            if(x > _lastMouseX)
            {
                if(mrv->getValue() != mrv->getMax())
                {
                    float change = (x - _lastMouseX) * (max - min) / pixelRange;

                    float newValue;

                    if(mrv->getIsLog())
                    {
                        newValue = pow(mrv->getLogBase(),current + change);
                        newValue = std::max(newValue,mrv->getMin());
                    }
                    else
                    {
                        newValue = std::max(mrv->getValue() + change,
                                mrv->getMin());
                    }
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }
            else if(x < _lastMouseX)
            {
                if(mrv->getValue() != mrv->getMin())
                {
                    float change = (x - _lastMouseX) * (max - min) / pixelRange;
                    float newValue;

                    if(mrv->getIsLog())
                    {
                        newValue = pow(mrv->getLogBase(),current + change);
                        newValue = std::min(newValue,mrv->getMax());
                    }
                    else
                    {
                        newValue = std::min(mrv->getValue() + change,
                                mrv->getMax());
                    }
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(mrv->getCallbackType() != MenuRangeValueCompact::ON_RELEASE
                        || event->getInteraction() == BUTTON_UP)
                {
                    if(mrv->getCallback())
                    {
                        mrv->getCallback()->menuCallback(_item,
                                event->asHandEvent() ?
                                        event->asHandEvent()->getHand() : 0);
                    }
                }
            }
            else if(mrv->getCallbackType() == MenuRangeValueCompact::ON_RELEASE
                    && event->getInteraction() == BUTTON_UP)
            {
                if(mrv->getCallback())
                {
                    mrv->getCallback()->menuCallback(_item,
                            event->asHandEvent() ?
                                    event->asHandEvent()->getHand() : 0);
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
                SceneManager::instance()->getPointOnTiledWall(
                        tie->getTransform(),_point);
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
            MenuRangeValueCompact * mrv = (MenuRangeValueCompact*)_item;

            float min, max, current;

            if(mrv->getIsLog())
            {
                min = log(mrv->getMin()) / log(mrv->getLogBase());
                max = log(mrv->getMax()) / log(mrv->getLogBase());
                current = log(mrv->getValue()) / log(mrv->getLogBase());
            }
            else
            {
                min = mrv->getMin();
                max = mrv->getMax();
                current = mrv->getValue();
            }

            float newDistance;
            float range;

            if(tie->asPointerEvent())
            {
                osg::Vec3 newPoint;
                SceneManager::instance()->getPointOnTiledWall(
                        tie->getTransform(),newPoint);
                newDistance = newPoint.z() - _point.z();
                range = SceneManager::instance()->getTiledWallHeight() * 0.6;
            }
            else
            {
                osg::Vec3 vec = tie->getTransform().getTrans();
                vec = vec - _point;
                newDistance = vec * _normal;
                range = 1000;
            }

            bool valueUpdated = false;
            if(newDistance < _lastDistance)
            {
                if(mrv->getValue() != mrv->getMin())
                {
                    float change = (newDistance - _lastDistance) * (max - min)
                            / range;
                    float newValue;

                    if(mrv->getIsLog())
                    {
                        newValue = pow(mrv->getLogBase(),current + change);
                        newValue = std::max(newValue,mrv->getMin());
                    }
                    else
                    {
                        newValue = std::max(mrv->getValue() + change,
                                mrv->getMin());
                    }
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }
            else if(newDistance > _lastDistance)
            {
                if(mrv->getValue() != mrv->getMax())
                {
                    float change = (newDistance - _lastDistance) * (max - min)
                            / range;
                    float newValue;

                    if(mrv->getIsLog())
                    {
                        newValue = pow(mrv->getLogBase(),current + change);
                        newValue = std::min(newValue,mrv->getMax());
                    }
                    else
                    {
                        newValue = std::min(mrv->getValue() + change,
                                mrv->getMax());
                    }
                    mrv->setValue(newValue);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(mrv->getCallbackType() != MenuRangeValueCompact::ON_RELEASE
                        || event->getInteraction() == BUTTON_UP)
                {
                    if(mrv->getCallback())
                    {
                        mrv->getCallback()->menuCallback(_item,
                                event->asHandEvent() ?
                                        event->asHandEvent()->getHand() : 0);
                    }
                }
            }
            else if(mrv->getCallbackType() == MenuRangeValueCompact::ON_RELEASE
                    && event->getInteraction() == BUTTON_UP)
            {
                if(mrv->getCallback())
                {
                    mrv->getCallback()->menuCallback(_item,
                            event->asHandEvent() ?
                                    event->asHandEvent()->getHand() : 0);
                }
            }

            _lastDistance = newDistance;

            return;
        }
    }
}
