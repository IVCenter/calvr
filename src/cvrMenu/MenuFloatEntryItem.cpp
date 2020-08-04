#include <cvrMenu/MenuFloatEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPanel.h>

#include <iostream>
#include <sstream>
#include <cstdio>

using namespace cvr;

MenuFloatEntryItem::MenuFloatEntryItem(std::string label, float value, MenuItemGroup::AlignmentHint hint) : MenuItemGroup(MenuItemGroup::ROW_LAYOUT,hint)
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

MenuFloatEntryItem::~MenuFloatEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPanel;
}

float MenuFloatEntryItem::getValue()
{
    return _value;
}

std::string MenuFloatEntryItem::getLabel()
{
    return _label;
}

void MenuFloatEntryItem::setValue(float value)
{
    _value = value;
    std::stringstream ss;
    ss << _label << value;
    _numberText->setText(ss.str());
}

void MenuFloatEntryItem::setLabel(std::string label)
{
    _label = label;
    setValue(_value);
}

void MenuFloatEntryItem::menuCallback(MenuItem * item, int handID)
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
	
	float temp = atof(_inputPanel->getText().c_str());
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
