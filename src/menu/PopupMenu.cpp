#include <menu/PopupMenu.h>
#include <menu/BoardPopupMenu.h>
#include <menu/MenuManager.h>

using namespace cvr;

PopupMenu::PopupMenu(std::string title, std::string configName)
{
    _title = title;
    _configName = configName;

    //TODO: get from somewhere
    _type = BOARDMENU;

    _menu = new BoardPopupMenu();

    _rootMenu = new SubMenu(_title,_title);
    _menu->setMenu(_rootMenu);

    //TODO: read default transform from config file
    
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

bool PopupMenu::processIsect(IsectInfo & isect, bool mouse)
{
    return _menu->processIsect(isect,mouse);
}

void PopupMenu::updateEnd()
{
    _menu->updateEnd();
}

void PopupMenu::itemDelete(MenuItem * item)
{
    _menu->itemDelete(item);
}
