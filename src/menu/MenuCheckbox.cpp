#include <menu/MenuCheckbox.h>

using namespace cvr;

MenuCheckbox::MenuCheckbox(std::string text, bool value) :
    MenuItem()
{
    _text = text;
    _value = value;
}

MenuCheckbox::~MenuCheckbox()
{
}

bool MenuCheckbox::getValue()
{
    return _value;
}

void MenuCheckbox::setValue(bool value)
{
    _value = value;
    setDirty(true);
}

std::string MenuCheckbox::getText()
{
    return _text;
}

void MenuCheckbox::setText(std::string text)
{
    _text = text;
    setDirty(true);
}
