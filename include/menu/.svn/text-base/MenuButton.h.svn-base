/**
 * @file MenuButton.h
 */

#ifndef CALVR_MENU_BUTTON_H
#define CALVR_MENU_BUTTON_H

#include <menu/MenuItem.h>

#include <osg/Vec4>

#include <string>

namespace cvr
{

/**
 * @brief Menu item that provides a clickable button with a text label
 */
class MenuButton : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param text label for button
         */
        MenuButton(std::string text);
        virtual ~MenuButton();

        /**
         * @brief get the text label for this button
         */
        std::string & getText();

        /**
         * @brief Returns BUTTON as this item's type
         */
        virtual MenuItemType getType();
    protected:
        std::string _text; ///< button's text label
};

}

#endif
