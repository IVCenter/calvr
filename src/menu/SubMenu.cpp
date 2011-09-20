#include <menu/SubMenu.h>
#include <menu/MenuSystem.h>

using namespace cvr;

SubMenu::SubMenu(std::string name, std::string title) :
    MenuItem()
{
    _name = name;
    if(title.empty())
    {
        _title = name;
    }
    else
    {
        _title = title;
    }
}

SubMenu::~SubMenu()
{
}

void SubMenu::addItem(MenuItem * item)
{
    if(!item)
    {
	return;
    }

    for(int i = 0; i < _children.size(); i++)
    {
        if(_children[i] == item)
        {
            return;
        }
    }
    _children.push_back(item);
    _dirty = true;
}

void SubMenu::removeItem(MenuItem * item)
{
    for(std::vector<MenuItem*>::iterator it = _children.begin(); it
            != _children.end(); it++)
    {
        if((*it) == item)
        {
            _children.erase(it);
	    _dirty = true;
            return;
        }
    }
}

MenuItem * SubMenu::getChild(int i)
{
    if(i < 0 || i >= _children.size())
    {
        return NULL;
    }
    return _children[i];
}

std::vector<MenuItem*> & SubMenu::getChildren()
{
    return _children;
}

int SubMenu::getNumChildren()
{
    return _children.size();
}

std::string SubMenu::getName()
{
    return _name;
}

std::string SubMenu::getTitle()
{
    return _title;
}
