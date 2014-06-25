#include <cvrMenu/MenuIntEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPannel.h>

#include <iostream>
#include <sstream>
#include <cstdio>

using namespace cvr;

MenuIntEntryItem::MenuIntEntryItem(std::string label, int value, MenuItemGroup::AlignmentHint hint) : MenuItemGroup(MenuItemGroup::ROW_LAYOUT,hint)
{
    _label = label;

    _numberText = new MenuButton("",false);
    _numberText->setCallback(this);
    addItem(_numberText);

    _inputPannel = new TextInputPannel("Numpad",TextInputPannel::KT_NUMPAD,"MenuSystem.EntryItemPannel");

    _enterButton = new MenuButton("Enter");
    _enterButton->setCallback(this);
    _inputPannel->addMenuItem(_enterButton);

    _inputPannel->setVisible(false);
    setValue(value);
}

MenuIntEntryItem::~MenuIntEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPannel;
}

int MenuIntEntryItem::getValue()
{
    return _value;
}

void MenuIntEntryItem::setValue(int value)
{
    _value = value;
    std::stringstream ss;
    ss << _label << " " << value;
    _numberText->setText(ss.str());
}

void MenuIntEntryItem::menuCallback(MenuItem * item, int handID)
{
    if(item == _numberText)
    {
	_inputPannel->setVisible(true);
	return;
    }

    if(item == _enterButton)
    {
	if(!_inputPannel->getText().size())
	{
	    return;
	}
	
	int temp = atol(_inputPannel->getText().c_str());
	setValue(temp);

	if(getCallback())
	{
	    getCallback()->menuCallback(this,handID);
	}
	_inputPannel->setVisible(false);
	_inputPannel->setText("");
	return;
    }
}
