#ifndef BOARD_MENU_TEXT_GEOMETRY_H
#define BOARD_MENU_TEXT_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

#include <osgText/Text>

namespace cvr
{

class BoardMenuTextGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuTextGeometry();
        virtual ~BoardMenuTextGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);

        virtual void processEvent(InteractionEvent * event);

        virtual void updateGeometry();

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osgText::Text> _text;
};

}

#endif
