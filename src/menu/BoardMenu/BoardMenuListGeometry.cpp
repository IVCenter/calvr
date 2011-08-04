#include <menu/BoardMenu/BoardMenuListGeometry.h>
#include <menu/MenuList.h>
#include <input/TrackingManager.h>
#include <kernel/InteractionManager.h>

#include <osg/Geometry>

using namespace cvr;

BoardMenuListGeometry::BoardMenuListGeometry() : BoardMenuGeometry()
{
    _clicked = false;
}

BoardMenuListGeometry::~BoardMenuListGeometry()
{
}

void BoardMenuListGeometry::selectItem(bool on)
{
    _node->removeChildren(0, _node->getNumChildren());
    if (on)
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

    _iconGeometry = makeQuad(_iconHeight, -_iconHeight, osg::Vec4(1.0,
                                                                        1.0,
                                                                        1.0,
                                                                        1.0),
                                   osg::Vec3(0, -2, 0));
    _geodeIcon->addDrawable(_iconGeometry.get());

    osg::ref_ptr<osg::Texture2D> iconTexture = loadIcon("list.rgb");
    if (iconTexture.get() != NULL)
    {
	_geodeIcon->getOrCreateStateSet()->setTextureAttributeAndModes(0, iconTexture);
    }

    _bbAll = loadIcon("listAll.rgb");
    _bbTop = loadIcon("listTop.rgb");
    _bbMid = loadIcon("listMiddle.rgb");
    _bbBot = loadIcon("listBottom.rgb");
    if (_bbAll.get() != NULL)
    {
	_geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0, _bbAll);
    }

    _listItem = dynamic_cast<MenuList*>(item);

    _value = makeText(_listItem->getValue(), _textSize, osg::Vec3(2*_iconHeight + _boarder, -4, -_iconHeight / 2.0), _textColor);

    _geode->addDrawable(_value);

    std::vector<std::string> stringValues = _listItem->getValues();
    const unsigned int valueCount = stringValues.size();
    const unsigned int margin = valueCount / 2; // Will truncate .5
    _valuesSelected.resize(valueCount,NULL);
    float tHeight = _iconHeight * (valueCount/2.0 - 1) + _boarder * margin;
    for (int i = 0; i < valueCount; i++)
    {
        _valuesSelected[i] = makeText(stringValues[i], _textSize, osg::Vec3(2*_iconHeight + _boarder, -4, tHeight),
            i == margin ? _textColorSelected : _textColor);
        tHeight -= _iconHeight + _boarder;
    }

    _geodeSelected->addDrawable(_valuesSelected[margin]);

    osg::BoundingBox bb = _valuesSelected[margin]->getBound();
    _width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
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
    const unsigned int margin = valueCount/2; // Will truncate .5
    _valuesSelected.resize(valueCount,NULL);
    float tHeight = _iconHeight * (valueCount/2.0 - 1) + _boarder * margin;
    for (int i = 0; i < valueCount; i++)
    {
        if (_valuesSelected[i] == NULL)
            _valuesSelected[i] = makeText(stringValues[i], _textSize, osg::Vec3(2*_iconHeight + _boarder, -4, tHeight),
                                 i == margin ? _textColorSelected : _textColor);
        else
            _valuesSelected[i]->setText(stringValues[i]);
        tHeight -= _iconHeight + _boarder;
    }

    _geode->addDrawable(_value.get());

    if (_clicked)
    {
        float maxWidth;
        for (int i = 0; i < valueCount; i++)
        {
            osg::BoundingBox bb = _valuesSelected[i]->getBound();
            float width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
            if (width > maxWidth)
                maxWidth = width;

            _geodeSelected->addDrawable(_valuesSelected[i]);
        }

        if (maxWidth != _bbWidth)
        {
            _bbWidth = maxWidth;
            _backboard = makeBackboard();
        }

        if (_bbAll.get() != NULL && _bbTop.get() != NULL && _bbMid.get() != NULL && _bbBot.get() != NULL)
        {
            int index = _listItem->getIndex();
            int total = _listItem->getListSize();
            if (total == 0 || (index <= margin && index >= total - margin - 1))
                 _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0, _bbAll);
            else if (index <= margin)
                 _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0, _bbTop);
            else if (index >= total - margin - 1)
                 _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0, _bbBot);
            else
                 _geodeSelected->getOrCreateStateSet()->setTextureAttributeAndModes(0, _bbMid);
        }

        _geodeSelected->addDrawable(_backboard.get());
    }
    else
    {
        _geodeSelected->addDrawable(_valuesSelected[valueCount/2]);

        osg::BoundingBox bb = _value->getBound();
        _width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
    }
}

void BoardMenuListGeometry::processEvent(InteractionEvent * event)
{
    if (event->type == MOUSE_BUTTON_UP || event->type == BUTTON_UP)
    {
        _geodeSelected->removeDrawables(0,_geodeSelected->getNumDrawables());
        _geodeSelected->addDrawable(_valuesSelected[_listItem->getFocus()]);
        _clicked = false;
        _listItem->setDirty(true);
    }

    if (event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK
	|| (TrackingManager::instance()->getUsingMouseTracker() && 
	(event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK)))
    {
	int y;

	if (event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK)
	{
	    MouseInteractionEvent* mie = (MouseInteractionEvent*)event;
	    y = mie->y;
	}
	else
	{
	    y = InteractionManager::instance()->getMouseY();
	}

        _lastMouseY = y;

        _clicked = true;
        _listItem->setDirty(true);
        return;
    }

    if (event->type == MOUSE_DRAG || event->type == MOUSE_BUTTON_UP
	|| (TrackingManager::instance()->getUsingMouseTracker() &&
	(event->type == BUTTON_DRAG || event->type == BUTTON_UP)))
    {
	int x,y;
	if (event->type == MOUSE_DRAG || event->type == MOUSE_BUTTON_UP)
	{
	    MouseInteractionEvent* mie = (MouseInteractionEvent*)event;
	    y = mie->y;
	}
	else
	{
	    y = InteractionManager::instance()->getMouseY();
	    if (y == _lastMouseY)
	    {
		return;
	    }
	}

        float pixelRange = 400;

        bool valueUpdated = false;
        int valueMax = _listItem->getListSize();
        int index = _listItem->getIndex();
        if (y != _lastMouseY)
        {
            int change = (int)((y - _lastMouseY) * valueMax / pixelRange);
            if (change)
            {
                index -= change;
                if (index > valueMax)
                    index = valueMax;
                else if (index < 0)
                    index = 0;

                _listItem->setIndex(index);
                valueUpdated = true;
            }
        }

        if (valueUpdated)
        {
            if (_listItem->getCallback())
            {
                _listItem->getCallback()->menuCallback(_item);
            }

            _lastMouseY = y;
        }

        return;
    }

    if (event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK)
    {
        TrackingInteractionEvent * tie = (TrackingInteractionEvent*)event;
        osg::Matrix m = tie2mat(tie);
        _point = m.getTrans();
        _lastDistance = 0.0;

        _clicked = true;
        _listItem->setDirty(true);
        return;
    }

    if (event->type == BUTTON_DRAG || event->type == BUTTON_UP)
    {
        TrackingInteractionEvent * tie = (TrackingInteractionEvent*)event;
        MenuList * _listItem = (MenuList*)_item;
        osg::Vec3 vec(tie->xyz[0], tie->xyz[1], tie->xyz[2]);
        vec = vec - _point;
        float newDistance = vec.z();

        float range = 600;

        bool valueUpdated = false;
        int valueMax = _listItem->getListSize();
        int index = _listItem->getIndex();
        if (newDistance != _lastDistance)
        {
            int change = (int)((newDistance - _lastDistance) * valueMax / range);
            if (change)
            {
                index += change;
                if (index > valueMax)
                    index = valueMax;
                else if (index < 0)
                    index = 0;

                _listItem->setIndex(index);
                valueUpdated = true;
            }
        }

        if (valueUpdated)
        {
            if (_listItem->getCallback())
            {
                _listItem->getCallback()->menuCallback(_item);
            }

            _lastDistance = newDistance;
        }

        return;
    }
}

osg::Geometry * BoardMenuListGeometry::makeBackboard()
{
    const unsigned int margin = _listItem->getFocus();
    float indent = _iconHeight + _boarder;
    float bbWidth = _bbWidth - indent + _iconHeight * 2;
    float bbHeight = (margin * 2 + 1) * _iconHeight + (margin * 2) * _boarder;
    return makeQuad(bbWidth, bbHeight, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec3(indent,-3,(bbHeight+_iconHeight)/-2));
}
