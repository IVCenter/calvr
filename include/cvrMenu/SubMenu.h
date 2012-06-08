/**
 * @file SubMenu.h
 */

#ifndef CALVR_SUB_MENU_H
#define CALVR_SUB_MENU_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

#include <osg/Vec4>

#include <vector>
#include <string>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item that represents a submenu
 */
class CVRMENU_EXPORT SubMenu : public MenuItem
{
    public:
        /**
         * @brief constructor
         * @param name name to appear on the menu line
         * @param title title at top of this sub menu
         *
         * title value is set to name if empty
         */
        SubMenu(std::string name, std::string title = "");
        virtual ~SubMenu();

        /**
         * @brief Returns true because this is a submenu
         */
        virtual bool isSubMenu()
        {
            return true;
        }

        /**
         * @brief Returns that this is of SUBMENU type
         */
        virtual MenuItemType getType()
        {
            return SUBMENU;
        }

        /**
         * @brief Add a MenuItem to this submenu
         * @param item MenuItem to add
         */
        virtual void addItem(MenuItem * item);

        /**
         * @brief Add a MenuItem to this submenu at a given position
         * @param item MenuItem to add
         *
         * If the position is out of range, item is added to the end
         */
        virtual void addItem(MenuItem * item, int position);

        /**
         * @brief Remove a MenuItem from this submenu
         * @param item MenuItem to remove
         */
        virtual void removeItem(MenuItem * item);

        /**
         * @brief Get the position of a MenuItem in the SubMenu
         * @param item MenuItem to locate
         * @return position of MenuItem
         *
         * If the item is not in the menu, -1 is returned
         */
        virtual int getItemPosition(MenuItem * item);

        /**
         * @brief Get this menu's child at location i
         */
        virtual MenuItem * getChild(int i);

        /**
         * @brief Get a list of all this menu's children
         */
        virtual std::vector<MenuItem *> & getChildren();

        /**
         * @brief Get the number of children for this menu
         */
        virtual int getNumChildren();

        /**
         * @brief Get the name of this menu
         */
        std::string getName();

        /**
         * @brief Get the menu title
         */
        std::string getTitle();

    protected:
        std::vector<MenuItem*> _children; ///< list of children for this menu
        std::string _name; ///< menu name
        std::string _title; ///< menu title
};

/**
 * @}
 */

}

#endif
