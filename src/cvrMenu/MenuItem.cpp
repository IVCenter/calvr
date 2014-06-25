#include <cvrMenu/MenuItem.h>
#include <cvrMenu/MenuManager.h>

using namespace cvr;

MenuItem::~MenuItem()
{
    MenuManager::instance()->itemDelete(this);
}

bool MenuItem::isDirty()
{
    return _dirty;
}

void MenuItem::setDirty(bool b)
{
    _dirty = b;
}
