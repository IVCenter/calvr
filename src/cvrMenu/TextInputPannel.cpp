#include <cvrMenu/TextInputPannel.h>
#include <cvrMenu/MenuButton.h>

#include <cctype>

using namespace cvr;

TextInputPannel::TextInputPannel(std::string title, KeyboardType kt, std::string configTag) : PopupMenu(title,configTag)
{
    _textItem = new MenuText("",1.0,false);
    addMenuItem(_textItem);

    _textBar = new MenuBar(osg::Vec4(1.0,1.0,1.0,1.0));
    addMenuItem(_textBar);

    if(kt == KT_QWERTY_NUM)
    {
	makeNumberRow();
    }
    else
    {
	_numberRow = NULL;
    }

    if(kt == KT_QWERTY || kt == KT_QWERTY_NUM)
    {
	makeQWERTY();
    }

    if(kt == KT_NUMPAD)
    {
	makeNumpad();
    }

    _optionGroup = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    addMenuItem(_optionGroup);

    if(kt != KT_NUMPAD)
    {
	_shiftButton = new MenuButton("Shift",false);
	_shiftButton->setCallback(this);
	_optionGroup->addItem(_shiftButton);
    }
    else
    {
	_shiftButton = NULL;
    }
    _backButton = new MenuButton("Backspace",false);
    _backButton->setCallback(this);
    _optionGroup->addItem(_backButton);

    setVisible(false);
}

TextInputPannel::~TextInputPannel()
{
}

void TextInputPannel::addCustomRow(std::vector<std::string> & row)
{
    MenuItemGroup * mig = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);

    for(int i = 0; i < row.size(); ++i)
    {
	MenuButton * mb = new MenuButton(row[i],false);
	mb->setCallback(this);
	mig->addItem(mb);
    }

    int pos = _rootMenu->getItemPosition(_rowGroup);
    if(pos >= 0)
    {
	_rootMenu->addItem(mig,pos+1);
    }
    else
    {
	_rootMenu->addItem(mig);
    }
}

void TextInputPannel::setText(std::string text)
{
    _text = text;
    _textItem->setText(_text);
}

std::string TextInputPannel::getText()
{
    return _text;
}

void TextInputPannel::menuCallback(MenuItem * item)
{
    if(item == _shiftButton)
    {
	for(int i = 0; i < _colGroups.size(); ++i)
	{
	    for(int j = 0; j < _colGroups[i]->getChildren().size(); ++j)
	    {
		MenuButton * mb = dynamic_cast<MenuButton*>(_colGroups[i]->getChild(j));
		if(mb)
		{
		    if(mb->getText().size() == 1 && isalpha(mb->getText()[0]))
		    {
			std::string temptxt;
			if(isupper(mb->getText()[0]))
			{
			    temptxt = tolower(mb->getText()[0]);
			}
			else
			{
			    temptxt = toupper(mb->getText()[0]);
			}
			mb->setText(temptxt);
		    }
		}
	    }
	}
	return;
    }

    if(item == _backButton)
    {
	if(_text.size())
	{
	    _text.erase(_text.size()-1);
	    _textItem->setText(_text);
	}
	return;
    }

    MenuButton * button = dynamic_cast<MenuButton*>(item);
    if(button)
    {
	_text += button->getText();
	_textItem->setText(_text);
	return;
    }

    PopupMenu::menuCallback(item);
}

void TextInputPannel::makeNumberRow()
{
    _numberRow = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    addMenuItem(_numberRow);

    MenuButton * mb;
    mb = new MenuButton("1",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("2",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("3",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("4",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("5",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("6",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("7",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("8",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("9",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
    mb = new MenuButton("0",false);
    mb->setCallback(this);
    _numberRow->addItem(mb);
}

void TextInputPannel::makeQWERTY()
{
    _rowGroup = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    addMenuItem(_rowGroup);

    MenuItemGroup * tempItem;
    MenuButton * tempButton;

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("Q",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("A",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("Z",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("W",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("S",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("X",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("E",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("D",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("C",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("R",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("F",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("V",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("T",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("G",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("B",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("Y",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("H",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("N",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("U",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("J",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("M",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("I",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("K",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton(",",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("O",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("L",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton(".",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("P",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton(";",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("/",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);
}

void TextInputPannel::makeNumpad()
{
     _rowGroup = new MenuItemGroup(MenuItemGroup::ROW_LAYOUT);
    addMenuItem(_rowGroup);

    MenuItemGroup * tempItem;
    MenuButton * tempButton;

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("7",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("4",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("1",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("-",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("8",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("5",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("2",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("0",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);

    tempItem = new MenuItemGroup(MenuItemGroup::COL_LAYOUT);
    tempButton = new MenuButton("9",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("6",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton("3",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    tempButton = new MenuButton(".",false);
    tempButton->setCallback(this);
    tempItem->addItem(tempButton);
    _rowGroup->addItem(tempItem);
    _colGroups.push_back(tempItem);
}
