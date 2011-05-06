#include <menu/BoardMenu/BoardMenuTextGeometry.h>
#include <menu/MenuText.h>

using namespace cvr;

BoardMenuTextGeometry::BoardMenuTextGeometry() :
    BoardMenuGeometry()
{
}

BoardMenuTextGeometry::~BoardMenuTextGeometry()
{
}

void BoardMenuTextGeometry::selectItem(bool on)
{
}

void BoardMenuTextGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    MenuText * mb = dynamic_cast<MenuText*> (item);

    _text = makeText(mb->getText(), _textSize, osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight / 2.0), _textColor);
    /*osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(_textSize);
    textNode->setAlignment(osgText::Text::LEFT_CENTER);
    textNode->setPosition(osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight
            / 2.0));
    textNode->setColor(_textColor);
    textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    textNode->setText(mb->getText());*/

    osg::BoundingBox bb = _text->getBound();
    _width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
    //mg->height = bb.zMax() - bb.zMin();
    _height = _iconHeight;

    _geode->addDrawable(_text);
}

void BoardMenuTextGeometry::processEvent(InteractionEvent * event)
{
}

void BoardMenuTextGeometry::updateGeometry()
{
    MenuText * mb = dynamic_cast<MenuText*> (_item);
    if(_text.valid())
    {
	_text->setText(mb->getText());

	 osg::BoundingBox bb = _text->getBound();
	_width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
    }
}
