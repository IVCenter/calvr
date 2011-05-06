/**
 * @file MenuButton.h
 */

#ifndef CALVR_MENU_TEXT_H
#define CALVR_MENU_TEXT_H

#include <menu/MenuItem.h>

#include <osg/Vec4>

#include <string>

namespace cvr
{

/**
 * @brief Menu item that provides a text label
 */
class MenuText : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param text initial text
         */
        MenuText(std::string text);
        virtual ~MenuText();

        /**
         * @brief get the text label for this button
         */
        std::string & getText();

        void setText(std::string text);

        /**
         * @brief Returns TEXT as this item's type
         */
        virtual MenuItemType getType();
    protected:
        std::string _text; ///< item's text label
};

}

#endif
