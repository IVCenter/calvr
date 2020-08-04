/**
 * @file BoardMenuRangeValueGeometry.h
 */
#ifndef BOARD_MENU_RANGE_VALUE_GEOMETRY_H
#define BOARD_MENU_RANGE_VALUE_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu range value
 */
class BoardMenuRangeValueGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuRangeValueGeometry();
        virtual ~BoardMenuRangeValueGeometry();

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

        osg::ref_ptr<osg::Group> _group;
        osg::ref_ptr<osg::Group> _groupSelected;
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osg::Geode> _geodeForwardIcon;
        osg::ref_ptr<osg::Geode> _geodeBackIcon;
        osg::ref_ptr<osgText::Text> _label;
        osg::ref_ptr<osgText::Text> _minValue;
        osg::ref_ptr<osgText::Text> _maxValue;
        osg::ref_ptr<osgText::Text> _currentValue;
        osg::ref_ptr<osg::Geometry> _forwardGeometry;
        osg::ref_ptr<osg::Geometry> _backGeometry;
        osg::ref_ptr<osg::Geometry> _rvGeometry;
        osg::ref_ptr<osg::Texture2D> _forwardIcon;
        osg::ref_ptr<osg::Texture2D> _backIcon;
        osg::ref_ptr<osg::Texture2D> _rvIcon;
};

/**
 * @}
 */

}

#endif
