#include <menu/BoardMenu/BoardMenuSubMenuGeometry.h>
#include <menu/SubMenu.h>

#include <osg/Geometry>

using namespace cvr;

BoardMenuSubMenuGeometry::BoardMenuSubMenuGeometry(bool head) :
    BoardMenuGeometry()
{
    _head = head;
    _open = false;
    _width = 0;
}

BoardMenuSubMenuGeometry::~BoardMenuSubMenuGeometry()
{
}

void BoardMenuSubMenuGeometry::selectItem(bool on)
{
    if(_head)
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
    else if(!_open)
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
}

void BoardMenuSubMenuGeometry::openMenu(bool open)
{
    if(!_head)
    {
        if(open)
        {
            _node->removeChild(_geode);
            _node->removeChild(_geodeSelected);
            _node->addChild(_geodeSelected);
            if(_openIcon)
            {
                osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
                stateset->setTextureAttributeAndModes(0, _openIcon,
                                                      osg::StateAttribute::ON);
            }
        }
        else
        {
            _node->removeChild(_geode);
            _node->removeChild(_geodeSelected);
            _node->addChild(_geode);
            if(_closedIcon)
            {
                osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
                stateset->setTextureAttributeAndModes(0, _closedIcon,
                                                      osg::StateAttribute::ON);
            }
        }
	_open = open;
    }
}

void BoardMenuSubMenuGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    SubMenu * submenu = dynamic_cast<SubMenu*> (item);

    if(_head)
    {
        _geodeLine = new osg::Geode();
        _node->addChild(_geodeLine);

	osgText::Text * textNode = makeText(submenu->getTitle(), 1.15 * _textSize, osg::Vec3(0, -2, 0), _textColor, osgText::Text::LEFT_TOP);
        /*osgText::Text * textNode = new osgText::Text();
        textNode->setCharacterSize(75);
        textNode->setAlignment(osgText::Text::LEFT_TOP);
        textNode->setPosition(osg::Vec3(0, -2, 0));
        textNode->setColor(_textColor);
        textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
        textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
        textNode->setText(submenu->getTitle());*/

        _geode->addDrawable(textNode);

        osg::BoundingBox bb = textNode->getBound();
        _width = bb.xMax() - bb.xMin();
        _height = bb.zMax() - bb.zMin() + _boarder;

        //smg->geode->addDrawable(makeLine(osg::Vec3(0,-2,-(smg->height + _boarder)), osg::Vec3(smg->width,-2,-(smg->height + _boarder)),submenu->getTextColor()));

	textNode = makeText(submenu->getTitle(), 1.15 * _textSize, osg::Vec3(0, -2, 0), _textColorSelected, osgText::Text::LEFT_TOP);
        /*textNode = new osgText::Text();
        textNode->setCharacterSize(75);
        textNode->setAlignment(osgText::Text::LEFT_TOP);
        textNode->setPosition(osg::Vec3(0, -2, 0));
        textNode->setColor(_textColorSelected);
        textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
        textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
        textNode->setText(submenu->getTitle());*/

        _geodeSelected->addDrawable(textNode);
    }
    else
    {
        _geodeIcon = new osg::Geode();
        _node->addChild(_geodeIcon);

        osg::Geometry * geo = makeQuad(_iconHeight, -_iconHeight,
                                       osg::Vec4(1.0, 1.0, 1.0, 1.0),
                                       osg::Vec3(0, -2, 0));
        osg::Vec2Array* texcoords = new osg::Vec2Array;
        texcoords->push_back(osg::Vec2(0, 0));
        texcoords->push_back(osg::Vec2(1, 0));
        texcoords->push_back(osg::Vec2(1, 1));
        texcoords->push_back(osg::Vec2(0, 1));
        geo->setTexCoordArray(0, texcoords);
        _geodeIcon->addDrawable(geo);

	osgText::Text * textNode = makeText(submenu->getName(), _textSize, osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight / 2.0), _textColor);
        /*osgText::Text * textNode = new osgText::Text();
        textNode->setCharacterSize(_textSize);
        textNode->setAlignment(osgText::Text::LEFT_CENTER);
        textNode->setPosition(osg::Vec3(_iconHeight + _boarder, -2,
                                        -_iconHeight / 2.0));
        textNode->setColor(_textColor);
        textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
        textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
        textNode->setText(submenu->getName());*/

        osg::BoundingBox bb = textNode->getBound();
        _width = bb.xMax() - bb.xMin() + _iconHeight + _boarder;
        //mg->height = bb.zMax() - bb.zMin();
        _height = _iconHeight;

        _geode->addDrawable(textNode);

	textNode = makeText(submenu->getName(), _textSize, osg::Vec3(_iconHeight + _boarder, -2, -_iconHeight / 2.0), _textColorSelected);
        /*textNode = new osgText::Text();
        textNode->setCharacterSize(_textSize);
        textNode->setAlignment(osgText::Text::LEFT_CENTER);
        textNode->setPosition(osg::Vec3(_iconHeight + _boarder, -2,
                                        -_iconHeight / 2.0));
        textNode->setColor(_textColorSelected);
        textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
        textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
        textNode->setText(submenu->getName());*/

        _geodeSelected->addDrawable(textNode);

        _openIcon = loadIcon("arrow-left-highlighted.rgb");
        _closedIcon = loadIcon("arrow-left.rgb");

        if(_closedIcon)
        {
            osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0, _closedIcon,
                                                  osg::StateAttribute::ON);
        }
    }
}

void BoardMenuSubMenuGeometry::processEvent(InteractionEvent * event)
{
    if(event->type == BUTTON_DOWN || event->type == MOUSE_BUTTON_DOWN)
    {
        if(_item->getCallback())
        {
            _item->getCallback()->menuCallback(_item);
        }
    }
}

bool BoardMenuSubMenuGeometry::isMenuHead()
{
    return _head;
}

bool BoardMenuSubMenuGeometry::isMenuOpen()
{
    return _open;
}

void BoardMenuSubMenuGeometry::resetMenuLine(float width)
{
    _geodeLine->removeDrawables(0, _geodeLine->getNumDrawables());
    _geodeLine->addDrawable(makeLine(osg::Vec3(0, -2, -(_height)),
                                     osg::Vec3(width, -2, -(_height)),
                                     _textColor));
}
