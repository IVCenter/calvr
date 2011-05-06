#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <kernel/InteractionManager.h>
#include <menu/MenuSystem.h>

#include <list>

namespace cvr
{

class MenuManager
{
    friend class MenuItem;
    public:
        virtual ~MenuManager();

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
