#include <cvrMenu/MenuText.h>

using namespace cvr;

MenuText::MenuText(std::string text, float sizeScale, bool indent,
        float maxWidth) :
        MenuItem()
{
    _text = text;
    _indent = indent;
    _maxWidth = maxWidth;
    _sizeScale = sizeScale;
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

void MenuText::setIndent(bool ind)
{
    _indent = ind;
    setDirty(true);
}

void MenuText::setMaxWidth(float width)
{
    _maxWidth = width;
    setDirty(true);
}

void MenuText::setSizeScale(float sizeScale)
{
    _sizeScale = sizeScale;
    setDirty(true);
}

MenuItemType MenuText::getType()
{
    return TEXT;
}
