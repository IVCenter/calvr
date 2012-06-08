/**
 * @file MenuList.h
 */

#ifndef CALVR_MENU_LIST_H
#define CALVR_MENU_LIST_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

#include <string>
#include <vector>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item to select a value within a range
 */
class CVRMENU_EXPORT MenuList : public MenuItem
{
    public:

        /**
         * @brief Constructor
         */
        MenuList();
        virtual ~MenuList();

        /**
         * @brief Returns List type
         */
        virtual MenuItemType getType();

        /**
         * @brief Sets the value at the given index
         * @param index Index of the string to modify
         * @param value New value to be put at the index
         */
        void setValue(const int index, const std::string & value);

        /**
         * @brief Get the value of the current index
         */
        const std::string getValue() const;

        /**
         * @brief Get the value at the given index
         * @param index Index of the desire value
         */
        const std::string getValue(int index) const;

        /**
         * @brief Sets the strings to be in the MenuList
         * @param v vector of strings to be used
         */
        void setValues(const std::vector<std::string> & v);

        /**
         * @brief Get the current value, along with a couple surrounding values
         */
        const std::vector<std::string> getValues() const;

        /**
         * @brief Gets the current index value
         */
        const int getIndex() const;

        /**
         * @brief Sets the current index
         * @param index New current index
         */
        void setIndex(const int index);

        /**
         * @brief Gets the focus margin size
         */
        const unsigned int getFocus() const;

        /**
         * @brief Sets the index to the given string, if it exists in the list
         * @param str String value that the list index should point to
         */
        void matchIndexToValue(const std::string & str);

        /**
         * @brief Sets the focus margin size
         * @param size Value of the new focus margin size
         */
        void setFocus(const unsigned int size);

        /**
         * @brief Get the size of the list
         */
        const int getListSize() const;

        /**
         * @brief Sets the sensitivity of the Menu scrolling
         * @param sensitivity Desired sensitivity (Negative value to use default.)
         */
        void setSensitivity(const float sensitivity);

        /**
         * @brief Gets the current sensitivity rating
         * @return Returns set sensitivity rating, or getListSize() otherwise.
         */
        const float getSensitivity();

    protected:
        std::vector<std::string> _values; ///< values stored in the list
        int _index; ///< current chosen index (defaults to first item in the list)
        unsigned int _focusMargin; ///<  how many values above/below the current index to return from getValues()
        float _sensitivity; ///< how sensitive the menu is to scrolling through options
};

/**
 * @}
 */

}

#endif
