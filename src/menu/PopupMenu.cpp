#include <menu/PopupMenu.h>
#include <menu/BoardPopupMenu.h>
#include <menu/MenuManager.h>
#include <menu/PopupMenuBase.h>

#include <config/ConfigManager.h>

using namespace cvr;

PopupMenu::PopupMenu(std::string title, std::string configTag)
{
    _title = title;
    _configName = configTag;

    //TODO: get from somewhere
    _type = BOARDMENU;

    _menu = new BoardPopupMenu();

    _rootMenu = new SubMenu(_title,_title);
    _menu->setMenu(_rootMenu);

    float x,y,z,h,p,r,scale;
    x = y = z = h = p = r = 0.0;
    scale = 1.0;
    if(!configTag.empty())
    {
	x = ConfigManager::getFloat("x",configTag,0.0);
	y = ConfigManager::getFloat("y",configTag,0.0);
	z = ConfigManager::getFloat("z",configTag,0.0);
	h = ConfigManager::getFloat("h",configTag,0.0);
	p = ConfigManager::getFloat("p",configTag,0.0);
	r = ConfigManager::getFloat("r",configTag,0.0);
	scale = ConfigManager::getFloat("scale",configTag,1.0);
    }

    osg::Vec3 pos(x,y,z);
    osg::Quat rot(r,osg::Vec3(0,1,0),p,osg::Vec3(1,0,0),h,osg::Vec3(0,0,1));
    _menu->setPosition(pos);
    _menu->setRotation(rot);
    _menu->setScale(scale);
    
    MenuManager::instance()->addMenuSystem(this);    
}

PopupMenu::~PopupMenu()
{
    MenuManager::instance()->removeMenuSystem(this);
}

void PopupMenu::addMenuItem(MenuItem * item)
{
    _rootMenu->addItem(item);
}

void PopupMenu::removeMenuItem(MenuItem * item)
{
    _rootMenu->removeItem(item);
}

void PopupMenu::setPosition(osg::Vec3 pos)
{
    _menu->setPosition(pos);
}

osg::Vec3 PopupMenu::getPosition()
{
    return _menu->getPosition();
}

void PopupMenu::setRotation(osg::Quat rot)
{
    _menu->setRotation(rot);
}

osg::Quat PopupMenu::getRotation()
{
    return _menu->getRotation();
}

void PopupMenu::setTransform(osg::Matrix m)
{
    _menu->setTransform(m);
}

osg::Matrix PopupMenu::getTransform()
{
    return _menu->getTransform();
}

void PopupMenu::setScale(float scale)
{
    _menu->setScale(scale);
}

float PopupMenu::getScale()
{
    return _menu->getScale();
}

void PopupMenu::setVisible(bool b)
{
    _menu->setVisible(b);
}

bool PopupMenu::isVisible()
{
    return _menu->isVisible();
}

bool PopupMenu::init()
{
    return true;
}

void PopupMenu::updateStart()
{
    _menu->updateStart();
}

bool PopupMenu::processEvent(InteractionEvent * event)
{
    return _menu->processEvent(event);
}

bool PopupMenu::processIsect(IsectInfo & isect, int hand)
{
    return _menu->processIsect(isect,hand);
}

void PopupMenu::updateEnd()
{
    _menu->updateEnd();
}

void PopupMenu::itemDelete(MenuItem * item)
{
    _menu->itemDelete(item);
}
