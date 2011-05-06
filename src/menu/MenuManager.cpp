#include <menu/MenuManager.h>
#include <config/ConfigManager.h>
#include <input/TrackingManager.h>
#include <kernel/SceneManager.h>

using namespace cvr;

MenuManager * MenuManager::_myPtr = NULL;

MenuManager::MenuManager()
{
}

MenuManager::~MenuManager()
{
}

MenuManager * MenuManager::instance()
{
    if(!_myPtr)
    {
	_myPtr = new MenuManager();
    }
    return _myPtr;
}

bool MenuManager::init()
{
    MenuSystem * mainMenu = MenuSystem::instance();
    if(!mainMenu->init())
    {
	return false;
    }

    _menuSystemList.push_back(mainMenu);

    _primaryHand = ConfigManager::getInt("MenuSystem.PrimaryHand",0);
    return true;
}

void MenuManager::update()
{
    // call update on all menus
    //_mainMenu->updateStart();
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	(*it)->updateStart();
    }


    // process intersection
    osg::Vec3 pointerStart =
            TrackingManager::instance()->getHandMat(_primaryHand).getTrans(),
            pointerEnd;
    pointerEnd.set(0.0f, 10000.0f, 0.0f);
    pointerEnd = pointerEnd
            * TrackingManager::instance()->getHandMat(_primaryHand);

    std::vector<IsectInfo> isecvec =
            getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                                  pointerStart, pointerEnd);

    for(int i = 0; i < isecvec.size(); i++)
    {
	if(processWithOrder(isecvec[i],false))
	{
	    updateEnd();
	    return;
	}
    }

    // process mouse intersection
    pointerStart = InteractionManager::instance()->getMouseMat().getTrans();
    pointerEnd.set(0.0f, 10000.0f, 0.0f);
    pointerEnd = pointerEnd * InteractionManager::instance()->getMouseMat();

    isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                                    pointerStart, pointerEnd);

    for(int i = 0; i < isecvec.size(); i++)
    {
	if(processWithOrder(isecvec[i],true))
	{
	    updateEnd();
	    return;
	}
    }

    updateEnd();
    //_mainMenu->updateEnd();
}

bool MenuManager::processEvent(InteractionEvent * event)
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it)->processEvent(event))
	{
	    return true;
	}
    }
    return false;
}

void MenuManager::addMenuSystem(MenuSystemBase * ms)
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it) == ms)
	{
	    return;
	}
    }
    _menuSystemList.push_back(ms);
}

void MenuManager::removeMenuSystem(MenuSystemBase * ms)
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it) == ms)
	{
	    _menuSystemList.erase(it);
	    return;
	}
    }
}

bool MenuManager::processWithOrder(IsectInfo & isect, bool mouse)
{
    bool used = false;
    MenuSystemBase * item = NULL;

    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it)->processIsect(isect,mouse))
	{
	    used = true;
	    item = (*it);
	    _menuSystemList.erase(it);
	    break;
	}
    }

    if(used)
    {
	_menuSystemList.push_front(item);
    }

    return used;
}

void MenuManager::updateEnd()
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	(*it)->updateEnd();
    }
}
