#include <menu/BoardMenu/BoardMenuImageGeometry.h>
#include <menu/MenuImage.h>

#include <osg/Geometry>

#include <iostream>

using namespace cvr;

BoardMenuImageGeometry::BoardMenuImageGeometry() : BoardMenuGeometry()
{
}

BoardMenuImageGeometry::~BoardMenuImageGeometry()
{
}

void BoardMenuImageGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _mt = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_mt);
    _mt->addChild(_geode);

    _item = item;

    MenuImage * mi = dynamic_cast<MenuImage*>(item);
    if(!mi)
    {
	std::cerr << "BoardMenuImageGeometry: Error, item is not of MenuImage type." << std::endl;
	return;
    }
    _mi = mi;

    osg::Geometry * geo = makeQuad(1.0,1.0, osg::Vec4(1.0,1.0,1.0,1.0), osg::Vec3(0,-2,-1.0));

    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0, 0));
    texcoords->push_back(osg::Vec2(1, 0));
    texcoords->push_back(osg::Vec2(1, 1));
    texcoords->push_back(osg::Vec2(0, 1));
    geo->setTexCoordArray(0, texcoords);

    _geode->addDrawable(geo);

    osg::StateSet * stateset = _geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    updateImage();
}

void BoardMenuImageGeometry::processEvent(InteractionEvent * event)
{
}

void BoardMenuImageGeometry::updateGeometry()
{
    updateImage();
}

void BoardMenuImageGeometry::updateImage()
{
    osg::Vec3 scale;
    if(_mi->getImage())
    {
	osg::StateSet * stateset = _geode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, _mi->getImage(),
                                              osg::StateAttribute::ON);
	//stateset->setMode(GL_TEXTURE_2D, osg::StateAttribute::ON);
	scale = osg::Vec3(_mi->getWidth(),1.0,_mi->getHeight());
	_width = _mi->getWidth();
	_height = _mi->getHeight();

	_geode->setNodeMask(~0);
    }
    else
    {
	osg::StateSet * stateset = _geode->getOrCreateStateSet();
	//stateset->setMode(GL_TEXTURE_2D, osg::StateAttribute::OFF);
	scale = osg::Vec3(0,1,0);
	_width = _height = 0;
    }

    osg::Matrix m;
    m.makeScale(scale);

    _mt->setMatrix(m);

}
