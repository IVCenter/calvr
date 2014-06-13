/**
 * @file MenuBar.h
 */

#ifndef CALVR_MENU_BAR_H
#define CALVR_MENU_BAR_H

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
 * @brief Menu item that creates a colored bar
 */
class CVRMENU_EXPORT MenuBar : public MenuItem
{
    public:
        /**
         * @brief Constructor
         * @param color bar color
         */
        MenuBar(osg::Vec4 color, float mult = 1.0);
        virtual ~MenuBar();

        void setColor(osg::Vec4 color);
        void setMultiplier(float mult);
        
        osg::Vec4 getColor();
        float getMultiplier();

        /**
         * @brief Returns BAR as this item's type
         */
        virtual MenuItemType getType();
    protected:
        osg::Vec4 _color;
        float _mult;
};

/**
 * @}
 */

}

#endif
