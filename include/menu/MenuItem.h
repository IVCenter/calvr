/**
 * @file MenuItem.h
 */

#ifndef CALVR_MENU_ITEM_H
#define CALVR_MENU_ITEM_H

#include <menu/Export.h>

#include <string>

namespace cvr
{

/**
 * @brief Used to identify the type of a menu item
 */
enum MenuItemType
{
    BUTTON,
    CHECKBOX,
    SUBMENU,
    SUBMENU_CLOSABLE,
    RANGEVALUE,
    RANGEVALUECOMPACT,
    TEXT,
    TEXTBUTTONSET,
    TEXTSCROLL,
    IMAGE,
    LIST,
    OTHER
};

class MenuCallback;

/**
 * @brief Base class for all menu items
 */
class CVRMENU_EXPORT MenuItem
{
    public:
        MenuItem()
        {
            _callback = NULL;
            _dirty = true;
        }

        virtual ~MenuItem();

        /**
         * @brief Is this a menu item with children, or not
         */
        virtual bool isSubMenu()
        {
            return false;
        }

        /**
         * @brief Gets the type of the menu item, used to identify an item from a
         *      baseclass pointer
         */
        virtual MenuItemType getType()
        {
            return OTHER;
        }

        /**
         * @brief Set the class object that should receive a callback when this item
         *      changes state
         */
        void setCallback(MenuCallback * c)
        {
            _callback = c;
        }

        /**
         * @brief Get the class object that should receive a callback when this item
         *      changes state
         */
        MenuCallback * getCallback()
        {
            return _callback;
        }

        /**
         * @brief Place to provide some extra information that may be used by a menu 
         *      system when creating the item geometry.
         */
        void setExtraInfo(std::string extra)
        {
            _extraInfo = extra;
        }

        /**
         * @brief Get any existing extra information
         */
        std::string & getExtraInfo()
        {
            return _extraInfo;
        }

        /**
         * @brief Returns if this item's status has changed since its geometry was last checked
         */
        bool isDirty()
        {
            return _dirty;
        }

        /**
         * @brief Set if this items geometry needs to be regenerated
         */
        void setDirty(bool b)
        {
            _dirty = b;
        }

    protected:
        MenuCallback * _callback; ///< Pointer to class object to receive update callbacks from this item
        std::string _extraInfo; ///< Extra information provided for the menu system
        bool _dirty; ///< Has this item changed
};

/**
 * @brief Interface class for menu item callbacks
 */
class MenuCallback
{
    public:
        MenuCallback()
        {
        }
        virtual ~MenuCallback()
        {
        }

        /**
         * @brief Called by the menu system when a registered item has an event
         */
        virtual void menuCallback(MenuItem *) = 0;
};

}

#endif
