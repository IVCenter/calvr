#ifndef BOARD_MENU_CHECKBOX_GEOMETRY_H
#define BOARD_MENU_CHECKBOX_GEOMETRY_H

#include <menu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

class BoardMenuCheckboxGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuCheckboxGeometry();
        virtual ~BoardMenuCheckboxGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);
    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osg::Texture2D> _checkedIcon;
        osg::ref_ptr<osg::Texture2D> _uncheckedIcon;
};

}

#endif
