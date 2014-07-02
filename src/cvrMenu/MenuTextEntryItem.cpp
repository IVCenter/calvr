#include <cvrMenu/MenuTextEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPannel.h>

#include <iostream>
#include <sstream>
#include <cstdio>

using namespace cvr;

MenuTextEntryItem::MenuTextEntryItem(std::string label, std::string text, MenuItemGroup::AlignmentHint hint) : MenuItemGroup(MenuItemGroup::ROW_LAYOUT,hint)
{
    _label = label;

    _numberText = new MenuButton("",false);
    _numberText->setCallback(this);
    addItem(_numberText);

    _inputPannel = new TextInputPannel("Keyboard",TextInputPannel::KT_QWERTY_NUM,"MenuSystem.EntryItemPannel");

    _enterRow = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    _inputPannel->addMenuItem(_enterRow);

    _enterButton = new MenuButton("Enter",false);
    _enterButton->setCallback(this);
    _enterRow->addItem(_enterButton);

    _inputPannel->setVisible(false);
    setText(text);
}

MenuTextEntryItem::~MenuTextEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPannel;
}

std::string MenuTextEntryItem::getText()
{
    return _text;
}

std::string MenuTextEntryItem::getLabel()
{
    return _label;
}

void MenuTextEntryItem::setText(std::string text)
{
    _text = text;
    _numberText->setText(_label + text);
}

void MenuTextEntryItem::setLabel(std::string label)
{
    _label = label;
    setText(_text);
}

void MenuTextEntryItem::setSearchList(std::vector<std::string> & list, int numDisplayResults)
{
    _inputPannel->setSearchList(list,numDisplayResults);
}

void MenuTextEntryItem::menuCallback(MenuItem * item, int handID)
{
    if(item == _numberText)
    {
	_inputPannel->setVisible(true);
	return;
    }

    if(item == _enterButton)
    {
	setText(_inputPannel->getText());

	if(getCallback())
	{
	    getCallback()->menuCallback(this,handID);
	}
	_inputPannel->setVisible(false);
	_inputPannel->setText("");
	return;
    }
}
