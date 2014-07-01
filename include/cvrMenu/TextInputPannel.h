#ifndef CALVR_TEXT_INPUT_PANNEL
#define CALVR_TEXT_INPUT_PANNEL

#include <cvrMenu/Export.h>
#include <cvrMenu/PopupMenu.h>
#include <cvrMenu/MenuItemGroup.h>
#include <cvrMenu/MenuText.h>
#include <cvrMenu/MenuBar.h>
#include <cvrMenu/MenuButton.h>

#include <string>
#include <vector>

namespace cvr
{

class CVRMENU_EXPORT TextInputPannel : public PopupMenu
{
    public:
        enum KeyboardType
        {
            KT_QWERTY=0,
            KT_QWERTY_NUM,
            KT_NUMPAD
        };

        TextInputPannel(std::string title, KeyboardType kt = KT_QWERTY, std::string configTag = "");
        virtual ~TextInputPannel();

        void addCustomRow(std::vector<std::string> & row);
        void setSearchList(std::vector<std::string> & list, int numDisplayResults);

        void setText(std::string text);
        std::string getText();

        virtual void menuCallback(MenuItem * item);
    protected:
        void makeNumberRow();
        void makeQWERTY();
        void makeNumpad();

        void updateListDisplay();

        MenuItemGroup * _rowGroup;
        std::vector<MenuItemGroup*> _colGroups;
        MenuText * _textItem;
        MenuBar * _textBar;
        MenuItemGroup * _optionGroup;
        MenuButton * _shiftButton;
        MenuButton * _spaceButton;
        MenuButton * _backButton;
        MenuButton * _doneButton;
        MenuItemGroup * _numberRow;

        std::vector<MenuItemGroup*> _customRows;

        std::string _text;

        std::vector<std::string> _searchList;
        int _numDisplayResults;
        MenuItemGroup * _searchListGroup;
        std::vector<MenuButton*> _searchListButtons;
        MenuBar * _searchListBar;
};

}

#endif
