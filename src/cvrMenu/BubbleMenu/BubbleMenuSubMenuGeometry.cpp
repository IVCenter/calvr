#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuGeometry.h>
#include <cvrMenu/SubMenu.h>

#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/LineWidth>

using namespace cvr;

BubbleMenuSubMenuGeometry::BubbleMenuSubMenuGeometry(bool head) :
        BubbleMenuGeometry()
{
    _head = head;
    _open = false;
    _width = 0;
}

BubbleMenuSubMenuGeometry::~BubbleMenuSubMenuGeometry()
{
}

void BubbleMenuSubMenuGeometry::selectItem(bool on)
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

void BubbleMenuSubMenuGeometry::openMenu(bool open)
{
    if(!_head)
    {
        if(open)
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
        _open = open;
    }
}

void BubbleMenuSubMenuGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _geode = new osg::Geode();
    _geodeSelected = new osg::Geode();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _node->addChild(_geode);
    _item = item;

    SubMenu * submenu = dynamic_cast<SubMenu*>(item);

    if(_head)
    {
        osgText::Text3D * textNode = make3DText(submenu->getTitle(),
                1.15 * _textSize, osg::Vec3(_radius/2,0,0),_textColor);

        _geode->addDrawable(textNode);

        osg::BoundingBox bb = textNode->getBound();
        _width = bb.xMax() - bb.xMin();
        _height = bb.zMax() - bb.zMin() + _border;

        osg::Geometry * sphereGeom = makeSphere(osg::Vec3(_radius/2,0,0), _radius, osg::Vec4(0,1,0,1));
        osg::PolygonMode * polygonMode = new osg::PolygonMode();
        polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode, osg::StateAttribute::ON);

        _geode->addDrawable(sphereGeom);
        _geodeSelected->addDrawable(sphereGeom);

        textNode = make3DText(submenu->getTitle(),1.15 * _textSize,
                osg::Vec3(_radius/2,0,0), _textColorSelected);

        _geodeSelected->addDrawable(textNode);
    }
    else
    {
        // Unselected text
        osgText::Text3D * textNode = make3DText(submenu->getName(), _textSize,
                osg::Vec3(0,0,0), _textColor);

        osg::BoundingBox bb = textNode->getBound();
        _width = bb.xMax() - bb.xMin();
        _height = _iconHeight;

        textNode->setPosition(osg::Vec3(_radius/2, 0, 0));

        _geode->addDrawable(textNode);

        // Selected text
        textNode = make3DText(submenu->getName(),_textSize,
                osg::Vec3(0,0,0), _textColorSelected);
        textNode->setPosition(osg::Vec3(_radius/2, 0, 0));
        _geodeSelected->addDrawable(textNode);

        // Unselected sphere
        osg::Geometry * sphereGeom = makeSphere(osg::Vec3(_radius/2,0,0),
            _radius, osg::Vec4(0,1,0,1));
        osg::PolygonMode * polygonMode = new osg::PolygonMode();
        polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
            osg::PolygonMode::LINE);
        sphereGeom->getOrCreateStateSet()->setAttribute(polygonMode, 
            osg::StateAttribute::ON);
        _geode->addDrawable(sphereGeom);

        // Selected sphere
        osg::Geometry * sphereSelectedGeom = makeSphere(osg::Vec3(_radius/2,0,0),
            _radius, osg::Vec4(0,1,0,1));
        sphereSelectedGeom->getOrCreateStateSet()->setAttribute(polygonMode, 
            osg::StateAttribute::ON);
        
        // Outer circle
        osg::Geometry * circleGeom = new osg::Geometry();
        osg::Vec3Array* verts = new osg::Vec3Array();

        osg::Vec3 p1, p2; 
        osg::Vec3 center = osg::Vec3(_radius/2,0,0);
        float interval = M_PI * 2 / (float)_tessellations;
        float theta = 0;
        float circleRad = _radius * 1.2;
        
        p2 = osg::Vec3(center[0] + cos(theta) * circleRad, center[1], 
            center[2] + sin(theta) * circleRad);

        for (int i = 0; i < _tessellations + 1; ++i)
        {
            theta = i * interval;
            p1 = p2;
            p2 = osg::Vec3(center[0] + cos(theta) * circleRad,  center[1], 
                center[2] + sin(theta) * circleRad);
            _geode->addDrawable(makeLine(p1, p2, _wireframeColor));
            _geodeSelected->addDrawable(makeLine(p1, p2, _wireframeColor));
        }

        osg::LineWidth * lineWidth = new osg::LineWidth();
        lineWidth->setWidth(3);
        sphereSelectedGeom->getOrCreateStateSet()->setAttribute(lineWidth, 
            osg::StateAttribute::ON);

        _geodeSelected->addDrawable(sphereSelectedGeom);
    }

    // Hover text
    _textGeode = new osg::Geode();
    osgText::Text * hoverTextNode =  makeText(item->getHoverText(), _textSize,
//        osg::Vec3(0,0,_radius), osg::Vec4(1,1,1,1), osgText::Text::LEFT_CENTER);
       osg::Vec3(_radius/2,0,1.5*_radius), osg::Vec4(1,1,1,1), osgText::Text::CENTER_CENTER);

    osg::StateSet * ss = _textGeode->getOrCreateStateSet();
    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setRenderBinDetails(11, "Render Bin");

    hoverTextNode->setFont(_font);
    _textGeode->addDrawable(hoverTextNode);
}

void BubbleMenuSubMenuGeometry::processEvent(InteractionEvent * event)
{
    if(event->getInteraction() == BUTTON_DOWN)
    {
        if(_item->getCallback())
        {
            _item->getCallback()->menuCallback(_item);
        }
    }
}

bool BubbleMenuSubMenuGeometry::isMenuHead()
{
    return _head;
}

bool BubbleMenuSubMenuGeometry::isMenuOpen()
{
    return _open;
}

void BubbleMenuSubMenuGeometry::resetMenuLine(float width)
{
    _geodeLine->removeDrawables(0,_geodeLine->getNumDrawables());
    _geodeLine->addDrawable(
            makeLine(osg::Vec3(0,-2,-(_height)),osg::Vec3(width,-2,-(_height)),
                _textColor));
}

void BubbleMenuSubMenuGeometry::showHoverText()
{
    if (_textGeode && _node)
    {
    _node->addChild(_textGeode);
    }
}

void BubbleMenuSubMenuGeometry::hideHoverText()
{
    if (_textGeode && _node)
    {
    _node->removeChild(_textGeode);
    }
}
