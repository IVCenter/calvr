#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuClosableGeometry.h>
#include <cvrUtil/Intersection.h>
#include <cvrKernel/NodeMask.h>
#include <cvrKernel/SceneManager.h>

#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/LineWidth>

#include <iostream>

using namespace cvr;

BubbleMenuSubMenuClosableGeometry::BubbleMenuSubMenuClosableGeometry(bool head) :
        BubbleMenuSubMenuGeometry(head)
{
    //std::cerr << "Making closable sub menu." << std::endl;
    _overX = false;
}

BubbleMenuSubMenuClosableGeometry::~BubbleMenuSubMenuClosableGeometry()
{
}

void BubbleMenuSubMenuClosableGeometry::createGeometry(MenuItem * item)
{
    BubbleMenuSubMenuGeometry::createGeometry(item);

    if(_head)
    {
        _xGeode = new osg::Geode();
        _xTransform = new osg::MatrixTransform();

        _xTransform->addChild(_xGeode);
        _node->addChild(_xTransform);

        osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
                osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(0,-2,0));
        _xGeode->addDrawable(geo);

        _xIcon = loadIcon("whiteX.rgb");
        _xSelectedIcon = loadIcon("greenX.rgb");

        if(_xIcon)
        {
            osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0,_xIcon);
        }

        _width += _border + _iconHeight;
    }

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

void BubbleMenuSubMenuClosableGeometry::selectItem(bool on)
{
    if(_head && !on && _overX && _xIcon)
    {
        osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_xIcon);
        _overX = false;
    }
    BubbleMenuSubMenuGeometry::selectItem(on);
}

void BubbleMenuSubMenuClosableGeometry::processEvent(InteractionEvent * event)
{
    if(_head)
    {
        if(_overX && event->getInteraction() == BUTTON_DOWN)
        {
            if(_item->getCallback())
            {
                _item->getCallback()->menuCallback(_item);
            }
        }
    }
}

void BubbleMenuSubMenuClosableGeometry::resetIntersect(float width)
{
    BubbleMenuGeometry::resetIntersect(width);

    if(_head)
    {
        osg::Matrix m;
        m.makeTranslate(osg::Vec3(width - _iconHeight,0,0));
        _xTransform->setMatrix(m);
    }
}

void BubbleMenuSubMenuClosableGeometry::update(osg::Vec3 & pointerStart,
        osg::Vec3 & pointerEnd)
{
    if(_head)
    {
        std::vector<IsectInfo> isecvec;
        isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                pointerStart,pointerEnd);

        bool hit = false;
        for(int i = 0; i < isecvec.size(); i++)
        {
            if(isecvec[i].geode == _xGeode.get())
            {
                hit = true;
                break;
            }
        }

        if(hit && !_overX && _xSelectedIcon)
        {
            osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0,_xSelectedIcon);
            _overX = true;
        }
        else if(!hit && _overX && _xIcon)
        {
            osg::StateSet * stateset = _xGeode->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0,_xIcon);
            _overX = false;
        }
    }
}

void BubbleMenuSubMenuClosableGeometry::showHoverText()
{
    if(_textGeode && _node)
    {
        _node->addChild(_textGeode);
    }
}

void BubbleMenuSubMenuClosableGeometry::hideHoverText()
{
    if(_textGeode && _node)
    {
        _node->removeChild(_textGeode);
    }
}
