#ifndef BOARD_MENU_ITEM_GROUP_GEOMETRY_H
#define BOARD_MENU_ITEM_GROUP_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

#include <osg/Group>

namespace cvr
{

class MenuItemGroup;
class BoardMenu;

class BoardMenuItemGroupGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuItemGroupGeometry(BoardMenu * menu);
        virtual ~BoardMenuItemGroupGeometry();

        virtual void selectItem(bool)
        {
        }
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);
        virtual void updateGeometry();
        virtual void resetIntersect(float width);

    protected:
        void updateLayout(float width);

        MenuItemGroup * _mig;
        BoardMenu * _menu;

        osg::ref_ptr<osg::Group> _group;
};

}

#endif
