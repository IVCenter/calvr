#include <cvrMenu/SubMenu.h>
#include <cvrMenu/MenuSystem.h>

using namespace cvr;

SubMenu::SubMenu(std::string name, std::string title, bool displayTitle) :
        MenuCollection()
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
	_displayTitle = displayTitle;
}

SubMenu::~SubMenu()
{
}

std::string SubMenu::getName()
{
    return _name;
}

void SubMenu::setName(std::string const& name)
{
	if (name.compare(_name) != 0)
	{
		setDirty(true);
	}
	_name = name;
}


std::string SubMenu::getTitle()
{
    return _title;
}

void SubMenu::setTitle(std::string const& title)
{
	if (title.compare(_title) != 0)
	{
		setDirty(true);
	}
	_title = title;
}

bool SubMenu::getDisplayTitle()
{
	return _displayTitle;
}

void SubMenu::setDisplayTitle(bool d)
{
	if (_displayTitle != d)
	{
		setDirty(true);
	}
	_displayTitle = d;
}
