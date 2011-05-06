#ifndef CALVR_MENU_BASE_H
#define CALVR_MENU_BASE_H

#include <kernel/InteractionManager.h>
#include <menu/SubMenu.h>
#include <util/Intersection.h>

namespace cvr
{

class MenuBase
{
    public:
        MenuBase()
        {
        }
        virtual ~MenuBase()
        {
        }

        virtual void setMenu(SubMenu * menu) = 0;
        virtual void updateStart() = 0;
        virtual bool processEvent(InteractionEvent * event) = 0;
        virtual bool processIsect(IsectInfo & isect, bool mouse) = 0;
        virtual void updateEnd() = 0;
        virtual void itemDelete(MenuItem * item) = 0;
        virtual void clear() = 0;
        virtual void close() = 0;
};

}

#endif
