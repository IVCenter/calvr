/**
 * @file MenuCheckbox.h
 */

#ifndef CALVR_MENU_CHECKBOX_H
#define CALVR_MENU_CHECKBOX_H

#include <menu/MenuItem.h>

#include <string>

namespace cvr
{

/**
 * @brief Menu item for a labeled checkbox
 */
class MenuCheckbox : public MenuItem
{
    public:

        /**
         * @brief Constructor
         * @text text label
         * @value value initial state of checkbox
         */
        MenuCheckbox(std::string text, bool value);
        virtual ~MenuCheckbox();

        /**
         * @brief Return CHECKBOX type
         */
        virtual MenuItemType getType()
        {
            return CHECKBOX;
        }

        /**
         * @brief Get the current checkbox state value
         */
        bool getValue();

        /**
         * @brief Set the checkbox state
         */
        void setValue(bool value);

        /**
         * @brief Get the checkbox label
         */
        std::string getText();

        /**
         * @brief Set the checkbox label
         */
        void setText(std::string text);

    protected:
        std::string _text; ///< checkbox label
        bool _value; ///< current checkbox state
};

}

#endif
