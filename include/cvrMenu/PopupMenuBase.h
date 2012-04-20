/**
 * @file PopupMenuBase.h
 */
#ifndef POPUP_MENU_BASE_H
#define POPUP_MENU_BASE_H

#include <cvrMenu/MenuBase.h>

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/Matrix>

namespace cvr
{

/**
 * @brief interface that defines pure virtual functions that must be defined for
 * a PopupMenu Implementation
 */
class PopupMenuBase : public MenuBase
{
    public:
        virtual ~PopupMenuBase()
        {
        }

        /**
         * @brief Set the position of the PopupMenu Implementation
         */
        virtual void setPosition(osg::Vec3 pos) = 0;

        /**
         * @brief Get the position of the PopupMenu Implementation
         */
        virtual osg::Vec3 getPosition() = 0;

        /**
         * @brief Set the rotation transform for the PopupMenu Implementation
         */
        virtual void setRotation(osg::Quat rot) = 0;

        /**
         * @brief Get the PopupMenu Implementation rotation
         */
        virtual osg::Quat getRotation() = 0;

        /**
         * @brief Set the full PopupMenu Implementation transform
         */
        virtual void setTransform(osg::Matrix m) = 0;

        /**
         * @brief Get the full PopupMenu Implementation transform
         */
        virtual osg::Matrix getTransform() = 0;

        /**
         * @brief Set if the menu should be visible
         */
        virtual void setVisible(bool v) = 0;

        /**
         * @brief Get if the menu is currently visible
         */
        virtual bool isVisible() = 0;
};

}
#endif
