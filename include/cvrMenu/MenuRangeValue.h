/**
 * @file MenuRangeValue.h
 */

#ifndef CALVR_MENU_RANGE_VALUE_H
#define CALVR_MENU_RANGE_VALUE_H

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
 * @brief Menu item to select a value within a range
 */
class CVRMENU_EXPORT MenuRangeValue : public MenuItem
{
    public:

        /**
         * @brief Constructor
         * @param label text label for menu item
         * @param min minimum value in range
         * @param max maximum value in range
         * @param current initial value
         * @param stepsize not yet used
         */
        MenuRangeValue(std::string label, float min, float max, float current,
                float stepsize = 0.0);
        virtual ~MenuRangeValue();

        /**
         * @brief Returns RANGEVALUE type
         */
        virtual MenuItemType getType();

        /**
         * @brief Set the current value
         */
        void setValue(float value);

        /**
         * @brief Get the current value
         */
        float getValue();

        /**
         * @brief Get the text label for the menu item
         */
        std::string getLabel();

        /**
         * @brief Get the minimum range value
         */
        float getMin();

        /**
         * @brief Get the maximum range value
         */
        float getMax();

        enum CallbackType
        {
            ON_CHANGE=0,
            ON_RELEASE
        };

        void setCallbackType(const CallbackType type);

        const CallbackType getCallbackType();

    protected:
        std::string _label; ///< text label
        float _min; ///< minimum value
        float _max; ///< maximum value
        float _value; ///< current value
        float _stepSize;
        CallbackType _callbackType;
};

/**
 * @}
 */

}

#endif
