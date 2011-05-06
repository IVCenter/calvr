#include <menu/MenuText.h>

using namespace cvr;

MenuText::MenuText(std::string text) :
    MenuItem()
{
    _text = text;
}

MenuText::~MenuText()
{
}

std::string & MenuText::getText()
{
    return _text;
}

void MenuText::setText(std::string text)
{
    _text = text;
    setDirty(true);
}

MenuItemType MenuText::getType()
{
    return TEXT;
}
