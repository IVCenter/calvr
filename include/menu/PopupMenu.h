/**
 * @file PopupMenu.h
 */

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include <menu/MenuManager.h>
#include <menu/PopupMenuBase.h>

#include <osg/Vec3>
#include <osg/Quat>

#include <string>

namespace cvr
{

/**
 * @brief Creates a popup menu that can hold all standard MenuItem members
 */
class PopupMenu : public MenuSystemBase
{
    friend class MenuManager;
    public:
        PopupMenu(std::string title, std::string configTag = "");
        virtual ~PopupMenu();

        void addMenuItem(MenuItem * item);
        void removeMenuItem(MenuItem * item);

        void setPosition(osg::Vec3 pos);
        osg::Vec3 getPosition();
        void setRotation(osg::Quat rot);
        osg::Quat getRotation();
        void setTransform(osg::Matrix m);
        osg::Matrix getTransform();

        void setScale(float scale);
        float getScale();

        void setVisible(bool b);
        bool isVisible();

    protected:
        virtual bool init();
        virtual void updateStart();
        virtual bool processEvent(InteractionEvent * event);
        virtual bool processIsect(IsectInfo & isect, bool mouse);
        virtual void updateEnd();
        virtual void itemDelete(MenuItem * item);

        SubMenu * _rootMenu;
        MenuType _type;
        PopupMenuBase * _menu;
        std::string _title;
        std::string _configName;
};

}

#endif
