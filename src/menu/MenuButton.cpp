#include <menu/MenuButton.h>

using namespace cvr;

MenuButton::MenuButton(std::string text) :
        MenuItem()
{
    _text = text;
}

MenuButton::~MenuButton()
{
}

std::string & MenuButton::getText()
{
    return _text;
}

void MenuButton::setText(std::string text)
{
    _text = text;
    setDirty(true);
}

MenuItemType MenuButton::getType()
{
    return BUTTON;
}
