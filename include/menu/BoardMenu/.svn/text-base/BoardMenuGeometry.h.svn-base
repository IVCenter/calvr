#ifndef BOARD_MENU_GEOMETRY_H
#define BOARD_MENU_GEOMETRY_H

#include <menu/MenuItem.h>

#include <kernel/InteractionManager.h>

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/Vec4>
#include <osgText/Text>

#include <string>

namespace cvr
{

class BoardMenu;

class BoardMenuGeometry
{
        friend class BoardMenu;
    public:
        BoardMenuGeometry();
        virtual ~BoardMenuGeometry();

        virtual void selectItem(bool on) = 0;
        virtual void createGeometry(MenuItem * item) = 0;
        virtual void updateGeometry() {}

        virtual void processEvent(InteractionEvent * event) = 0;

        virtual void resetIntersect(float width);

        MenuItem * getMenuItem();
        osg::Geode * getIntersect();
        float getWidth();
        float getHeight();
        osg::MatrixTransform * getNode();

    protected:
        static osg::Geometry * makeQuad(float width, float height, osg::Vec4 color,
                                 osg::Vec3 pos = osg::Vec3(0,0,0));
        static osg::Geometry * makeLine(osg::Vec3 p1, osg::Vec3 p2, osg::Vec4 color);
        osg::Texture2D * loadIcon(std::string name);
        osgText::Text * makeText(std::string text, float size, osg::Vec3 pos, osg::Vec4 color, osgText::Text::AlignmentType align = osgText::Text::LEFT_CENTER);
        static void calibrateTextSize(float textSize);

        static std::string _iconDir;
        float _width;
        float _height;
        osg::Geode * _intersect;
        osg::ref_ptr<osg::MatrixTransform> _node;
        MenuItem * _item;

        static osg::Vec4 _textColor;
        static osg::Vec4 _textColorSelected;
        static osg::Vec4 _backgroundColor;
        static float _boarder;
        static float _iconHeight;
        static float _textSize;
        static osg::ref_ptr<osgText::Font> _font;
};

BoardMenuGeometry * createGeometry(MenuItem * item, bool head = false);

}

#endif
