#ifndef MENU_SYSTEM_BASE_H
#define MENU_SYSTEM_BASE_H

#include <kernel/InteractionManager.h>
#include <util/Intersection.h>

namespace cvr
{

enum MenuType
{
    BOARDMENU
};

class MenuItem;

class MenuSystemBase
{
    public:
        MenuSystemBase() {}
        virtual ~MenuSystemBase() {}

        virtual bool init() = 0;
        virtual void updateStart() = 0;
        virtual bool processEvent(InteractionEvent * event) = 0;
        virtual bool processIsect(IsectInfo & isect, bool mouse) = 0;
        virtual void updateEnd() = 0;
        virtual void itemDelete(MenuItem * item) = 0;
};

}

#endif
