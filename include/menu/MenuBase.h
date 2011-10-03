/**
 * @file MenuBase.h
 */
#ifndef CALVR_MENU_BASE_H
#define CALVR_MENU_BASE_H

#include <kernel/InteractionManager.h>
#include <menu/SubMenu.h>
#include <util/Intersection.h>

namespace cvr
{

/**
 * @brief Pure virtual base class for a menu implementation
 */
class MenuBase
{
    public:
        MenuBase()
        {
        }
        virtual ~MenuBase()
        {
        }

        /**
         * @brief Set a submenu to be the root for this menu
         */
        virtual void setMenu(SubMenu * menu) = 0;

        /**
         * @brief Function called right before the processIsect calls happen
         */
        virtual void updateStart() = 0;

        /**
         * @brief Handle tracker/mouse interaction with this menu geometry
         */
        virtual bool processEvent(InteractionEvent * event) = 0;

        /**
         * @brief Check to see if this isect is with an item in this menu
         * @param isect hit information from the intersection
         * @param hand Hand number for this intersection
         * @return true if the isect is in the menu, makes this menu the active menu
         */
        virtual bool processIsect(IsectInfo & isect, int hand) = 0;

        /**
         * @brief Function called right after the processIsect calls happen
         */
        virtual void updateEnd() = 0;

        /**
         * @brief Called when a MenuItem is deleted
         *
         * Lets the menu remove the item if it is present and cleanup any of its resources
         */
        virtual void itemDelete(MenuItem * item) = 0;

        /**
         * @brief Removes all items from the menu
         */
        virtual void clear() = 0;

        /**
         * @brief Removes the menu from view
         */
        virtual void close() = 0;

        /**
         * @brief Set the scale for the menu geometry
         */
        virtual void setScale(float scale) = 0;

        /**
         * @brief Get the scale for the menu geometry
         */
        virtual float getScale() = 0;
};

}

#endif
