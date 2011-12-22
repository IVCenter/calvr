#ifndef CALVR_SUB_MENU_CLOSABLE_H
#define CALVR_SUB_MENU_CLOSABLE_H

#include <menu/SubMenu.h>

namespace cvr
{

class CVRMENU_EXPORT SubMenuClosable : public SubMenu
{
    public:
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

}

#endif
