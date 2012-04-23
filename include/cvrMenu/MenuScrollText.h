/**
 * @file MenuScrollText.h
 */

#ifndef CALVR_MENU_SCROLL_TEXT_H
#define CALVR_MENU_SCROLL_TEXT_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

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
         * @param width width of the text box
         * @param rows number of text rows visible
         * @param sizeScale scale multiplier for the text size
         * @param indent if the text should be indented to align with checkboxes, etc
         * @param followEnd should the window scroll with new text when it is added
         */
        MenuScrollText(std::string text, float width, int rows, float sizeScale = 1.0, bool indent = false, bool followEnd = false);
        virtual ~MenuScrollText();

        /**
         * @brief Get the text for this item
         */
        std::string & getText();

        /**
         * @brief Adds text to the end of the text box
         */
        void appendText(std::string text);

        /**
         * @brief Get text that needs to be appended
         */
        std::string & getAppendText()
        {
            return _appendText;
        }

        /**
         * @brief Clears the pending append text after it has been processed
         */
        void appendDone();

        /**
         * @brief Removes all text from the text box
         */
        void clear();

        /**
         * @brief Get the current length of the text string
         */
        int getLength() { return _text.length(); }

        /**
         * @brief Get if the text should be indented to align with checkboxes, etc
         */
        bool getIndent()
        {
            return _indent;
        }

        /**
         * @brief Get if the end of the text should be followed as new text is added
         */
        bool getFollowEnd()
        {
            return _followEnd;
        }

        /**
         * @brief Set if the text should be indented
         */
        void setIndent(bool ind);

        /**
         * @brief Get the width for the text block, will wordwrap
         */
        float getWidth()
        {
            return _width;
        }

        /**
         * @brief Get the number of text rows to display
         */
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
        std::string _text; ///< item's text
        std::string _appendText; ///< text waiting to be appended
        bool _indent; ///< indent state
        bool _followEnd; ///< if the end of the text should be followed
        float _width; ///< width of text block
        int _rows; ///< number of text rows
        float _sizeScale; ///< text size scale
};

}

#endif
