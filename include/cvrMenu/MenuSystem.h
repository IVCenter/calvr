/**
 * @file MenuSystem.h
 */
#ifndef CALVR_MENU_SYSTEM_H
#define CALVR_MENU_SYSTEM_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuSystemBase.h>
#include <cvrMenu/SubMenu.h>

class IsectInfo;

namespace cvr
{

struct InteractionEvent;
class MenuBase;
class MenuManager;

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief MenuSystem implementation for the main CalVR menu
 */
class CVRMENU_EXPORT MenuSystem : public MenuSystemBase
{
        friend class MenuManager;
    public:

        /**
         * @brief Get a static pointer to the MenuSystem
         */
        static MenuSystem * instance();

        /**
         * @brief Setup the menu
         */
        virtual bool init();

        /**
         * @brief Add an item to the main SubMenu
         */
        void addMenuItem(MenuItem * item);

        /**
         * @brief Remove an item from the main SubMenu
         */
        void removeMenuItem(MenuItem * item);

        /**
         * @brief Get the root SubMenu
         */
        SubMenu * getMenu()
        {
            return _rootMenu;
        }

        /**
         * @brief Function called before the Isect results are processed
         */
        virtual void updateStart();

        /**
         * @brief Handle tracker/mouse button events
         */
        virtual bool processEvent(InteractionEvent * event);

        /**
         * @brief Check the IsectInfo to see if it hit this menu
         */
        virtual bool processIsect(IsectInfo & isect, int hand);

        /**
         * @brief Function called after the Isect results are processed
         */
        virtual void updateEnd();

        /**
         * @brief Called when a menu item is deleted
         *
         * Lets menu remove item if needed and cleanup resources
         */
        virtual void itemDelete(MenuItem * item);

        virtual MenuType getMenuType();
        virtual bool setFavorites(bool showFav);

    protected:
        MenuSystem();
        virtual ~MenuSystem();

        static MenuSystem * _myPtr; ///< static self pointer

        SubMenu * _rootMenu; ///< root SubMenu
        MenuType _type; ///< type value for the menu implementation
        MenuBase * _menu; ///< class that implements menu geometry
};

/**
 * @}
 */

}

#endif
