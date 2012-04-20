#include <cvrMenu/MenuItem.h>
#include <cvrMenu/MenuManager.h>

using namespace cvr;

MenuItem::~MenuItem()
{
    MenuManager::instance()->itemDelete(this);
}
