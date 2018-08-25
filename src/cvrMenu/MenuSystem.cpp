#include <cvrMenu/MenuSystem.h>
#include <cvrMenu/BoardMenu.h>
#include <cvrMenu/MenuBase.h>
#include <cvrMenu/SubMenu.h>
#include <cvrKernel/InteractionManager.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrUtil/Intersection.h>
#include <cvrMenu/BubbleMenu.h>

#include <iostream>

using namespace cvr;

MenuSystem * MenuSystem::_myPtr = NULL;

MenuSystem::MenuSystem()
{
    _menu = NULL;
}

MenuSystem::~MenuSystem()
{
    if(_menu)
    {
        delete _menu;
    }
}

MenuSystem * MenuSystem::instance()
{
    if(!_myPtr)
    {
        _myPtr = new MenuSystem();
    }
    return _myPtr;
}

bool MenuSystem::init()
{
    _rootMenu = new SubMenu("CalVR");

    std::string menuType = ConfigManager::getEntry("type","MenuSystem",
            "BOARDMENU");
    if(menuType == "BOARDMENU")
    {
        _type = BOARDMENU;
        _menu = new BoardMenu();
    }
    else if(menuType == "BUBBLEMENU")
    {
        _type = BUBBLEMENU;
        _menu = new BubbleMenu();
        std::cout << "Bubble menu loaded." << std::endl;
    }
    else
    {
        std::cerr << "No menu of type " << menuType << std::endl;
        return false;
    }

    _menu->setMenu(_rootMenu);

    return true;
}

void MenuSystem::addMenuItem(MenuItem * item)
{
    _rootMenu->addItem(item);
}

void MenuSystem::removeMenuItem(MenuItem * item)
{
    _rootMenu->removeItem(item);
}

void MenuSystem::updateStart()
{
    if(_menu)
    {
        _menu->updateStart();
    }
}

bool MenuSystem::processEvent(InteractionEvent * event)
{
    if(_menu)
    {
        cvr::BoardMenu* menu = dynamic_cast< cvr::BoardMenu *>(_menu);
        return menu->debugFunc(event);
//        return menu->processEvent(event);
    }
    return false;
}

bool MenuSystem::processIsect(IsectInfo & isect, int hand)
{
    if(_menu)
    {
        return _menu->processIsect(isect,hand);
    }
    return false;
}

void MenuSystem::updateEnd()
{
    if(_menu)
    {
        _menu->updateEnd();
    }
}

void MenuSystem::itemDelete(MenuItem * item)
{
    if(_menu)
    {
        _menu->itemDelete(item);
    }
}

MenuType MenuSystem::getMenuType()
{
    return _type;
}

bool MenuSystem::setFavorites(bool showFav)
{
    if(_type != BUBBLEMENU)
        return false;

    ((BubbleMenu*)_menu)->setFavorites(showFav);
    return true;
}

