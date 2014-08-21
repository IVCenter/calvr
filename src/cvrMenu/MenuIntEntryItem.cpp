#include <cvrMenu/MenuIntEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPanel.h>

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

    _inputPanel = new TextInputPanel("Numpad",TextInputPanel::KT_NUMPAD,"MenuSystem.EntryItemPanel");

    _enterButton = new MenuButton("Enter");
    _enterButton->setCallback(this);
    _inputPanel->addMenuItem(_enterButton);

    _inputPanel->setVisible(false);
    setValue(value);
}

MenuIntEntryItem::~MenuIntEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPanel;
}

int MenuIntEntryItem::getValue()
{
    return _value;
}

std::string MenuIntEntryItem::getLabel()
{
    return _label;
}

void MenuIntEntryItem::setValue(int value)
{
    _value = value;
    std::stringstream ss;
    ss << _label << value;
    _numberText->setText(ss.str());
}

void MenuIntEntryItem::setLabel(std::string label)
{
    _label = label;
    setValue(_value);
}

void MenuIntEntryItem::menuCallback(MenuItem * item, int handID)
{
    if(item == _numberText)
    {
	_inputPanel->setVisible(true);
	return;
    }

    if(item == _enterButton)
    {
	if(!_inputPanel->getText().size())
	{
	    return;
	}
	
	int temp = atol(_inputPanel->getText().c_str());
	setValue(temp);

	if(getCallback())
	{
	    getCallback()->menuCallback(this,handID);
	}
	_inputPanel->setVisible(false);
	_inputPanel->setText("");
	return;
    }
}
