#ifndef CALVR_MENU_FLOAT_ENTRY_ITEM_H
#define CALVR_MENU_FLOAT_ENTRY_ITEM_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItemGroup.h>

#include <string>

namespace cvr
{

class MenuButton;
class TextInputPannel;

class CVRMENU_EXPORT MenuFloatEntryItem : public MenuItemGroup, public MenuCallback
{
    public:
        MenuFloatEntryItem(std::string label, float value, MenuItemGroup::AlignmentHint hint = MenuItemGroup::ALIGN_LEFT);
        virtual ~MenuFloatEntryItem();

        float getValue();
        void setValue(float value);

        virtual void menuCallback(MenuItem * item, int handID);
    protected:
        std::string _label;
        MenuButton * _numberText;
        TextInputPannel * _inputPannel;
        MenuButton * _enterButton;
        float _value;
};

}

#endif
