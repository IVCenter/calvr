#include <cvrMenu/BoardMenu/BoardMenuBarGeometry.h>
#include <cvrMenu/MenuBar.h>

#include <iostream>
#include <cvrConfig/ConfigManager.h>

using namespace cvr;

BoardMenuBarGeometry::BoardMenuBarGeometry() : BoardMenuGeometry()
{
}

BoardMenuBarGeometry::~BoardMenuBarGeometry()
{
}

void BoardMenuBarGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();
    _mt = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_mt);
    _mt->addChild(_geode);
    
    _item = item;

    MenuBar * mb = dynamic_cast<MenuBar*>(item);
    if(!mb)
    {
	std::cerr << "BoardMenuBarGeometry: Error, item is not of MenuBar type." << std::endl;
	return;
    }
    _mb = mb;

    _geo = makeQuad(1.0,1.0,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(0,ConfigManager::CONTENT_BOARD_DIST,-1.0));

    _geode->addDrawable(_geo);

    osg::StateSet * stateset = _geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    
    updateBar(0.0);
}

void BoardMenuBarGeometry::updateGeometry()
{
    updateBar(0.0);
}

void BoardMenuBarGeometry::resetIntersect(float width)
{
    updateBar(width);
    BoardMenuGeometry::resetIntersect(width);
}

void BoardMenuBarGeometry::updateBar(float width)
{
    _width = width - 2.0 * _border;
    _height = _mb->getMultiplier() * 3.0;

    osg::Matrix m;
    m.makeScale(osg::Vec3(width,1.0,_height));
    _mt->setMatrix(m);

    osg::Vec4Array * colors = dynamic_cast<osg::Vec4Array*>(_geo->getColorArray());
    if(colors && colors->size())
    {
	colors->at(0) = _mb->getColor();
	colors->dirty();
    }
}
