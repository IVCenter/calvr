/**
 * @file MenuButton.h
 */

#ifndef CALVR_MENU_BUTTON_H
#define CALVR_MENU_BUTTON_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

#include <osg/Vec4>

#include <string>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item that provides a clickable button with a text label
 */
class CVRMENU_EXPORT MenuButton : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param text label for button
         */
        MenuButton(std::string text, bool indent = true);
        virtual ~MenuButton();

        /**
         * @brief get the text label for this button
         */
        std::string & getText();

        bool getIndent();

        /**
         * @brief set the text label for this button
         */
        void setText(std::string text);

        void setIndent(bool b);

        /**
         * @brief Returns BUTTON as this item's type
         */
        virtual MenuItemType getType();
    protected:
        std::string _text; ///< button's text label
        bool _indent;
};

/**
 * @}
 */

}

#endif
