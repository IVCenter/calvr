#ifndef CALVR_MENU_TEXT_ENTRY_ITEM_H
#define CALVR_MENU_TEXT_ENTRY_ITEM_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItemGroup.h>

#include <string>

namespace cvr
{

class MenuButton;
class TextInputPannel;

class CVRMENU_EXPORT MenuTextEntryItem : public MenuItemGroup, public MenuCallback
{
    public:
        MenuTextEntryItem(std::string label, std::string text, MenuItemGroup::AlignmentHint hint = MenuItemGroup::ALIGN_LEFT);
        virtual ~MenuTextEntryItem();

        std::string getText();
        void setText(std::string text);

        virtual void menuCallback(MenuItem * item, int handID);
    protected:
        std::string _label;
        MenuButton * _numberText;
        TextInputPannel * _inputPannel;
        MenuButton * _enterButton;
        std::string _text;
};

}

#endif
