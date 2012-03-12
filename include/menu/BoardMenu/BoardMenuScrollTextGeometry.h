#ifndef BOARD_MENU_SCROLL_TEXT_GEOMETRY_H
#define BOARD_MENU_SCROLL_TEXT_GEOMETRY_H

#include <menu/BoardMenu/BoardMenuGeometry.h>

#include <osgText/Text>
#include <osg/Geode>
#include <osg/MatrixTransform>

#include <map>
#include <string>

namespace cvr
{

class MenuTextButtonSet;

class BoardMenuScrollTextGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuScrollTextGeometry();
        virtual ~BoardMenuScrollTextGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);

        virtual void processEvent(InteractionEvent * event);

        virtual void updateGeometry();

        virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

    protected:
        void parseString(std::string & s);
        void prepDisplay(bool append);
        void makeDisplay();

        void calcSizes();
        void updateScrollbar();

        enum Arrow
        {
            UP_ARROW,
            DOWN_ARROW,
            NO_ARROW
        };

        bool _scrollActive;
        Arrow _activeArrow;

        unsigned int _textLength;
        int _lastVisibleRow;
        osg::ref_ptr<osgText::Text> _text;
        osg::ref_ptr<osg::Geode> _textGeode;
        osg::ref_ptr<osg::Texture2D> _upIcon;
        osg::ref_ptr<osg::Texture2D> _upIconSelected;
        osg::ref_ptr<osg::Texture2D> _downIcon;
        osg::ref_ptr<osg::Texture2D> _downIconSelected;
        osg::ref_ptr<osg::Geode> _upGeode;
        osg::ref_ptr<osg::Geode> _downGeode;
        osg::ref_ptr<osg::Geode> _scrollbarGeode;
        osg::ref_ptr<osg::Geometry> _scrollbarGeometry;
        osg::ref_ptr<osg::MatrixTransform> _scrollbarMT;

        std::string _display;

        std::list< std::pair< std::string, float > > _words, _wordsraw;
        std::vector< std::list< std::pair< std::string, float > >::iterator > _rowindex;
        int _lines;

        int _rows;
        float _textScale;
        float _maxHeight;
        float _baselineHeight;
        float _spaceSize;
        float _textWidth;

        double _scrollDelay;
        double _currentScrollDelay;
        double _scrollTimer;
        double _timePerScroll;

        bool _scrollHit;
        osg::Vec3 _scrollPoint;
        float _scrollRows;
};

}

#endif
