#include <menu/BoardMenu/BoardMenuButtonGeometry.h>
#include <menu/MenuButton.h>

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

    MenuButton * mb = dynamic_cast<MenuButton*> (item);

    osgText::Text * textNode = makeText(mb->getText(), _textSize, osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight / 2.0), _textColor);
    /*osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(_textSize);
    textNode->setAlignment(osgText::Text::LEFT_CENTER);
    textNode->setPosition(osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight
            / 2.0));
    textNode->setColor(_textColor);
    textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    textNode->setText(mb->getText());*/

    osg::BoundingBox bb = textNode->getBound();
    _width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
    //mg->height = bb.zMax() - bb.zMin();
    _height = _iconHeight;

    _geode->addDrawable(textNode);

    textNode = makeText(mb->getText(), _textSize, osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight
            / 2.0), _textColorSelected);
    /*textNode = new osgText::Text();
    textNode->setCharacterSize(_textSize);
    textNode->setAlignment(osgText::Text::LEFT_CENTER);
    textNode->setPosition(osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight
            / 2.0));
    textNode->setColor(_textColorSelected);
    textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    textNode->setText(mb->getText());*/

    _geodeSelected->addDrawable(textNode);
}

void BoardMenuButtonGeometry::processEvent(InteractionEvent * event)
{
    switch(event->type)
    {
	case BUTTON_DOWN:
	case BUTTON_DOUBLE_CLICK:
	case MOUSE_BUTTON_DOWN:
	case MOUSE_DOUBLE_CLICK:
	    if(_item->getCallback())
	    {
		_item->getCallback()->menuCallback(_item);
	    }
	    break;
	default:
	    break;
    }
}
