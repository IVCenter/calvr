/**
 * @file DialogPanel.h
 */
#ifndef DIALOG_PANEL_H
#define DIALOG_PANEL_H

#include <menu/PopupMenu.h>
#include <menu/MenuText.h>

#include <string>

namespace cvr
{

/**
 * @brief A PopupMenu with an updateable text field
 */
class DialogPanel : public PopupMenu
{
    public:
        /**
         * @brief Constructor
         * @param menuWidth maximum width of the text before wrapping
         * @param title title for the PopupMenu
         * @param configTag location in config file for initial position/rotation/scale
         */
        DialogPanel(float menuWidth, std::string title, std::string configTag = "");
        virtual ~DialogPanel();

        /**
         * @brief Set the menu text
         */
        void setText(std::string text);

        /**
         * @brief Get the menu text
         */
        std::string getText();

    protected:
        std::string _text; ///< menu text
        MenuText * _menuText; ///< menu item for text
        float _menuWidth; ///< maximum width of text
};

}

#endif
