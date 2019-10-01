#include <cvrMenu/MenuButton.h>

using namespace cvr;

MenuButton::MenuButton(std::string text, bool indent, std::string icon) :
        MenuItem()
{
    _text = text;
    _indent = indent;
	_icon = icon;
}

MenuButton::~MenuButton()
{
}

std::string & MenuButton::getText()
{
    return _text;
}

bool MenuButton::getIndent()
{
    return _indent;
}

std::string & MenuButton::getIcon()
{
	return _icon;
}

void MenuButton::setText(std::string text)
{
    _text = text;
    setDirty(true);
}

void MenuButton::setIndent(bool b)
{
    _indent = b;
    setDirty(true);
}

void MenuButton::setIcon(std::string iconPath)
{
	_icon = iconPath;
	setDirty(true);
}

MenuItemType MenuButton::getType()
{
    return BUTTON;
}
