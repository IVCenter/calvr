/**
 * @file SubMenuClosable.h
 */

#ifndef CALVR_SUB_MENU_CLOSABLE_H
#define CALVR_SUB_MENU_CLOSABLE_H

#include <cvrMenu/SubMenu.h>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item for a submenu with a close button on it
 */
class CVRMENU_EXPORT SubMenuClosable : public SubMenu
{
    public:
        /**
         * @brief Constructor
         * @param name name to appear on the menu line
         * @param title title at top of this sub menu
         *
         * title value is set to name if empty
         */
        SubMenuClosable(std::string name, std::string title = "") :
                SubMenu(name,title)
        {
        }

        virtual ~SubMenuClosable()
        {
        }

        virtual MenuItemType getType()
        {
            return SUBMENU_CLOSABLE;
        }
};

/**
 * @}
 */

}

#endif
