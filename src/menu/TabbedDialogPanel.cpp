#include <menu/TabbedDialogPanel.h>
#include <menu/MenuText.h>

using namespace cvr;

TabbedDialogPanel::TabbedDialogPanel(float menuWidth, float rowHeight, int buttonsPerRow, std::string title, std::string configName)
{
    _menuWidth = menuWidth;
    _title = title;
    _configName = configName;

    _popupMenu = new PopupMenu(title, configName);
    _popupMenu->setVisible(false);

    _textButtonSet = new MenuTextButtonSet(true, _menuWidth, rowHeight, buttonsPerRow);
    _textButtonSet->setCallback(this);

    _popupMenu->addMenuItem(_textButtonSet);

    _activeTab = "";
}

TabbedDialogPanel::~TabbedDialogPanel()
{
}

void TabbedDialogPanel::addTextTab(std::string name, std::string text)
{
    if(name.empty())
    {
	return;
    }

    if(!_menuItemMap[name])
    {
	_menuItemMap[name] = new MenuText(text, 1.0, false, _menuWidth);
	_textButtonSet->addButton(name);
    }
}

void TabbedDialogPanel::addTextureTab(std::string name, std::string file)
{
}

void TabbedDialogPanel::addTextureTab(std::string name, osg::Texture2D & texture)
{
}

void TabbedDialogPanel::updateTabWithText(std::string name, std::string text)
{
    if(_menuItemMap.find(name) != _menuItemMap.end())
    {
	MenuText * mt = dynamic_cast<MenuText*>(_menuItemMap[name]);
	if(mt)
	{
	    mt->setText(text);
	}
	else
	{
	    //change to text tab
	}
    }
}

void TabbedDialogPanel::updateTabWithTexture(std::string name, std::string file)
{
}

void TabbedDialogPanel::updateTabWithTexture(std::string name, osg::Texture2D & texture)
{
}

void TabbedDialogPanel::removeTab(std::string name)
{
    std::map<std::string,MenuItem*>::iterator it;
    if((it = _menuItemMap.find(name)) != _menuItemMap.end())
    {
	for(int i = 0; i < _textButtonSet->getNumButtons(); i++)
	{
	    if(name == _textButtonSet->getButton(i))
	    {
		if(_textButtonSet->getValue(i))
		{
		    _popupMenu->removeMenuItem(it->second);
		}

		_textButtonSet->removeButton(i);

		break;
	    }
	}

	delete it->second;
	_menuItemMap.erase(name);
    }

    if(name == _activeTab)
    {
	_activeTab = "";
    }
}

int TabbedDialogPanel::getNumTabs()
{
    return _textButtonSet->getNumButtons();
}

std::string TabbedDialogPanel::getTabName(int tab)
{
    return _textButtonSet->getButton(tab);
}

void TabbedDialogPanel::setVisible(bool v)
{
    _popupMenu->setVisible(v);
}

bool TabbedDialogPanel::isVisible()
{
    return _popupMenu->isVisible();
}

void TabbedDialogPanel::clear()
{
    for(int i = 0; i < _textButtonSet->getNumButtons(); i++)
    {
	if(_textButtonSet->getValue(i))
	{
	    _popupMenu->removeMenuItem(_menuItemMap[_textButtonSet->getButton(i)]);
	    break;
	}
    }

    _textButtonSet->clear();

    for(std::map<std::string,MenuItem*>::iterator it = _menuItemMap.begin(); it != _menuItemMap.end(); it++)
    {
	delete it->second;
    }

    _menuItemMap.clear();

    _activeTab = "";
}

void TabbedDialogPanel::menuCallback(MenuItem * item)
{
    if(item == _textButtonSet)
    {
	if(!_activeTab.empty())
	{
	    if(_menuItemMap.find(_activeTab) != _menuItemMap.end())
	    {
		_popupMenu->removeMenuItem(_menuItemMap[_activeTab]);
	    }
	    _activeTab = "";
	}

	int atab = _textButtonSet->firstNumOn();

	if(atab >= 0)
	{
	    _activeTab = _textButtonSet->getButton(atab);
	    _popupMenu->addMenuItem(_menuItemMap[_activeTab]);
	}
    }
}
