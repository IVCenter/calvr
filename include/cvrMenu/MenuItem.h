/**
 * @file MenuItem.h
 */

#ifndef CALVR_MENU_ITEM_H
#define CALVR_MENU_ITEM_H

#include <cvrMenu/Export.h>

#include <string>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

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
    BAR,
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
            _hoverText = "";
            _extraData = 0;
            _parent = 0;
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
         * @brief A user supplied pointer that can be used to make decisions during menu callbacks
         */
        void setExtraData(void* data)
        {
            _extraData = data;
        }

        /**
         * @brief Get any user supplied pointer or 0 otherwise
         */
        void* getExtraData()
        {
            return _extraData;
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

        void setHoverText(std::string text)
        {
            _hoverText = text;
        }

        std::string & getHoverText()
        {
            return _hoverText;
        }

        const MenuItem* getParent() const
        {
            return _parent;
        }

        void setParent(const MenuItem* parent)
        {
            _parent = parent;
        }

    protected:
        MenuCallback * _callback; ///< Pointer to class object to receive update callbacks from this item
        void* _extraData; ///< Extra data pointer supplied by the user for decision making in callbacks
        bool _dirty; ///< Has this item changed
        std::string _hoverText;
        const MenuItem* _parent;
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
         * @param item MenuItem for this event
         * @param handID Hand performing the interaction
         */
        virtual void menuCallback(MenuItem * item, int handID)
        {
            menuCallback(item);
        }

        /**
         * @brief Deprecated - menuCallback(item,handID) calls this function if
         *        not redefined
         */
        virtual void menuCallback(MenuItem * item)
        {
        }
};

/**
 * @}
 */

}

#endif
