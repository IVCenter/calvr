#ifndef BOARD_MENU_RANGE_VALUE_COMPACT_GEOMETRY_H
#define BOARD_MENU_RANGE_VALUE_COMPACT_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

class BoardMenuRangeValueCompactGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuRangeValueCompactGeometry();
        virtual ~BoardMenuRangeValueCompactGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);
    protected:
        bool _sign;

        int _lastMouseX;
        int _lastMouseY;

        osg::Vec3 _point;
        osg::Vec3 _normal;
        float _lastDistance;

        float _widthLabel;
        float _widthValue;

        osg::ref_ptr<osg::Group> _group;
        osg::ref_ptr<osg::Group> _groupSelected;
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osgText::Text> _label;
        osg::ref_ptr<osgText::Text> _currentValue;
        osg::ref_ptr<osg::Geometry> _rvGeometry;
        osg::ref_ptr<osg::Texture2D> _rvIcon;
};

}

#endif
