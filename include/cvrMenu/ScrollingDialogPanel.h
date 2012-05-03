/**
 * @file ScrollingDialogPanel.h
 */
#ifndef SCROLLING_DIALOG_PANEL_H
#define SCROLLING_DIALOG_PANEL_H

#include <cvrMenu/Export.h>
#include <cvrMenu/PopupMenu.h>
#include <cvrMenu/MenuScrollText.h>

#include <string>

namespace cvr
{

/**
 * @brief A PopupMenu with an scrolling text field
 */
class CVRMENU_EXPORT ScrollingDialogPanel : public PopupMenu
{
    public:
        /**
         * @brief Constructor
         * @param textWidth width of the text before wrapping
         * @param textRows number of rows of text to display
         * @param textScale scale factor for text size
         * @param followEnd should the end of the text be followed on appends
         * @param title title for the PopupMenu
         * @param configTag location in config file for initial position/rotation/scale
         */
        ScrollingDialogPanel(float textWidth, int textRows, float textScale, bool followEnd, std::string title, std::string configTag =
                "");
        virtual ~ScrollingDialogPanel();

        /**
         * @brief Add text to the menu
         */
        void addText(std::string text);

        /**
         * @brief Get the current menu text
         */
        const std::string & getText() { return _text; }

        /**
         * @brief Clear the menu text
         */
        void clear();

    protected:
        std::string _text; ///< menu text
        MenuScrollText * _menuScrollText; ///< menu item for text box
        float _textWidth; ///< width of text box
        int _textRows; ///< rows of text
        float _textScale; ///< scale factor for text size
};

}

#endif
