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
        SubMenu(std::string name, std::string title = "", bool displayTitle = true);
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

		void setName(std::string const& name);

        /**
         * @brief Get the menu title
         */
        std::string getTitle();

		void setTitle(std::string const& title);

		/**
		 * @brief Get whether the submenu is displaying the title
		 */
		bool getDisplayTitle();

		void setDisplayTitle(bool d = true);

    protected:
        std::string _name; ///< menu name
        std::string _title; ///< menu title
		bool _displayTitle; ///< whether the submenu should display the title
};

/**
 * @}
 */

}

#endif
