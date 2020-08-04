#include <cvrMenu/ScrollingDialogPanel.h>

#include <iostream>

using namespace cvr;

ScrollingDialogPanel::ScrollingDialogPanel(float textWidth, int textrows,
        float textscale, bool followEnd, std::string title,
        std::string configTag) :
        PopupMenu(title,configTag)
{
    _textWidth = textWidth;
    _textRows = textrows;
    _textScale = textscale;
    _text = "";
    _menuScrollText = new MenuScrollText(_text,_textWidth,_textRows,_textScale,
            false,followEnd);
    addMenuItem(_menuScrollText);
    setVisible(false);
}

ScrollingDialogPanel::~ScrollingDialogPanel()
{
    removeMenuItem(_menuScrollText);
    setVisible(false);
    delete _menuScrollText;
}

void ScrollingDialogPanel::addText(std::string text)
{
    _menuScrollText->appendText(text);
    _text.append(text);
}

void ScrollingDialogPanel::clear()
{
    _menuScrollText->clear();
    _text.clear();
}
