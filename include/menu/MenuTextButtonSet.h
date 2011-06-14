#ifndef CALVR_MENU_TEXT_BUTTON_SET_H
#define CALVR_MENU_TEXT_BUTTON_SET_H

#include <menu/MenuItem.h>

#include <string>
#include <vector>

namespace cvr
{

class MenuTextButtonSet : public MenuItem
{
    public:
        MenuTextButtonSet(bool radioButtons, float width, float rowHeight, int buttonsPerRow);
        MenuTextButtonSet(bool radioButtons, float width, float rowHeight, int buttonsPerRow, std::vector<std::string> & buttons);
        virtual ~MenuTextButtonSet();

        virtual MenuItemType getType()
        {
            return TEXTBUTTONSET;
        }

        void addButton(std::string button);
        void removeButton(std::string button);
        void removeButton(int num);

        void clear();

        bool getRadioButtons();
        int getButtonsPerRow();
        float getWidth();
        float getRowHeight();
        int getNumButtons();
        int getButtonNumber(std::string button);
        const std::string & getButton(int num);

        void setValue(std::string button, bool val);
        void setValue(int num, bool val);
        bool getValue(std::string button);
        bool getValue(int num);

        std::string firstOn();
        int firstNumOn();

    protected:
        bool _radioButtons;
        float _width;
        float _rowHeight;
        int _buttonsPerRow;
        std::vector<std::string> _buttons;
        std::vector<bool> _buttonStates;
};

}

#endif
