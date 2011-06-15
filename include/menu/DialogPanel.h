#ifndef DIALOG_PANEL_H
#define DIALOG_PANEL_H

#include <menu/PopupMenu.h>
#include <menu/MenuText.h>

#include <string>

namespace cvr
{

class DialogPanel : public PopupMenu
{
    public:
        DialogPanel(float menuWidth, std::string title, std::string configTag = "");
        virtual ~DialogPanel();

        void setText(std::string text);
        std::string getText();

    protected:
        std::string _text;
        MenuText * _menuText;
        float _menuWidth;
};

}

#endif
