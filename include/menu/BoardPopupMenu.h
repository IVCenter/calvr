#ifndef BOARD_POPUP_MENU_H
#define BOARD_POPUP_MENU_H

#include <menu/BoardMenu.h>
#include <menu/PopupMenuBase.h>

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/Matrix>

namespace cvr
{

class BoardPopupMenu : public PopupMenuBase, public BoardMenu
{
    public:
        BoardPopupMenu();
        virtual ~BoardPopupMenu();

        virtual void setMenu(SubMenu * menu);
        virtual void updateStart();
        virtual bool processIsect(IsectInfo & isect, bool mouse);
        virtual void updateEnd();
        virtual bool processEvent(InteractionEvent * event);
        virtual void itemDelete(MenuItem * item);
        virtual void clear();
        virtual void close();

        virtual void setScale(float scale);
        virtual float getScale();

        virtual void setPosition(osg::Vec3 pos);
        virtual osg::Vec3 getPosition();
        virtual void setRotation(osg::Quat rot);
        virtual osg::Quat getRotation();
        virtual void setTransform(osg::Matrix m);
        virtual osg::Matrix getTransform();

        virtual void setVisible(bool v);
        virtual bool isVisible();

    protected:
        void updateMovement(InteractionEvent * event);

        float _moveDistance;
        osg::Vec3 _currentPoint;
        osg::Vec3 _menuPoint;
};

}
#endif
