#include <menu/MenuScrollText.h>

using namespace cvr;

MenuScrollText::MenuScrollText(std::string text, float width, int rows, float sizeScale, bool indent, bool followEnd) :
        MenuItem()
{
    _text = text;
    _indent = indent;
    _followEnd = followEnd;
    _width = width;
    _rows = rows;
    _sizeScale = sizeScale;
}

MenuScrollText::~MenuScrollText()
{
}

std::string & MenuScrollText::getText()
{
    return _text;
}

void MenuScrollText::appendText(std::string text)
{
    _appendText.append(text);
    setDirty(true);
}

void MenuScrollText::appendDone()
{
    _text.append(_appendText);
    _appendText.clear();
}

void MenuScrollText::setIndent(bool ind)
{
    _indent = ind;
    setDirty(true);
}

void MenuScrollText::setSizeScale(float sizeScale)
{
    _sizeScale = sizeScale;
    setDirty(true);
}

MenuItemType MenuScrollText::getType()
{
    return TEXTSCROLL;
}
