#ifndef CALVR_MENU_SYSTEM_H
#define CALVR_MENU_SYSTEM_H

#include <menu/MenuSystemBase.h>
#include <menu/SubMenu.h>
#include <menu/MenuBase.h>
#include <kernel/InteractionManager.h>
#include <util/Intersection.h>

namespace cvr
{

class MenuSystem : public MenuSystemBase
{
    public:
        virtual ~MenuSystem();

        static MenuSystem * instance();

        virtual bool init();
        void addMenuItem(MenuItem * item);
        virtual void updateStart();
        virtual bool processEvent(InteractionEvent * event);
        virtual bool processIsect(IsectInfo & isect, bool mouse);
        virtual void updateEnd();
        virtual void itemDelete(MenuItem * item);

    protected:
        MenuSystem();

        static MenuSystem * _myPtr;

        SubMenu * _rootMenu;
        MenuType _type;
        MenuBase * _menu;
};

}

#endif
