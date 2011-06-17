/**
 * @file MenuManager.h
 */
#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <menu/Export.h>
#include <kernel/InteractionManager.h>
#include <menu/MenuSystem.h>

#include <list>

namespace cvr
{

/**
 * @brief Manages all active MenuSystemBase menus
 */
class CVRMENU_EXPORT MenuManager
{
    friend class MenuItem;
    public:
        virtual ~MenuManager();

        /**
         * @brief Get a static pointer to the class
         */
        static MenuManager * instance();

        bool init();
        void update();
        bool processEvent(InteractionEvent * event);

        void addMenuSystem(MenuSystemBase * ms);
        void removeMenuSystem(MenuSystemBase * ms);

    protected:
        MenuManager();

        static MenuManager * _myPtr;

        bool processWithOrder(IsectInfo & isect, bool mouse);
        void updateEnd();
        void itemDelete(MenuItem * item);

        std::list<MenuSystemBase *> _menuSystemList;

        int _primaryHand;
};

}

#endif
