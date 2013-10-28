#include <cvrMenu/BoardMenu/BoardMenuListGeometry.h>
#include <cvrMenu/MenuList.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrUtil/LocalToWorldVisitor.h>
#include <cvrUtil/OsgMath.h>

#include <osg/Geometry>

using namespace cvr;

BoardMenuListGeometry::BoardMenuListGeometry() :
        BoardMenuGeometry()
{
    _clicked = false;
}

BoardMenuListGeometry::~BoardMenuListGeometry()
{
}

void BoardMenuListGeometry::selectItem(bool on)
{
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

void BoardMenuListGeometry::createGeometry(MenuItem * item)
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

    _iconGeometry = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
    _geodeIcon->addDrawable(_iconGeometry.get());

    osg::ref_ptr<osg::Texture2D> iconTexture = loadIcon("brackets.rgb");
    if(iconTexture.get() != NULL)
    {
        _geodeIcon->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                iconTexture);
    }

    _bbAll = loadIcon("listAll.rgb");
    _bbTop = loadIcon("listTop.rgb");
    _bbMid = loadIcon("listMiddle.rgb");
    _bbBot = loadIcon("listBottom.rgb");
    if(_bbAll.get() != NULL)
    {
        _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                _bbAll);
    }

    _listItem = dynamic_cast<MenuList*>(item);

    _value = makeText(_listItem->getValue(),_textSize,
            osg::Vec3(2 * _iconHeight + _border,-2,-_iconHeight / 2.0),
            _textColor);

    _geode->addDrawable(_value);

    std::vector<std::string> stringValues = _listItem->getValues();
    const unsigned int valueCount = stringValues.size();
    const unsigned int margin = valueCount / 2; // Will truncate .5
    _valuesSelected.resize(valueCount,NULL);
    float tHeight = _iconHeight * (valueCount / 2.0 - 1) + _border * margin;
    for(int i = 0; i < valueCount; i++)
    {
        _valuesSelected[i] = makeText(stringValues[i],_textSize,
                osg::Vec3(2 * _iconHeight + _border,-4,tHeight),
                i == margin ? _textColorSelected : _textColor);
        tHeight -= _iconHeight + _border;
    }

    _geodeSelected->addDrawable(_valuesSelected[margin]);

    osg::BoundingBox bb = _valuesSelected[margin]->getBound();
    _width = bb.xMax() - bb.xMin() + _iconHeight * 2 + _border;
    _height = _iconHeight;

    _backboard = makeBackboard();
}

void BoardMenuListGeometry::updateGeometry()
{
    _geode->removeDrawable(_value.get());
    _geodeSelected->removeDrawables(0,_geodeSelected->getNumDrawables());

    _value->setText(_listItem->getValue());

    std::vector<std::string> stringValues = _listItem->getValues();
    const unsigned int valueCount = stringValues.size();
    const unsigned int margin = valueCount / 2; // Will truncate .5
    _valuesSelected.resize(valueCount,NULL);
    float tHeight = _iconHeight * (valueCount / 2.0 - 1) + _border * margin;
    for(int i = 0; i < valueCount; i++)
    {
        if(_valuesSelected[i] == NULL)
            _valuesSelected[i] = makeText(stringValues[i],_textSize,
                    osg::Vec3(2 * _iconHeight + _border,-4,tHeight),
                    i == margin ? _textColorSelected : _textColor);
        else
            _valuesSelected[i]->setText(stringValues[i]);
        tHeight -= _iconHeight + _border;
    }

    _geode->addDrawable(_value.get());

    if(_clicked)
    {
        float maxWidth = 0;
        for(int i = 0; i < valueCount; i++)
        {
            osg::BoundingBox bb = _valuesSelected[i]->getBound();
            float width = bb.xMax() - bb.xMin() + _iconHeight + _border;
            if(width > maxWidth)
                maxWidth = width;

            _geodeSelected->addDrawable(_valuesSelected[i]);
        }

        if(maxWidth != _bbWidth)
        {
            _bbWidth = maxWidth;
            _backboard = makeBackboard();
        }

        if(_bbAll.get() != NULL && _bbTop.get() != NULL && _bbMid.get() != NULL
                && _bbBot.get() != NULL)
        {
            int index = _listItem->getIndex();
            int total = _listItem->getListSize();
            if(total == 0 || (index <= margin && index >= total - margin - 1))
                _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(
                        0,_bbAll);
            else if(index <= margin)
                _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(
                        0,_bbTop);
            else if(index >= total - margin - 1)
                _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(
                        0,_bbBot);
            else
                _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(
                        0,_bbMid);
        }

        _geodeSelected->addDrawable(_backboard.get());
    }
    else
    {
        _geodeSelected->addDrawable(_valuesSelected[valueCount / 2]);

        osg::BoundingBox bb = _value->getBound();
        _width = bb.xMax() - bb.xMin() + _iconHeight * 2 + _border;
    }
}

void BoardMenuListGeometry::processEvent(InteractionEvent * event)
{
    if(event->getInteraction() == BUTTON_UP)
    {
        _geodeSelected->removeDrawables(0,_geodeSelected->getNumDrawables());
        _geodeSelected->addDrawable(_valuesSelected[_listItem->getFocus()]);
        _clicked = false;
        _listItem->setDirty(true);
    }

    if(event->asMouseEvent())
    {
        MouseInteractionEvent * mie = event->asMouseEvent();
        if(event->getInteraction() == BUTTON_DOWN
                || event->getInteraction() == BUTTON_DOUBLE_CLICK)
        {
            int y = mie->getY();

            _lastMouseY = y;

            _clicked = true;
            _listItem->setDirty(true);
            _change = 0.0;

            if(_listItem->getScrollingHint() == MenuList::ONE_TO_ONE)
            {
                osg::Matrix mat = getLocalToWorldMatrix(_node);
                osg::Vec3 point, normal(0,-1,0);
                point = point * mat;
                normal = normal * mat;
                normal = normal - point;
                normal.normalize();

                osg::Vec3 point1, point2(0,1,0);
                point1 = point1 * mie->getTransform();
                point2 = point2 * mie->getTransform();

                osg::Vec3 intersect;
                float w;

                if(linePlaneIntersectionRef(point1,point2,point,normal,
                        intersect,w))
                {
                    _lastDistance = intersect.z();
                }
            }

            return;
        }

        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
            int y = mie->getY();
            float pixelRange = 400;

            bool valueUpdated = false;
            int valueMax = _listItem->getListSize();
            int index = _listItem->getIndex();
            if(y != _lastMouseY)
            {
                int lastIndex = index;
                switch(_listItem->getScrollingHint())
                {
                    case MenuList::LINEAR:
                    {
                        _change += ((y - _lastMouseY)
                                * _listItem->getSensitivity() / pixelRange)
                                * valueMax;

                        if(fabs(_change) > 1.0)
                        {

                            int offset = (int)_change;
                            index -= offset;

                            _change -= ((float)offset);
                        }
                        break;
                    }
                    case MenuList::CONTINUOUS:
                    {
                        const float coeff = (35.0f)
                                / (pixelRange * pixelRange * pixelRange);
                        _change += coeff * (y - _lastMouseY) * (y - _lastMouseY)
                                * (y - _lastMouseY)
                                * _listItem->getSensitivity()
                                * CVRViewer::instance()->getLastFrameDuration();

                        if(fabs(_change) > 1.0)
                        {
                            int offset = (int)_change;
                            index -= offset;

                            _change -= ((float)offset);
                        }

                        y = _lastMouseY;
                        break;
                    }
                    case MenuList::ONE_TO_ONE:
                    {
                        osg::Matrix mat = getLocalToWorldMatrix(_node);
                        osg::Vec3 point, normal(0,-1,0);
                        point = point * mat;
                        normal = normal * mat;
                        normal = normal - point;
                        normal.normalize();

                        osg::Vec3 distVec(0,0,_iconHeight + _border);
                        distVec = distVec * mat;
                        distVec = distVec - point;
                        float lineDist = distVec.length();

                        osg::Vec3 point1, point2(0,1,0);
                        point1 = point1 * mie->getTransform();
                        point2 = point2 * mie->getTransform();

                        osg::Vec3 intersect;
                        float w;

                        if(linePlaneIntersectionRef(point1,point2,point,normal,
                                intersect,w))
                        {
                            _change += ((intersect.z() - _lastDistance)
                                    / lineDist);

                            if(fabs(_change) > 1.0)
                            {
                                int lastIndex = index;
                                int offset = (int)_change;
                                index -= offset;

                                _change -= ((float)offset);
                            }

                            _lastDistance = intersect.z();
                        }

                        break;
                    }
                    default:
                        break;
                }

                if(index > valueMax)
                    index = valueMax;
                else if(index < 0)
                    index = 0;

                if(lastIndex != index)
                {
                    _listItem->setIndex(index);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(_listItem->getCallbackType() != MenuList::ON_RELEASE
                        || event->getInteraction() == BUTTON_UP)
                {
                    if(_listItem->getCallback())
                    {
                        _listItem->getCallback()->menuCallback(_item,
                                event->asHandEvent() ?
                                        event->asHandEvent()->getHand() : 0);
                    }
                }

                _lastMouseY = y;
            }
            else if(_listItem->getCallbackType() == MenuList::ON_RELEASE
                    && event->getInteraction() == BUTTON_UP)
            {
                if(_listItem->getCallback())
                {
                    _listItem->getCallback()->menuCallback(_item,
                            event->asHandEvent() ?
                                    event->asHandEvent()->getHand() : 0);
                }
            }

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
            }

            _clicked = true;
            _listItem->setDirty(true);
            _change = 0.0;

            if(_listItem->getScrollingHint() == MenuList::ONE_TO_ONE)
            {
                osg::Matrix mat = getLocalToWorldMatrix(_node);
                osg::Vec3 point, normal(0,-1,0);
                point = point * mat;
                normal = normal * mat;
                normal = normal - point;
                normal.normalize();

                osg::Vec3 point1, point2(0,1,0);
                point1 = point1 * tie->getTransform();
                point2 = point2 * tie->getTransform();

                osg::Vec3 intersect;
                float w;

                if(linePlaneIntersectionRef(point1,point2,point,normal,
                        intersect,w))
                {
                    _lastDistance = intersect.z();
                }
            }
            else
            {
                _lastDistance = 0.0;
            }

            return;
        }

        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
            MenuList * _listItem = (MenuList*)_item;

            float newDistance;
            float range;

            if(event->asPointerEvent())
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
                newDistance = vec.z();
                range = 400;
            }

            bool valueUpdated = false;
            int valueMax = _listItem->getListSize();
            int index = _listItem->getIndex();
            if(newDistance != _lastDistance)
            {
                int lastIndex = index;
                switch(_listItem->getScrollingHint())
                {
                    case MenuList::LINEAR:
                    {
                        _change += ((newDistance - _lastDistance)
                                * _listItem->getSensitivity() / range)
                                * valueMax;
                        if(fabs(_change) > 1.0)
                        {
                            int offset = (int)_change;
                            index -= offset;

                            _change -= ((float)offset);
                        }
                        break;
                    }
                    case MenuList::CONTINUOUS:
                    {
                        const float coeff = (100.0f) / (range * range * range);
                        _change += coeff * (newDistance - _lastDistance)
                                * (newDistance - _lastDistance)
                                * (newDistance - _lastDistance)
                                * _listItem->getSensitivity()
                                * CVRViewer::instance()->getLastFrameDuration();

                        if(fabs(_change) > 1.0)
                        {
                            int offset = (int)_change;
                            index -= offset;

                            _change -= ((float)offset);
                        }

                        newDistance = _lastDistance;
                        break;
                    }
                    case MenuList::ONE_TO_ONE:
                    {
                        osg::Matrix mat = getLocalToWorldMatrix(_node);
                        osg::Vec3 point, normal(0,-1,0);
                        point = point * mat;
                        normal = normal * mat;
                        normal = normal - point;
                        normal.normalize();

                        osg::Vec3 distVec(0,0,_iconHeight + _border);
                        distVec = distVec * mat;
                        distVec = distVec - point;
                        float lineDist = distVec.length();

                        osg::Vec3 point1, point2(0,1,0);
                        point1 = point1 * tie->getTransform();
                        point2 = point2 * tie->getTransform();

                        osg::Vec3 intersect;
                        float w;

                        if(linePlaneIntersectionRef(point1,point2,point,normal,
                                intersect,w))
                        {
                            _change += ((intersect.z() - _lastDistance)
                                    / lineDist);

                            if(fabs(_change) > 1.0)
                            {
                                int lastIndex = index;
                                int offset = (int)_change;
                                index -= offset;

                                _change -= ((float)offset);
                            }

                            _lastDistance = intersect.z();
                            newDistance = _lastDistance;
                        }

                        break;
                    }
                    default:
                        break;

                }

                if(index > valueMax)
                    index = valueMax;
                else if(index < 0)
                    index = 0;

                if(lastIndex != index)
                {
                    _listItem->setIndex(index);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(_listItem->getCallbackType() != MenuList::ON_RELEASE
                        || event->getInteraction() == BUTTON_UP)
                {
                    if(_listItem->getCallback())
                    {
                        _listItem->getCallback()->menuCallback(_item,
                                event->asHandEvent() ?
                                        event->asHandEvent()->getHand() : 0);
                    }
                }

                _lastDistance = newDistance;
            }
            else if(_listItem->getCallbackType() == MenuList::ON_RELEASE
                    && event->getInteraction() == BUTTON_UP)
            {
                if(_listItem->getCallback())
                {
                    _listItem->getCallback()->menuCallback(_item,
                            event->asHandEvent() ?
                                    event->asHandEvent()->getHand() : 0);
                }
            }

            return;
        }
    }
}

osg::Geometry * BoardMenuListGeometry::makeBackboard()
{
    const unsigned int margin = _listItem->getFocus();
    float indent = _iconHeight + _border;
    float bbWidth = _bbWidth - indent + _iconHeight * 2;
    float bbHeight = (margin * 2 + 1) * _iconHeight + (margin * 2) * _border;
    return makeQuad(bbWidth,bbHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(indent,-3,(bbHeight + _iconHeight) / -2));
}
