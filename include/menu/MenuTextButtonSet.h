/**
 * @file MenuTextButtonSet.h
 */
#ifndef CALVR_MENU_TEXT_BUTTON_SET_H
#define CALVR_MENU_TEXT_BUTTON_SET_H

#include <menu/Export.h>
#include <menu/MenuItem.h>

#include <string>
#include <vector>

namespace cvr
{

/**
 * @brief Menu item for a gridded layout of state buttons with text labels
 */
class CVRMENU_EXPORT MenuTextButtonSet : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param radioButtons If true, the button set will act as radio buttons where only a max of one can be down at a time
         * @param width sets the width of the button set in the menu
         * @param rowHeight sets the height of each row of buttons in the menu
         * @param buttonsPerRow the number of buttons put into each row before moving to the next one
         */
        MenuTextButtonSet(bool radioButtons, float width, float rowHeight, int buttonsPerRow);
        
        /**
         * @brief Constructor
         * @param radioButtons If true, the button set will act as radio buttons where only a max of one can be down at a time
         * @param width sets the width of the button set in the menu
         * @param rowHeight sets the height of each row of buttons in the menu
         * @param buttonsPerRow the number of buttons put into each row before moving to the next one
         * @param buttons list of buttons to add to the item, states default to false
         */
        MenuTextButtonSet(bool radioButtons, float width, float rowHeight, int buttonsPerRow, std::vector<std::string> & buttons);
        virtual ~MenuTextButtonSet();

        /**
         * @brief returns TEXTBUTTONSET for this item
         */
        virtual MenuItemType getType()
        {
            return TEXTBUTTONSET;
        }

        /**
         * @brief Add a button with the given label to the set
         */
        void addButton(std::string button);

        /**
         * @brief Remove the button with the given label from the set
         */
        void removeButton(std::string button);

        /**
         * @brief Remove button by index number
         */
        void removeButton(int num);

        /**
         * @brief Clear all buttons from the set
         */
        void clear();

        /**
         * @brief Get if this button set acts as radio buttons
         */
        bool getRadioButtons();

        /**
         * @brief Get the number of buttons to put in each row
         */
        int getButtonsPerRow();

        /**
         * @brief Get the menu width for this button set
         */
        float getWidth();

        /**
         * @brief Get the height of each button row
         */
        float getRowHeight();

        /**
         * @brief Get the number of buttons in the set
         */
        int getNumButtons();

        /**
         * @brief Get the index number for a button with the given label
         * @return returns -1 if the button is not found
         */
        int getButtonNumber(std::string button);

        /**
         * @brief returns the label for a given button index
         * @return returns an empty string if not in the index range
         */
        const std::string & getButton(int num);

        /**
         * @brief Set the state value for a button
         * @param button label of button to set
         * @param val state value to set button to
         */
        void setValue(std::string button, bool val);

        /**
         * @brief Set the state value for a button
         * @param num index number of button
         * @param val state value to set button to
         */
        void setValue(int num, bool val);

        /**
         * @brief Get the state value of the button with the given label
         * @return returns false if label not valid
         */
        bool getValue(std::string button);

        /**
         * @brief Get the state value of the button at the given index
         * @return returns false if index not valid
         */
        bool getValue(int num);

        /**
         * @brief Returns the label of the first button with a true state
         * @return returns an empty string if all buttons are off
         *
         * Useful for radio buttons where only one can be on
         */
        std::string firstOn();

        /**
         * @brief Returns the index of the first button with a true state
         * @return returns -1 if all buttons are off
         *
         * Useful for radio buttons where only one can be on
         */
        int firstNumOn();

    protected:
        bool _radioButtons; ///< if this set should act as radio buttons
        float _width; ///< width of set in menu
        float _rowHeight; ///< height of each button row
        int _buttonsPerRow; ///< number of buttons to put in each row
        std::vector<std::string> _buttons; ///< buttons currently in the set
        std::vector<bool> _buttonStates; ///< states of buttons in the set
};

}

#endif
