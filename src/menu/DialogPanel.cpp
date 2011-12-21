#include <menu/DialogPanel.h>

#include <iostream>

using namespace cvr;

DialogPanel::DialogPanel(float menuWidth, std::string title,
        std::string configTag) :
        PopupMenu(title,configTag)
{
    _menuWidth = menuWidth;
    _text = "";
    _menuText = new MenuText(_text,1.0,false,_menuWidth);
    addMenuItem(_menuText);
    setVisible(false);
}

DialogPanel::~DialogPanel()
{
    removeMenuItem(_menuText);
    setVisible(false);
    delete _menuText;
}

void DialogPanel::setText(std::string text)
{
    _menuText->setText(text);
    _text = text;
}

std::string DialogPanel::getText()
{
    return _text;
}
