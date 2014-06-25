#include <cvrMenu/SubMenu.h>
#include <cvrMenu/MenuSystem.h>

using namespace cvr;

SubMenu::SubMenu(std::string name, std::string title) :
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
}

SubMenu::~SubMenu()
{
}

std::string SubMenu::getName()
{
    return _name;
}

std::string SubMenu::getTitle()
{
    return _title;
}
