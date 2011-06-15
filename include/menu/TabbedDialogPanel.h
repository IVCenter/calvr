#ifndef TABBED_DIALOG_PANEL_H
#define TABBED_DIALOG_PANEL_H

#include <menu/PopupMenu.h>
#include <menu/MenuItem.h>
#include <menu/MenuTextButtonSet.h>

#include <osg/Texture2D>

#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace cvr
{

class TabbedDialogPanel : public PopupMenu, public MenuCallback
{
    public:
        TabbedDialogPanel(float menuWidth, float rowHeight, int buttonsPerRow, std::string title, std::string configTag = "");
        virtual ~TabbedDialogPanel();

        void addTextTab(std::string name, std::string text);
        void addTextureTab(std::string name, std::string file);
        void addTextureTab(std::string name, osg::Texture2D * texture, float aspectRatio);

        void updateTabWithText(std::string name, std::string text);
        void updateTabWithTexture(std::string name, std::string file);
        void updateTabWithTexture(std::string name, osg::Texture2D * texture, float aspectRatio);

        void removeTab(std::string name);

        int getNumTabs();
        std::string getTabName(int tab);

        //void setVisible(bool v);
        //bool isVisible();

        void clear();

    protected:
        void menuCallback(MenuItem * item);

        MenuTextButtonSet * _textButtonSet;

        std::vector<std::string> _tabNames;
        std::map<std::string,MenuItem *> _menuItemMap;

        std::string _title;
        std::string _configName;

        std::string _activeTab;

        float _menuWidth;
};

}

#endif
