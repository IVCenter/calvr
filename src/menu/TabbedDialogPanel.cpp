#include <menu/TabbedDialogPanel.h>
#include <menu/MenuTextButtonSet.h>
#include <menu/MenuText.h>
#include <menu/MenuImage.h>

using namespace cvr;

TabbedDialogPanel::TabbedDialogPanel(float menuWidth, float rowHeight, int buttonsPerRow, std::string title, std::string configTag) : PopupMenu(title,configTag)
{
    _menuWidth = menuWidth;
    _title = title;
    _configName = configTag;

    setVisible(false);

    _textButtonSet = new MenuTextButtonSet(true, _menuWidth, rowHeight, buttonsPerRow);
    _textButtonSet->setCallback(this);

    addMenuItem(_textButtonSet);

    _activeTab = "";
}

TabbedDialogPanel::~TabbedDialogPanel()
{
    removeMenuItem(_textButtonSet);
    setVisible(false);
    delete _textButtonSet;
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
    if(name.empty())
    {
	return;
    }

    if(!_menuItemMap[name])
    {
	MenuImage * mi = new MenuImage(file);
	_menuItemMap[name] = mi;
	float ratio = _menuWidth / mi->getWidth();
	mi->setWidth(_menuWidth);
	mi->setHeight(mi->getHeight() * ratio);
	_textButtonSet->addButton(name);
    }
}

void TabbedDialogPanel::addTextureTab(std::string name, osg::Texture2D * texture, float aspectRatio)
{
    if(name.empty())
    {
	return;
    }

    if(!_menuItemMap[name])
    {
	_menuItemMap[name] = new MenuImage(texture, _menuWidth, _menuWidth * (1.0 / aspectRatio));
	_textButtonSet->addButton(name);
    }
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
	    if(name == _activeTab)
	    {
		removeMenuItem(_menuItemMap[name]);
	    }
	    delete _menuItemMap[name];
	    _menuItemMap[name] = new MenuText(text, 1.0, false, _menuWidth);

	    if(name == _activeTab)
	    {
		addMenuItem(_menuItemMap[name]);
	    }
	}
    }
}

void TabbedDialogPanel::updateTabWithTexture(std::string name, std::string file)
{
    if(_menuItemMap.find(name) != _menuItemMap.end())
    {
	MenuImage * mi = dynamic_cast<MenuImage*>(_menuItemMap[name]);
	if(mi)
	{
	    mi->setImage(file);
	    float ratio = _menuWidth / mi->getWidth();
	    mi->setWidth(_menuWidth);
	    mi->setHeight(mi->getHeight() * ratio);
	}
	else
	{
	    if(name == _activeTab)
	    {
		removeMenuItem(_menuItemMap[name]);
	    }
	    delete _menuItemMap[name];
	    
	    MenuImage * memimg = new MenuImage(file);
	    _menuItemMap[name] = memimg;
	    float ratio = _menuWidth / memimg->getWidth();
	    memimg->setWidth(_menuWidth);
	    memimg->setHeight(memimg->getHeight() * ratio);   

	    if(name == _activeTab)
	    {
		addMenuItem(_menuItemMap[name]);
	    }
	}
    }
}

void TabbedDialogPanel::updateTabWithTexture(std::string name, osg::Texture2D * texture, float aspectRatio)
{
    if(_menuItemMap.find(name) != _menuItemMap.end())
    {
	MenuImage * mi = dynamic_cast<MenuImage*>(_menuItemMap[name]);
	if(mi)
	{
	    mi->setImage(texture, _menuWidth, _menuWidth * (1.0 / aspectRatio));
	}
	else
	{
	    if(name == _activeTab)
	    {
		removeMenuItem(_menuItemMap[name]);
	    }
	    delete _menuItemMap[name];
	    
	    _menuItemMap[name] = new MenuImage(texture, _menuWidth, _menuWidth * (1.0 / aspectRatio));

	    if(name == _activeTab)
	    {
		addMenuItem(_menuItemMap[name]);
	    }
	}
    }
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
		    removeMenuItem(it->second);
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

int TabbedDialogPanel::getTabNum(std::string name)
{
    return _textButtonSet->getButtonNumber(name);
}

void TabbedDialogPanel::clear()
{
    for(int i = 0; i < _textButtonSet->getNumButtons(); i++)
    {
	if(_textButtonSet->getValue(i))
	{
	    removeMenuItem(_menuItemMap[_textButtonSet->getButton(i)]);
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

void TabbedDialogPanel::setActiveTab(std::string name)
{
    setActiveTabNum(getTabNum(name));
}

void TabbedDialogPanel::setActiveTabNum(int index)
{
    if(_textButtonSet->firstNumOn() == index)
    {
	return;
    }

    for(int i = 0; i < _textButtonSet->getNumButtons(); i++)
    {
	_textButtonSet->setValue(i,false);
    }

    if(index >= 0 && index < _textButtonSet->getNumButtons())
    {
	_textButtonSet->setValue(index,true);
	_activeTab = _textButtonSet->getButton(index);
    }
    else
    {
	_activeTab = "";
    }

    menuCallback(_textButtonSet);
}

std::string TabbedDialogPanel::getActiveTab()
{
    return _activeTab;
}

int TabbedDialogPanel::getActiveTabNum()
{
    return getTabNum(_activeTab);
}

void TabbedDialogPanel::menuCallback(MenuItem * item)
{
    if(item == _textButtonSet)
    {
	if(!_activeTab.empty())
	{
	    if(_menuItemMap.find(_activeTab) != _menuItemMap.end())
	    {
		removeMenuItem(_menuItemMap[_activeTab]);
	    }
	    _activeTab = "";
	}

	int atab = _textButtonSet->firstNumOn();

	if(atab >= 0)
	{
	    _activeTab = _textButtonSet->getButton(atab);
	    addMenuItem(_menuItemMap[_activeTab]);
	}
    }
}
