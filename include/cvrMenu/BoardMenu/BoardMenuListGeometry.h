/**
 * @file BoardMenuListGeometry.h
 */
#ifndef BOARD_MENU_LIST_GEOMETRY_H
#define BOARD_MENU_LIST_GEOMETRY_H

#include <cvrMenu/MenuList.h>
#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu list
 */
class BoardMenuListGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuListGeometry();
        virtual ~BoardMenuListGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);
    protected:
        osg::Geometry * makeBackboard();

        MenuList * _listItem;

        bool _clicked;
        int _lastMouseY;
        osg::Vec3 _point;
        float _lastDistance;
        float _change;

        osg::ref_ptr<osg::Group> _group;
        osg::ref_ptr<osg::Group> _groupSelected;
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeIcon;

        osg::ref_ptr<osgText::Text> _value;
        std::vector<osg::ref_ptr<osgText::Text> > _valuesSelected;
        osg::ref_ptr<osg::Geometry> _backboard;
        osg::ref_ptr<osg::Texture2D> _bbAll;
        osg::ref_ptr<osg::Texture2D> _bbTop;
        osg::ref_ptr<osg::Texture2D> _bbMid;
        osg::ref_ptr<osg::Texture2D> _bbBot;
        float _bbWidth;

        osg::ref_ptr<osg::Geometry> _iconGeometry;
};

/**
 * @}
 */

}

#endif
