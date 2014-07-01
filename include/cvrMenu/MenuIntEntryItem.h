#ifndef CALVR_MENU_INT_ENTRY_ITEM_H
#define CALVR_MENU_INT_ENTRY_ITEM_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItemGroup.h>

#include <string>

namespace cvr
{

class MenuButton;
class TextInputPannel;

class CVRMENU_EXPORT MenuIntEntryItem : public MenuItemGroup, public MenuCallback
{
    public:
        MenuIntEntryItem(std::string label, int value, MenuItemGroup::AlignmentHint hint = MenuItemGroup::ALIGN_LEFT);
        virtual ~MenuIntEntryItem();

        int getValue();
        std::string getLabel();
        void setValue(int value);
        void setLabel(std::string label);

        virtual void menuCallback(MenuItem * item, int handID);
    protected:
        std::string _label;
        MenuButton * _numberText;
        TextInputPannel * _inputPannel;
        MenuButton * _enterButton;
        int _value;
};

}

#endif
