#include <cvrMenu/MenuBar.h>

using namespace cvr;

MenuBar::MenuBar(osg::Vec4 color, float mult) :
        MenuItem()
{
    _color = color;
    _mult = mult;
}

MenuBar::~MenuBar()
{
}

void MenuBar::setColor(osg::Vec4 color)
{
    _color = color;
    setDirty(true);
}

void MenuBar::setMultiplier(float mult)
{
    _mult = mult;
    setDirty(true);
}

osg::Vec4 MenuBar::getColor()
{
    return _color;
}

float MenuBar::getMultiplier()
{
    return _mult;
}

MenuItemType MenuBar::getType()
{
    return BAR;
}
