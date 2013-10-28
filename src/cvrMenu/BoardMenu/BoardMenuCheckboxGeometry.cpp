#include <cvrMenu/BoardMenu/BoardMenuCheckboxGeometry.h>
#include <cvrMenu/MenuCheckbox.h>

#include <osg/Geometry>

using namespace cvr;

BoardMenuCheckboxGeometry::BoardMenuCheckboxGeometry() :
        BoardMenuGeometry()
{
}

BoardMenuCheckboxGeometry::~BoardMenuCheckboxGeometry()
{
}

void BoardMenuCheckboxGeometry::selectItem(bool on)
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

void BoardMenuCheckboxGeometry::createGeometry(MenuItem * item)
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

    osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
    _geodeIcon->addDrawable(geo);

    osgText::Text * textNode = makeText(checkbox->getText(),_textSize,
            osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),_textColor);
    /*osgText::Text * textNode = new osgText::Text();
     textNode->setCharacterSize(_textSize);
     textNode->setAlignment(osgText::Text::LEFT_CENTER);
     textNode->setPosition(osg::Vec3(_iconHeight + _border, -2, -_iconHeight
     / 2.0));
     textNode->setColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
     textNode->setBackdropColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));
     textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
     textNode->setText(checkbox->getText());*/

    osg::BoundingBox bb = textNode->getBound();
    _width = bb.xMax() - bb.xMin() + _iconHeight + _border;
    //mg->height = bb.zMax() - bb.zMin();
    _height = _iconHeight;

    _geode->addDrawable(textNode);

    textNode = makeText(checkbox->getText(),_textSize,
            osg::Vec3(_iconHeight + _border,-2,-_iconHeight / 2.0),
            _textColorSelected);
    /*textNode = new osgText::Text();
     textNode->setCharacterSize(_textSize);
     textNode->setAlignment(osgText::Text::LEFT_CENTER);
     textNode->setPosition(osg::Vec3(_iconHeight + _border, -2, -_iconHeight
     / 2.0));
     textNode->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
     textNode->setBackdropColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));
     textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
     textNode->setText(checkbox->getText());*/

    _geodeSelected->addDrawable(textNode);

    _checkedIcon = loadIcon("checkbox=TRUE.rgb");
    _uncheckedIcon = loadIcon("checkbox=FALSE.rgb");

    if(_uncheckedIcon && !checkbox->getValue())
    {
        osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_uncheckedIcon,
                osg::StateAttribute::ON);
    }
    else if(_checkedIcon && checkbox->getValue())
    {
        osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_checkedIcon,
                osg::StateAttribute::ON);
    }
}

void BoardMenuCheckboxGeometry::updateGeometry()
{
    MenuCheckbox * checkbox = dynamic_cast<MenuCheckbox*>(_item);
    if(!checkbox)
    {
        return;
    }

    if(_uncheckedIcon && !checkbox->getValue())
    {
        osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_uncheckedIcon,
                osg::StateAttribute::ON);
    }
    else if(_checkedIcon && checkbox->getValue())
    {
        osg::StateSet * stateset = _geodeIcon->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_checkedIcon,
                osg::StateAttribute::ON);
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
                osg::BoundingBox bb = text->getBound();
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

void BoardMenuCheckboxGeometry::processEvent(InteractionEvent * event)
{
    switch(event->getInteraction())
    {
        case BUTTON_DOWN:
        case BUTTON_DOUBLE_CLICK:
            ((MenuCheckbox*)_item)->setValue(
                    !((MenuCheckbox*)_item)->getValue());
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
