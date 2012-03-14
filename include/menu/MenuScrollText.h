/**
 * @file MenuScrollText.h
 */

#ifndef CALVR_MENU_SCROLL_TEXT_H
#define CALVR_MENU_SCROLL_TEXT_H

#include <menu/Export.h>
#include <menu/MenuItem.h>

#include <osg/Vec4>

#include <string>

namespace cvr
{

/**
 * @brief Menu item that provides a scrolled text box
 */
class CVRMENU_EXPORT MenuScrollText : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param text initial text
         * @param sizeScale scale multiplier for the text size
         * @param indent if the text should be indented to align with checkboxes, etc
         * @param maxWidth sets the maximum width for the text line, will wordwrap
         */
        MenuScrollText(std::string text, float width, int rows, float sizeScale = 1.0, bool indent = false, bool followEnd = false);
        virtual ~MenuScrollText();

        /**
         * @brief Get the text label for this item
         */
        std::string & getText();

        void appendText(std::string text);

        std::string & getAppendText()
        {
            return _appendText;
        }

        void appendDone();

        void clear();

        int getLength() { return _text.length(); }

        /**
         * @brief Get if the text should be indented to align with checkboxes, etc
         */
        bool getIndent()
        {
            return _indent;
        }

        bool getFollowEnd()
        {
            return _followEnd;
        }

        /**
         * @brief Set if the text should be indented
         */
        void setIndent(bool ind);

        /**
         * @brief Get the maximum width for the text block, will wordwrap
         */
        float getWidth()
        {
            return _width;
        }

        int getRows()
        {
            return _rows;
        }

        /**
         * @brief Get the scale value for the text size
         */
        float getSizeScale()
        {
            return _sizeScale;
        }

        /**
         * @brief Set the scale value for the text size
         */
        void setSizeScale(float sizeScale);

        /**
         * @brief Returns TEXT as this item's type
         */
        virtual MenuItemType getType();
    protected:
        std::string _text; ///< item's text label
        std::string _appendText;
        bool _indent; ///< indent state
        bool _followEnd;
        float _width; ///< max width of text block
        int _rows;
        float _sizeScale; ///< text size scale
};

}

#endif
