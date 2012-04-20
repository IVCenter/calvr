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
         * @param menuWidth maximum width of the text before wrapping
         * @param title title for the PopupMenu
         * @param configTag location in config file for initial position/rotation/scale
         */
        ScrollingDialogPanel(float textWidth, int textrows, float textscale, bool followEnd, std::string title, std::string configTag =
                "");
        virtual ~ScrollingDialogPanel();

        /**
         * @brief Add text to the menu
         */
        void addText(std::string text);

        const std::string & getText() { return _text; }

        void clear();

    protected:
        std::string _text; ///< menu text
        MenuScrollText * _menuScrollText; ///< menu item for text
        float _textWidth; ///< maximum width of text
        int _textRows;
        float _textScale;
};

}

#endif
