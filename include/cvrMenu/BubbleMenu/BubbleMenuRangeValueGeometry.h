#ifndef BUBBLE_MENU_RANGE_VALUE_GEOMETRY_H
#define BUBBLE_MENU_RANGE_VALUE_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

namespace cvr
{

class BubbleMenuRangeValueGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuRangeValueGeometry();
        virtual ~BubbleMenuRangeValueGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();
        virtual void processEvent(InteractionEvent * event);

        virtual void showHoverText();
        virtual void hideHoverText();

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
        osg::ref_ptr<osgText::Text3D> _label;
        osg::ref_ptr<osgText::Text> _minValue;
        osg::ref_ptr<osgText::Text> _maxValue;
        osg::ref_ptr<osgText::Text3D> _currentValue;
        osg::ref_ptr<osg::Geometry> _forwardGeometry;
        osg::ref_ptr<osg::Geometry> _backGeometry;
        osg::ref_ptr<osg::Geometry> _rvGeometry;
        osg::ref_ptr<osg::Texture2D> _forwardIcon;
        osg::ref_ptr<osg::Texture2D> _backIcon;
        osg::ref_ptr<osg::Texture2D> _rvIcon;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
