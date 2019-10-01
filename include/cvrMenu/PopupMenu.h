/**
 * @file PopupMenu.h
 */

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuSystemBase.h>
#include <cvrMenu/SubMenu.h>

#include <osg/Vec3>
#include <osg/Quat>

#include <string>

namespace cvr
{

class MenuManager;
class PopupMenuBase;

/**
 * @addtogroup menu
 * @{
 */

/**
 * @brief Creates a popup menu that can hold all standard MenuItem members
 */
class CVRMENU_EXPORT PopupMenu : public MenuSystemBase, public MenuCallback
{
        friend class MenuManager;
    public:
        /**
         * @brief Constructor
         * @param title title for this popup menu
         * @param configTag config location when position/orientation/scale values can be found
         * @param closable should this menu have a button that closes it
         *
         * Looks for attibutes x,y,z,h,p,r,scale in configTag
         */
        PopupMenu(std::string title, std::string configTag = "", bool closable =
                true);
        virtual ~PopupMenu();

        /**
         * @brief Add an item to the PopupMenu
         */
        void addMenuItem(MenuItem * item);

        /**
         * @brief Remove an item from the PopupMenu
         */
        void removeMenuItem(MenuItem * item);

        /**
         * @brief Set the menu position
         */
        void setPosition(osg::Vec3 pos);

        /**
         * @brief Get the menu position
         */
        osg::Vec3 getPosition();

        /**
         * @brief Set the menu rotation
         */
        void setRotation(osg::Quat rot);

        /**
         * @brief Get the menu rotation
         */
        osg::Quat getRotation();

        /**
         * @brief Set the full menu transform matrix
         */
        void setTransform(osg::Matrix m);

        /**
         * @brief Get the full menu transform matrix
         */
        osg::Matrix getTransform();

		/**
		 * @Brief Get the root osg object containing this menu
		 */
		osg::MatrixTransform* getRootObject();

        /**
         * @brief Set the menu scale
         */
        void setScale(float scale);

        /**
         * @brief Get the menu scale
         */
        float getScale();

        /**
         * @brief Set if the menu is visible
         */
        void setVisible(bool b);

        /**
         * @brief Get if the menu is visible
         */
        bool isVisible();

		/**
		 * @brief Set if the menu should be movable by the user
		 */
		void setMovable(bool m);

    protected:
        virtual bool init();
        virtual void updateStart();
        virtual bool processEvent(InteractionEvent * event);
        virtual bool processIsect(IsectInfo & isect, int hand);
        virtual void updateEnd();
        virtual void itemDelete(MenuItem * item);
        virtual void menuCallback(MenuItem * item);

        SubMenu * _rootMenu; ///< root menu item of PopupMenu
        MenuType _type; ///< type value for the internal menu geometry implementation
        PopupMenuBase * _menu; ///< PopupMenu geometry implementation
        std::string _title; ///< menu title
        std::string _configName; ///< config tag with init values
};

/**
 * @}
 */

}

#endif
