/**
 * @file MenuText.h
 */

#ifndef CALVR_MENU_TEXT_H
#define CALVR_MENU_TEXT_H

#include <menu/Export.h>
#include <menu/MenuItem.h>

#include <osg/Vec4>

#include <string>

namespace cvr
{

/**
 * @brief Menu item that provides a text label
 */
class CVRMENU_EXPORT MenuText : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param text initial text
         * @param sizeScale scale multiplier for the text size
         * @param indent if the text should be indented to align with checkboxes, etc
         * @param maxWidth sets the maximum width for the text line, will wordwrap
         */
        MenuText(std::string text, float sizeScale = 1.0, bool indent = true, float maxWidth = 0.0);
        virtual ~MenuText();

        /**
         * @brief Get the text label for this item
         */
        std::string & getText();

        /**
         * @brief Set the text for this item
         */
        void setText(std::string text);

        /**
         * @brief Get if the text should be indented to align with checkboxes, etc
         */
        bool getIndent() { return _indent; }

        /**
         * @brief Set if the text should be indented
         */
        void setIndent(bool ind);

        /**
         * @brief Get the maximum width for the text block, will wordwrap
         */
        float getMaxWidth() { return _maxWidth; }

        /**
         * @brief Set the maximum width for the text block, will wordwrap
         */
        void setMaxWidth(float width);

        /**
         * @brief Get the scale value for the text size
         */
        float getSizeScale() { return _sizeScale; }

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
        bool _indent; ///< indent state
        float _maxWidth; ///< max width of text block
        float _sizeScale; ///< text size scale
};

}

#endif
