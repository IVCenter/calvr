/**
 * @file MenuCollection.h
 */

#ifndef CALVR_MENU_COLLECTION_H
#define CALVR_MENU_COLLECTION_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

#include <osg/Vec4>

#include <vector>
#include <string>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item that represents a submenu
 */
class CVRMENU_EXPORT MenuCollection : public MenuItem
{
    public:
        MenuCollection();
        virtual ~MenuCollection();

        /**
         * @brief Returns true because this is a menu collection
         */
        virtual bool isCollection()
        {
            return true;
        }

        /**
         * @brief Must be defined in implemented collections
         */
        virtual MenuItemType getType() = 0;

        virtual bool isDirty();

        /**
         * @brief Add a MenuItem to this collection
         * @param item MenuItem to add
         */
        virtual void addItem(MenuItem * item);

        /**
         * @brief Add a MenuItem to this collection at a given position
         * @param item MenuItem to add
         * @param position location in collection to add this item
         *
         * If the position is out of range, item is added to the end
         */
        virtual void addItem(MenuItem * item, int position);

        /**
         * @brief Remove a MenuItem from this collection
         * @param item MenuItem to remove
         */
        virtual void removeItem(MenuItem * item);

        /**
         * @brief Get the position of a MenuItem in the MenuCollection
         * @param item MenuItem to locate
         * @return position of MenuItem
         *
         * If the item is not in the collection, -1 is returned
         */
        virtual int getItemPosition(MenuItem * item);

        /**
         * @brief Get this menu's child at location i
         */
        virtual MenuItem * getChild(int i);

        /**
         * @brief Get a list of all this collection's children
         */
        virtual std::vector<MenuItem *> & getChildren();

        /**
         * @brief Get the number of children for this collection
         */
        virtual int getNumChildren();

    protected:
        std::vector<MenuItem*> _children; ///< list of children for this collection
};

/**
 * @}
 */

}

#endif
