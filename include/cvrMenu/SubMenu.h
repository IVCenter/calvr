/**
 * @file SubMenu.h
 */

#ifndef CALVR_SUB_MENU_H
#define CALVR_SUB_MENU_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuCollection.h>

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
class CVRMENU_EXPORT SubMenu : public MenuCollection
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
         * @brief Get the name of this menu
         */
        std::string getName();

        /**
         * @brief Get the menu title
         */
        std::string getTitle();

    protected:
        std::string _name; ///< menu name
        std::string _title; ///< menu title
};

/**
 * @}
 */

}

#endif
