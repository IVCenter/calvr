/**
 * @file MenuRadial.h
 */

#ifndef CALVR_MENU_RADIAL_H
#define CALVR_MENU_RADIAL_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuItem.h>

#include <string>
#include <vector>

namespace cvr
{

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Menu item for a labeled checkbox
 */
class CVRMENU_EXPORT MenuRadial : public MenuItem
{
    public:

        /**
         * @brief Constructor
         * @param text text label
         * @param value value initial state of checkbox
         */
        MenuRadial();
        virtual ~MenuRadial();

        /**
         * @brief Return CHECKBOX type
         */
        virtual MenuItemType getType()
        {
            return RADIAL;
        }

        /**
         * @brief Get the current checkbox state value
         */
        int getValue() const;

        /**
         * @brief Set the checkbox state
         */
        void setValue(const int value);

        /**
         * @brief Returns the currently active label
         */
        const std::string getActiveLabel() const;

        /**
         * @brief Returns a vector containing all the labels
         */
        const std::vector<std::string> getLabels() const;

        /**
         * @brief Get the label for indicated slice
         */
        const std::string getText(const int index) const;

        /**
         * @brief Set the label for indicated slice
         */
        void setText(const int index, const std::string & text);

        /**
         * @brief Create slices and set their labels
         */
        void setLabels(const std::vector<std::string> & l);

		/**
		 * @brief Insert a label at given index (if index not given or out of range, insert at back)
		 */
		void addLabel(const std::string & label, const int index = -1);


    protected:
        std::vector<std::string> _labels; ///< checkbox label
        int _value; ///< current checkbox state
};

/**
 * @}
 */

}

#endif
