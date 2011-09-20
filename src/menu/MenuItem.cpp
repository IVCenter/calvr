#include <menu/MenuItem.h>

#include <menu/MenuManager.h>

using namespace cvr;

MenuItem::~MenuItem()
{
    MenuManager::instance()->itemDelete(this);
}
