#include <cvrMenu/BubbleMenu/BubbleMenuCheckboxGeometry.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrUtil/Bounds.h>

#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Shape>
#include <osg/ShapeDrawable>

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

BubbleMenuCheckboxGeometry::BubbleMenuCheckboxGeometry() :
        BubbleMenuGeometry()
{
}

BubbleMenuCheckboxGeometry::~BubbleMenuCheckboxGeometry()
{
}

void BubbleMenuCheckboxGeometry::selectItem(bool on)
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

void BubbleMenuCheckboxGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    MenuCheckbox * checkbox = dynamic_cast<MenuCheckbox*>(item);

    _geodeIcon = new osg::Geode();
    _node->addChild(_geodeIcon);

    // Unselected text
    osgText::Text3D * textNode = make3DText(checkbox->getText(),_textSize,
            osg::Vec3(0,0,0),_textColor);
    _geode->addDrawable(textNode);

    osg::BoundingBox bb = getBound(textNode);
    _width = bb.xMax() - bb.xMin();
    _height = _iconHeight;

    textNode->setPosition(osg::Vec3(_radius / 2,0,0));

    // Selected text
    textNode = make3DText(checkbox->getText(),_textSize,osg::Vec3(0,0,0),
            _textColorSelected);
    _geodeSelected->addDrawable(textNode);

    textNode->setPosition(osg::Vec3(_radius / 2,0,0));

//  osg::Vec4 c = osg::Vec4((float)(85.0/255.0), (float)(217.0/255.0), (float)(188.0/255.0), 1);

// Unselected sphere
    osg::Geometry * sphereGeom = makeSphere(osg::Vec3(_radius / 2,0,0),_radius,
            osg::Vec4(0,1,0,1));
    osg::PolygonMode * polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
            osg::PolygonMode::LINE);
    sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);
    _geode->addDrawable(sphereGeom);

    // Selected sphere
    osg::Geometry * sphereSelectedGeom = makeSphere(osg::Vec3(_radius / 2,0,0),
            _radius,osg::Vec4(0,1,0,1));
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(polygonMode,
            osg::StateAttribute::ON);
    osg::LineWidth * lineWidth = new osg::LineWidth();
    lineWidth->setWidth(3);
    sphereSelectedGeom->getOrCreateStateSet()->setAttribute(lineWidth,
            osg::StateAttribute::ON);
    _geodeSelected->addDrawable(sphereSelectedGeom);

    osg::Box *lbox, *rbox;
    osg::ShapeDrawable * boxDrawable;

    // Red X
    _xGeode = new osg::Geode();

    float xLength = _radius / 2.5, yLength = _radius / 9, zLength = _radius / 9;

    lbox = new osg::Box(osg::Vec3(_radius + _radius / 3,0,_radius),xLength,
            yLength,zLength);
    lbox->setRotation(osg::Quat(M_PI / 4,osg::Vec3(0,1,0)));

    rbox = new osg::Box(osg::Vec3(_radius + _radius / 3,0,_radius),xLength,
            yLength,zLength);
    rbox->setRotation(osg::Quat(-M_PI / 4,osg::Vec3(0,1,0)));

    boxDrawable = new osg::ShapeDrawable(lbox);
    boxDrawable->setColor(osg::Vec4(1,0,0,1));
    boxDrawable->getOrCreateStateSet()->setMode(GL_BLEND,
            osg::StateAttribute::ON);
    boxDrawable->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::ON);
    _xGeode->addDrawable(boxDrawable);

    boxDrawable = new osg::ShapeDrawable(rbox);
    boxDrawable->setColor(osg::Vec4(1,0,0,1));
    boxDrawable->getOrCreateStateSet()->setMode(GL_BLEND,
            osg::StateAttribute::ON);
    boxDrawable->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::ON);
    _xGeode->addDrawable(boxDrawable);

    // Green check
    _checkGeode = new osg::Geode();

    lbox = new osg::Box(osg::Vec3(_radius + _radius / 4,0,_radius),_radius / 3,
            _radius / 9,_radius / 9);
    lbox->setRotation(osg::Quat(M_PI / 4,osg::Vec3(0,1,0)));

    rbox = new osg::Box(
            osg::Vec3(_radius + _radius / 4 + _radius / 4,0,
                    _radius + _radius / 9),_radius / 2,_radius / 9,_radius / 9);
    rbox->setRotation(osg::Quat(-M_PI / 4,osg::Vec3(0,1,0)));

    boxDrawable = new osg::ShapeDrawable(lbox);
    boxDrawable->setColor(osg::Vec4(0,1,0,1));

    boxDrawable->getOrCreateStateSet()->setMode(GL_BLEND,
            osg::StateAttribute::ON);
    boxDrawable->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::ON);
    _checkGeode->addDrawable(boxDrawable);

    boxDrawable = new osg::ShapeDrawable(rbox);
    boxDrawable->setColor(osg::Vec4(0,1,0,1));
    boxDrawable->getOrCreateStateSet()->setMode(GL_BLEND,
            osg::StateAttribute::ON);
    boxDrawable->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::ON);
    _checkGeode->addDrawable(boxDrawable);

    _node->addChild(_checkGeode);
    _node->addChild(_xGeode);

    if(!checkbox->getValue())
    {
        _checkGeode->setNodeMask(0x0);
        _xGeode->setNodeMask(0xFFFFFF);
    }
    else if(checkbox->getValue())
    {
        _checkGeode->setNodeMask(0xFFFFFF);
        _xGeode->setNodeMask(0x0);
    }

    // Hover text
    _textGeode = new osg::Geode();
    osgText::Text * hoverTextNode = makeText(item->getHoverText(),_textSize,
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

void BubbleMenuCheckboxGeometry::updateGeometry()
{
    MenuCheckbox * checkbox = dynamic_cast<MenuCheckbox*>(_item);
    if(!checkbox)
    {
        return;
    }

    if(!checkbox->getValue())
    {
        _checkGeode->setNodeMask(0x0);
        _xGeode->setNodeMask(0xFFFFFF);
    }
    else if(checkbox->getValue())
    {
        _checkGeode->setNodeMask(0xFFFFFF);
        _xGeode->setNodeMask(0x0);
    }

    if(_geode->getNumDrawables())
    {
        osgText::Text * text = dynamic_cast<osgText::Text*>(_geode->getDrawable(
                0));
        if(text)
        {
            if(text->getText().createUTF8EncodedString() != checkbox->getText())
            {
                text->setText(checkbox->getText());
                osg::BoundingBox bb = getBound(text);
                _width = bb.xMax() - bb.xMin() + _iconHeight + _border;
                text = dynamic_cast<osgText::Text*>(_geodeSelected->getDrawable(
                        0));
                if(text)
                {
                    text->setText(checkbox->getText());
                }
            }
        }

    }
}

void BubbleMenuCheckboxGeometry::processEvent(InteractionEvent * event)
{
    switch(event->getInteraction())
    {
        case BUTTON_DOWN:
        case BUTTON_DOUBLE_CLICK:
            ((MenuCheckbox*)_item)->setValue(
                    !((MenuCheckbox*)_item)->getValue());
            if(_item->getCallback())
            {
                _item->getCallback()->menuCallback(_item);
            }
            break;
        default:
            break;
    }
}

void BubbleMenuCheckboxGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuCheckboxGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}
