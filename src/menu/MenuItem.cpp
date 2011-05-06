#include <menu/MenuItem.h>

#include <menu/MenuSystem.h>

using namespace cvr;

MenuItem::~MenuItem()
{
    MenuSystem::instance()->itemDelete(this);
}
