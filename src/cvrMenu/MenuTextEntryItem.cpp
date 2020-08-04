#include <cvrMenu/MenuTextEntryItem.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/TextInputPanel.h>

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

    _inputPanel = new TextInputPanel("Keyboard",TextInputPanel::KT_QWERTY_NUM,"MenuSystem.EntryItemPanel");

    _enterRow = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    _inputPanel->addMenuItem(_enterRow);

    _enterButton = new MenuButton("Enter",false);
    _enterButton->setCallback(this);
    _enterRow->addItem(_enterButton);

    _inputPanel->setVisible(false);
    setText(text);
}

MenuTextEntryItem::~MenuTextEntryItem()
{
    delete _numberText;
    delete _enterButton;
    delete _inputPanel;
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
    _inputPanel->setSearchList(list,numDisplayResults);
}

void MenuTextEntryItem::menuCallback(MenuItem * item, int handID)
{
    if(item == _numberText)
    {
	_inputPanel->setVisible(true);
	return;
    }

    if(item == _enterButton)
    {
	setText(_inputPanel->getText());

	if(getCallback())
	{
	    getCallback()->menuCallback(this,handID);
	}
	_inputPanel->setVisible(false);
	_inputPanel->setText("");
	return;
    }
}
