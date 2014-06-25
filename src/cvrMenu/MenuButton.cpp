#include <cvrMenu/MenuButton.h>

using namespace cvr;

MenuButton::MenuButton(std::string text, bool indent) :
        MenuItem()
{
    _text = text;
    _indent = indent;
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

MenuItemType MenuButton::getType()
{
    return BUTTON;
}
