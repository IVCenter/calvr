#include <cvrMenu/BubbleMenu/BubbleMenuImageGeometry.h>
#include <cvrMenu/MenuImage.h>

#include <osg/Geometry>

#include <iostream>

using namespace cvr;

BubbleMenuImageGeometry::BubbleMenuImageGeometry() :
        BubbleMenuGeometry()
{
}

BubbleMenuImageGeometry::~BubbleMenuImageGeometry()
{
}

void BubbleMenuImageGeometry::createGeometry(MenuItem * item)
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
        std::cerr
                << "BubbleMenuImageGeometry: Error, item is not of MenuImage type."
                << std::endl;
        return;
    }
    _mi = mi;

    osg::Geometry * geo = makeQuad(1.0,1.0,osg::Vec4(1.0,1.0,1.0,1.0),
            osg::Vec3(0,-2,-1.0));

    _geode->addDrawable(geo);

    osg::StateSet * stateset = _geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    updateImage();

    // Hover text
    _textGeode = new osg::Geode();
    osgText::Text * hoverTextNode = makeText(item->getHoverText(), _textSize,
//       osg::Vec3(0,0,_radius), osg::Vec4(1,1,1,1), osgText::Text::LEFT_CENTER);
       osg::Vec3(_radius/2,0,1.5*_radius), osg::Vec4(1,1,1,1), osgText::Text::CENTER_CENTER);

    osg::StateSet * ss = _textGeode->getOrCreateStateSet();
    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setRenderBinDetails(11, "Render Bin");

    hoverTextNode->setFont(_font);
    _textGeode->addDrawable(hoverTextNode);
    //_node->addChild(_textGeode);
}

void BubbleMenuImageGeometry::processEvent(InteractionEvent * event)
{
}

void BubbleMenuImageGeometry::updateGeometry()
{
    updateImage();
}

void BubbleMenuImageGeometry::updateImage()
{
    osg::Vec3 scale;
    if(_mi->getImage())
    {
        osg::StateSet * stateset = _geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_mi->getImage(),
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

void BubbleMenuImageGeometry::showHoverText()
{
    if (_textGeode && _node)
    {
    _node->addChild(_textGeode);
    }
}

void BubbleMenuImageGeometry::hideHoverText()
{
    if (_textGeode && _node)
    {
    _node->removeChild(_textGeode);
    }
}
