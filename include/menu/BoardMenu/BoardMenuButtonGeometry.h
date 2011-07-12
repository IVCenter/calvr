#ifndef BOARD_MENU_BUTTON_GEOMETRY_H
#define BOARD_MENU_BUTTON_GEOMETRY_H

#include <menu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

class BoardMenuButtonGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuButtonGeometry();
        virtual ~BoardMenuButtonGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
};

}

#endif
