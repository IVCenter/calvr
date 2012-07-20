/**
 * @file MenuSystemBase.h
 */
#ifndef MENU_SYSTEM_BASE_H
#define MENU_SYSTEM_BASE_H

#include <cvrKernel/InteractionManager.h>
#include <cvrUtil/Intersection.h>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

enum MenuType
{
    BOARDMENU,
    BUBBLEMENU
};

class MenuItem;

/**
 * @brief virtual interface for a menu system
 *
 * A MenuSystem will create and manage an instance of a class derived from MenuBase
 */
class MenuSystemBase
{
    public:
        MenuSystemBase()
        {
        }
        virtual ~MenuSystemBase()
        {
        }

        /**
         * @brief Setup the menu
         * @return true if no error
         */
        virtual bool init() = 0;

        /**
         * @brief Function called before the Isect results are processed
         */
        virtual void updateStart() = 0;

        /**
         * @brief Handle tracker/mouse button events
         */
        virtual bool processEvent(InteractionEvent * event) = 0;

        /**
         * @brief Check the IsectInfo to see if it hit this menu
         * @param isect geometry intersection to process
         * @param hand Hand for this intersection
         * @return return true if this menu claims the intersect
         */
        virtual bool processIsect(IsectInfo & isect, int hand) = 0;

        /**
         * @brief Function called after the Isect results are processed
         */
        virtual void updateEnd() = 0;

        /**
         * @brief Called when a menu item is deleted
         *
         * Lets menu remove item if needed and cleanup resources
         */
        virtual void itemDelete(MenuItem * item) = 0;
};

/**
 * @}
 */

}

#endif
