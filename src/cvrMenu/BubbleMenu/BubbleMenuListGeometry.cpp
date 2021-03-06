#include <cvrMenu/BubbleMenu/BubbleMenuListGeometry.h>
#include <cvrMenu/MenuList.h>
#include <cvrInput/TrackingManager.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrUtil/Bounds.h>

#include <osg/Geometry>
#include <osg/PolygonMode>

using namespace cvr;

BubbleMenuListGeometry::BubbleMenuListGeometry() :
        BubbleMenuGeometry()
{
    _clicked = false;
}

BubbleMenuListGeometry::~BubbleMenuListGeometry()
{
}

void BubbleMenuListGeometry::selectItem(bool on)
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

void BubbleMenuListGeometry::createGeometry(MenuItem * item)
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

    osg::ref_ptr<osg::Texture2D> iconTexture = loadIcon("list.rgb");
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

    osg::Geometry * sphereGeom = makeSphere(osg::Vec3(50,0,0),100,
            osg::Vec4(0,1,0,1));
    osg::PolygonMode * polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
            osg::PolygonMode::LINE);
    sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);

    _geode->addDrawable(sphereGeom);

    osg::BoundingBox bb = getBound(_valuesSelected[margin]);
    _width = bb.xMax() - bb.xMin() + _iconHeight * 2 + _border;
    _height = _iconHeight;

    _backboard = makeBackboard();

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

void BubbleMenuListGeometry::updateGeometry()
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
            osg::BoundingBox bb = getBound(_valuesSelected[i]);
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

        osg::BoundingBox bb = getBound(_value);
        _width = bb.xMax() - bb.xMin() + _iconHeight * 2 + _border;
    }
}

void BubbleMenuListGeometry::processEvent(InteractionEvent * event)
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
                int change = (int)((y - _lastMouseY)
                        * _listItem->getSensitivity() / pixelRange);
                if(change)
                {
                    index -= change;
                    if(index > valueMax)
                        index = valueMax;
                    else if(index < 0)
                        index = 0;

                    _listItem->setIndex(index);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(_listItem->getCallback())
                {
                    _listItem->getCallback()->menuCallback(_item);
                }

                _lastMouseY = y;
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
            _point = tie->getTransform().getTrans();
            _lastDistance = 0.0;

            _clicked = true;
            _listItem->setDirty(true);
            return;
        }

        if(event->getInteraction() == BUTTON_DRAG
                || event->getInteraction() == BUTTON_UP)
        {
            MenuList * _listItem = (MenuList*)_item;
            osg::Vec3 vec = tie->getTransform().getTrans();
            ;
            vec = vec - _point;
            float newDistance = vec.z();

            float range = 400;

            bool valueUpdated = false;
            int valueMax = _listItem->getListSize();
            int index = _listItem->getIndex();
            if(newDistance != _lastDistance)
            {
                int change = (int)((newDistance - _lastDistance)
                        * _listItem->getSensitivity() / range);
                if(change)
                {
                    index -= change;
                    if(index > valueMax)
                        index = valueMax;
                    else if(index < 0)
                        index = 0;

                    _listItem->setIndex(index);
                    valueUpdated = true;
                }
            }

            if(valueUpdated)
            {
                if(_listItem->getCallback())
                {
                    _listItem->getCallback()->menuCallback(_item);
                }

                _lastDistance = newDistance;
            }

            return;
        }
    }
}

osg::Geometry * BubbleMenuListGeometry::makeBackboard()
{
    const unsigned int margin = _listItem->getFocus();
    float indent = _iconHeight + _border;
    float bbWidth = _bbWidth - indent + _iconHeight * 2;
    float bbHeight = (margin * 2 + 1) * _iconHeight + (margin * 2) * _border;
    return makeQuad(bbWidth,bbHeight,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(indent,-3,(bbHeight + _iconHeight) / -2));
}

void BubbleMenuListGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuListGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}
