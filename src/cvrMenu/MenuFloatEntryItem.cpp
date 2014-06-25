#include <cvrMenu/MenuFloatEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPannel.h>

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

    _inputPannel = new TextInputPannel("Numpad",TextInputPannel::KT_NUMPAD,"MenuSystem.EntryItemPannel");

    _enterButton = new MenuButton("Enter");
    _enterButton->setCallback(this);
    _inputPannel->addMenuItem(_enterButton);

    _inputPannel->setVisible(false);
    setValue(value);
}

MenuFloatEntryItem::~MenuFloatEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPannel;
}

float MenuFloatEntryItem::getValue()
{
    return _value;
}

void MenuFloatEntryItem::setValue(float value)
{
    _value = value;
    std::stringstream ss;
    ss << _label << " " << value;
    _numberText->setText(ss.str());
}

void MenuFloatEntryItem::menuCallback(MenuItem * item, int handID)
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
	
	float temp = atof(_inputPannel->getText().c_str());
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
