/**
 * @file MenuManager.h
 */
#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <menu/Export.h>

#include <list>
#include <vector>

class IsectInfo;

namespace cvr
{

struct InteractionEvent;
class MenuItem;
class MenuSystemBase;

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

        /**
         * @brief Setup main menu
         */
        bool init();

        /**
         * @brief Update all active MenuSystemBase instances
         */
        void update();

        /**
         * @brief Have menus process interaction events
         */
        bool processEvent(InteractionEvent * event);

        /**
         * @brief Add an instance of a menu system
         */
        void addMenuSystem(MenuSystemBase * ms);

        /**
         * @brief Remove an instance of a menu system
         */
        void removeMenuSystem(MenuSystemBase * ms);

    protected:
        MenuManager();

        static MenuManager * _myPtr; ///< static self pointer

        bool processWithOrder(IsectInfo & isect, bool mouse);
        void updateEnd();
        void itemDelete(MenuItem * item);

        std::list<MenuSystemBase *> _menuSystemList;
        std::vector<MenuSystemBase*> _handLastMenuSystem;

        int _primaryHand;
};

}

#endif
