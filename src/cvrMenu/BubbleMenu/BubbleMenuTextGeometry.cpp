#include <cvrMenu/BubbleMenu/BubbleMenuTextGeometry.h>
#include <cvrMenu/MenuText.h>
#include <cvrUtil/Bounds.h>

using namespace cvr;

BubbleMenuTextGeometry::BubbleMenuTextGeometry() :
        BubbleMenuGeometry()
{
}

BubbleMenuTextGeometry::~BubbleMenuTextGeometry()
{
}

void BubbleMenuTextGeometry::selectItem(bool on)
{
}

void BubbleMenuTextGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    MenuText * mb = dynamic_cast<MenuText*>(item);

    float xoffset = 0;
    if(mb->getIndent())
    {
        xoffset = _iconHeight + _border;
    }

    _text = makeText(mb->getText(),_textSize * mb->getSizeScale(),
            osg::Vec3(xoffset,-2,0),_textColor,osgText::Text::LEFT_TOP);
    /*osgText::Text * textNode = new osgText::Text();
     textNode->setCharacterSize(_textSize);
     textNode->setAlignment(osgText::Text::LEFT_CENTER);
     textNode->setPosition(osg::Vec3(_iconHeight + _border, -2, -_iconHeight
     / 2.0));
     textNode->setColor(_textColor);
     textNode->setBackdropColor(osg::Vec4(0, 0, 0, 0));
     textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
     textNode->setText(mb->getText());*/

    _text->setMaximumWidth(mb->getMaxWidth());

    osg::BoundingBox bb = getBound(_text);
    _width = bb.xMax() - bb.xMin(); // + _iconHeight + _border;
    if(mb->getIndent())
    {
        _width += _iconHeight + _border;
    }
    _height = bb.zMax() - bb.zMin();
    //_height = _iconHeight;

    _geode->addDrawable(_text);

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

void BubbleMenuTextGeometry::processEvent(InteractionEvent * event)
{
}

void BubbleMenuTextGeometry::updateGeometry()
{
    MenuText * mb = dynamic_cast<MenuText*>(_item);
    if(_text.valid())
    {
        _text->setText(mb->getText());

        _text->setMaximumWidth(mb->getMaxWidth());

        _text->setCharacterSize(_textSize * mb->getSizeScale());

        osg::Vec3 pos = _text->getPosition();
        if(mb->getIndent())
        {
            pos.x() = _iconHeight + _border;
        }
        else
        {
            pos.x() = 0;
        }
        _text->setPosition(pos);

        osg::BoundingBox bb = getBound(_text);
        _width = bb.xMax() - bb.xMin(); // + _iconHeight + _border;

        if(mb->getIndent())
        {
            _width += _iconHeight + _border;
        }

        _height = bb.zMax() - bb.zMin();
    }
}

void BubbleMenuTextGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuTextGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}
