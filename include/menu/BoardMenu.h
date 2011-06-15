#ifndef CALVR_BOARD_MENU_H
#define CALVR_BOARD_MENU_H

#include <menu/MenuBase.h>
#include <menu/BoardMenu/BoardMenuGeometry.h>
#include <menu/BoardMenu/BoardMenuSubMenuGeometry.h>

#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>

#include <stack>
#include <map>

namespace cvr
{

class BoardMenu : public MenuBase
{
    friend class BoardMenuGeometry;
    public:
        BoardMenu();
        virtual ~BoardMenu();

        enum MenuTrigger
        {
            DOUBLECLICK,
            UPCLICK
        };

        enum ActiveInteractor
        {
            HAND,
            MOUSE,
            NONE
        };

        virtual void setMenu(SubMenu * menu);
        virtual void updateStart();
        virtual bool processEvent(InteractionEvent * event);
        virtual bool processIsect(IsectInfo & isect, bool mouse);
        virtual void updateEnd();
        virtual void itemDelete(MenuItem * item);
        virtual void clear();
        virtual void close();

        virtual void setScale(float scale);
        virtual float getScale();

    protected:
        void updateMenus();
        //BoardMenuGeometry * createGeometry(MenuItem * item, bool head = false);

        void checkIntersection();
        void selectItem(BoardMenuGeometry * mg);
        void openMenu(BoardMenuSubMenuGeometry * smg);
        void closeMenu(SubMenu * menu);

        std::string _iconDir;

        SubMenu * _myMenu;
        osg::ref_ptr<osg::MatrixTransform> _menuRoot;
        osg::ref_ptr<osg::MatrixTransform> _menuScale;

        MenuTrigger _trigger;

        ActiveInteractor _activeInteractor;

        bool _menuActive;
        BoardMenuGeometry * _activeItem;

        bool _clickActive;

        bool _foundItem;

        float _distance;
        float _scale;
        int _primaryHand;
        int _primaryButton;
        int _secondaryHand;
        int _secondaryButton;
        int _primaryMouseButton;
        int _secondaryMouseButton;

        int _primaryIntersectHand;

        float _boarder;
        float _rootMenuWidth;

        std::map<SubMenu*,float> _widthMap;
        std::map<SubMenu*,osg::ref_ptr<osg::MatrixTransform> > _menuMap;
        std::map<osg::Geode *,BoardMenuGeometry*> _intersectMap;
        std::map<MenuItem *, BoardMenuGeometry *> _geometryMap;
        std::map<SubMenu*, std::pair<BoardMenuGeometry*,BoardMenuGeometry*> > _menuGeometryMap;

        std::stack<SubMenu*> _openMenus;
        //SubMenu * _openingMenu;
        double _openingElapse;
        double _openingThreshold;
};

}

#endif
