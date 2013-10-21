/**
 * @file MenuRangeValueCompact.h
 */

#ifndef CALVR_MENU_RANGE_VALUE_COMPACT_H
#define CALVR_MENU_RANGE_VALUE_COMPACT_H

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
 * @brief Menu item to select a value within a range, more compact than the regular
 * MenuRangeValue
 */
class CVRMENU_EXPORT MenuRangeValueCompact : public MenuItem
{
    public:

        /**
         * @brief Constructor
         * @param label text label for menu item
         * @param min minimum value in range
         * @param max maximum value in range
         * @param current initial value
         * @param log should the range scale be logarithmic
         * @param base log base
         */
        MenuRangeValueCompact(std::string label, float min, float max, float current,
                bool log = false, float base = 10);
        virtual ~MenuRangeValueCompact();

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

        /**
         * @brief Get if the range scale is logarithmic
         */
        bool getIsLog();

        /**
         * @brief Get base of log scale
         */
        float getLogBase();

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
        bool _log; ///< is scale logarithmic
        float _logBase; ///< log scale base
        CallbackType _callbackType;
};

/**
 * @}
 */

}

#endif
