/**
 * @file TabbedDialogPanel.h
 */
#ifndef TABBED_DIALOG_PANEL_H
#define TABBED_DIALOG_PANEL_H

#include <cvrMenu/Export.h>
#include <cvrMenu/PopupMenu.h>
#include <cvrMenu/MenuItem.h>

#include <osg/Texture2D>

#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace cvr
{

class MenuItem;
class MenuTextButtonSet;

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief PopupMenu you can add text and image tabs to
 */
class CVRMENU_EXPORT TabbedDialogPanel : public PopupMenu
{
    public:
        /**
         * @brief Constructor
         * @param menuWidth maximum width of the menu
         * @param rowHeight height of each row of tab buttons
         * @param buttonsPerRow number of tab buttons to put in each row
         * @param title title of PopupMenu
         * @param configTag location of initial position/rotation/scale in config file
         */
        TabbedDialogPanel(float menuWidth, float rowHeight, int buttonsPerRow,
                std::string title, std::string configTag = "");
        virtual ~TabbedDialogPanel();

        /**
         * @brief Add a new text tab to the panel
         * @param name name of tab, can't be empty
         * @param text text in tab
         *
         * If the tab exists, this does nothing
         */
        void addTextTab(std::string name, std::string text);

        /**
         * @brief Add a new image tab to the panel
         * @param name name of tab, can't be empty
         * @param file image file to load
         *
         * If the tab exists, this does nothing.  The image is sized to fit the menuWidth.
         */
        void addTextureTab(std::string name, std::string file);

        /**
         * @brief Add a new image tab to the panel
         * @param name name of tab, can't be empty
         * @param texture image texture
         * @param aspectRatio image width/height
         *
         * If the tab exists, this does nothing.  The image is sized to fit the menuWidth.
         */
        void addTextureTab(std::string name, osg::Texture2D * texture,
                float aspectRatio);

        /**
         * @brief Replace the contents of an existing tab with text
         * @param name name of tab, can't be empty
         * @param text text to put in tab
         *
         * If the tab does not exist, this does nothing.  Will replace both text and image tabs.
         */
        void updateTabWithText(std::string name, std::string text);

        /**
         * @brief Replace the contents of an existing tab with an image
         * @param name name of tab, can't be empty
         * @param file image file
         *
         * If the tab does not exist, this does nothing.  Will replace both text and image tabs.
         * The image is sized to fit the menuWidth
         */
        void updateTabWithTexture(std::string name, std::string file);

        /**
         * @brief Replace the contents of an existing tab with an image
         * @param name name of tab, can't be empty
         * @param texture image texture
         * @param aspectRatio image width/height
         *
         * If the tab does not exist, this does nothing.  Will replace both text and image tabs.
         * The image is sized to fit the menuWidth
         */
        void updateTabWithTexture(std::string name, osg::Texture2D * texture,
                float aspectRatio);

        /**
         * @brief Remove a tab from the panel
         * @param name tab name
         */
        void removeTab(std::string name);

        /**
         * @brief Get the number of tabs in the panel
         */
        int getNumTabs();

        /**
         * @brief Get the tab name by index
         * @return returns "" if index is not valid
         */
        std::string getTabName(int tab);

        /**
         * @brief Get tab number by name
         * @return returns -1 if name is not valid
         */
        int getTabNum(std::string name);

        /**
         * @brief Set the currently displayed tab by name
         *
         * An invalid name will close all tabs
         */
        void setActiveTab(std::string name);

        /**
         * @brief Set the currently displayed tab by index
         *
         * An invalid index will close all tabs
         */
        void setActiveTabNum(int index);

        /**
         * @brief Get the currenly displayed tab name
         * @return returns "" is all tabs are closed
         */
        std::string getActiveTab();

        /**
         * @brief Get the currenly displayed tab index
         * @return returns -1 is all tabs are closed
         */
        int getActiveTabNum();

        /**
         * @brief Removes all tabs from panel
         */
        void clear();

    protected:
        void menuCallback(MenuItem * item);

        MenuTextButtonSet * _textButtonSet; ///< MenuItem for tab buttons

        std::map<std::string,MenuItem *> _menuItemMap; ///< map of tab names to MenuItem

        std::string _title;
        std::string _configName;

        std::string _activeTab; ///< currently active tab name

        float _menuWidth; ///< width of dialog panel
};

/**
 * @}
 */

}

#endif
