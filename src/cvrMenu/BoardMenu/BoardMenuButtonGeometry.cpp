#include <cvrMenu/BoardMenu/BoardMenuButtonGeometry.h>
#include <cvrMenu/MenuButton.h>
#include <cvrUtil/Bounds.h>

#include <osg/Version>

using namespace cvr;

BoardMenuButtonGeometry::BoardMenuButtonGeometry() :
        BoardMenuGeometry()
{
}

BoardMenuButtonGeometry::~BoardMenuButtonGeometry()
{
}

void BoardMenuButtonGeometry::selectItem(bool on)
{
    if(on)
    {
        _node->removeChild(_geode);
        _node->removeChild(_geodeSelected);
        _node->addChild(_geodeSelected);
    }
    else
    {
        _node->removeChild(_geode);
        _node->removeChild(_geodeSelected);
        _node->addChild(_geode);
    }
}

void BoardMenuButtonGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    MenuButton * mb = dynamic_cast<MenuButton*>(item);

    osgText::Text * textNode = makeText(mb->getText(),_textSize,
            osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),_textColor);

    if(!mb->getIndent())
    {
	textNode->setPosition(osg::Vec3(0,-2,-_iconHeight / 2.0));
    }

    osg::BoundingBox bb = cvr::getBound(textNode);
    _width = bb.xMax() - bb.xMin() + _iconHeight + _border;
    _height = _iconHeight;

    _geode->addDrawable(textNode);

    textNode = makeText(mb->getText(),_textSize,
            osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),
            _textColorSelected);

    _geodeSelected->addDrawable(textNode);

    if(!mb->getIndent())
    {
	textNode->setPosition(osg::Vec3(0,-2,-_iconHeight / 2.0));
	_width = bb.xMax() - bb.xMin();
    }
}

void BoardMenuButtonGeometry::updateGeometry()
{
    MenuButton * button = dynamic_cast<MenuButton*>(_item);
    if(!button)
    {
        return;
    }

    if(_geode->getNumDrawables())
    {
        osgText::Text * text = dynamic_cast<osgText::Text*>(_geode->getDrawable(
                0));
        if(text)
        {
            if(text->getText().createUTF8EncodedString() != button->getText())
            {
                text->setText(button->getText());
        osg::BoundingBox bb = cvr::getBound(text);

		if(button->getIndent())
		{
		    text->setPosition(osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0));
		    _width = bb.xMax() - bb.xMin() + _iconHeight + _border;
		}
		else
		{
		    text->setPosition(osg::Vec3(0,-2,-_iconHeight / 2.0));
		    _width = bb.xMax() - bb.xMin();
		}

                text = dynamic_cast<osgText::Text*>(_geodeSelected->getDrawable(
                        0));
                if(text)
		{
		    text->setText(button->getText());

		    if(button->getIndent())
		    {
			text->setPosition(osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0));
		    }
		    else
		    {
			text->setPosition(osg::Vec3(0,-2,-_iconHeight / 2.0));
		    }
		}
            }
        }

    }
}

void BoardMenuButtonGeometry::processEvent(InteractionEvent * event)
{
    switch(event->getInteraction())
    {
        case BUTTON_DOWN:
        case BUTTON_DOUBLE_CLICK:
            if(_item->getCallback())
            {
                _item->getCallback()->menuCallback(_item,
                        event->asHandEvent() ?
                                event->asHandEvent()->getHand() : 0);
            }
            break;
        default:
            break;
    }
}
