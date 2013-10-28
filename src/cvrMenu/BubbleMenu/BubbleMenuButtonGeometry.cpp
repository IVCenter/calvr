#include <cvrMenu/BubbleMenu/BubbleMenuButtonGeometry.h>
#include <cvrMenu/MenuButton.h>
#include <osg/PolygonMode>
#include <osg/LineWidth>

using namespace cvr;

BubbleMenuButtonGeometry::BubbleMenuButtonGeometry() :
        BubbleMenuGeometry()
{
}

BubbleMenuButtonGeometry::~BubbleMenuButtonGeometry()
{
}

void BubbleMenuButtonGeometry::selectItem(bool on)
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

void BubbleMenuButtonGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    MenuButton * mb = dynamic_cast<MenuButton*>(item);

    osg::Vec4 color = osg::Vec4(0,1,0,1);

    // Unselected text
    osgText::Text3D * textNode = make3DText(mb->getText(),_textSize,
            osg::Vec3(0,0,0),_textColor);
    _geode->addDrawable(textNode);

    osg::BoundingBox bb = textNode->getBound();
    _width = bb.xMax() - bb.xMin();
    _height = bb.yMax() - bb.yMin();

    textNode->setPosition(osg::Vec3(_radius / 2,0,0));

    // Selected text
    textNode = make3DText(mb->getText(),_textSize,osg::Vec3(0,0,0),
            _textColorSelected);
    _geodeSelected->addDrawable(textNode);
    textNode->setPosition(osg::Vec3(_radius / 2,0,0));

    // Unselected sphere
    osg::Geometry * sphereGeom = makeSphere(osg::Vec3(_radius / 2,0,0),_radius,
            color);
    osg::PolygonMode * polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
            osg::PolygonMode::LINE);
    sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);
    _geode->addDrawable(sphereGeom);

    // Selected sphere
    osg::Geometry * sphereSelectedGeom = makeSphere(osg::Vec3(_radius / 2,0,0),
            _radius,color);
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);
    osg::LineWidth * lineWidth = new osg::LineWidth();
    lineWidth->setWidth(3);
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(lineWidth,
            osg::StateAttribute::ON);
    _geodeSelected->addDrawable(sphereSelectedGeom);

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

void BubbleMenuButtonGeometry::updateGeometry()
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
                osg::BoundingBox bb = text->getBound();
                _width = bb.xMax() - bb.xMin() + _iconHeight + _border;
                text = dynamic_cast<osgText::Text*>(_geodeSelected->getDrawable(
                        0));
                if(text)
                {
                    text->setText(button->getText());
                }
            }
        }

    }
}

void BubbleMenuButtonGeometry::processEvent(InteractionEvent * event)
{
    switch(event->getInteraction())
    {
        case BUTTON_DOWN:
        case BUTTON_DOUBLE_CLICK:
            if(_item->getCallback())
            {
                _item->getCallback()->menuCallback(_item);
            }
            break;
        default:
            break;
    }
}

void BubbleMenuButtonGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuButtonGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}

