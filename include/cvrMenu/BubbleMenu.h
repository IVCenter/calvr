/**
 * @file BubbleMenu.h
 */
#ifndef CALVR_BUBBLE_MENU_H
#define CALVR_BUBBLE_MENU_H

#include <cvrMenu/MenuBase.h>
#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>
#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuGeometry.h>
#include <cvrMenu/BubbleMenu/Lerp.h>

#include "/home/cehughes/Sound/OASClientInterface.h"
#include "/home/cehughes/Sound/OASSound.h"

#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>

#include <stack>
#include <map>

namespace cvr
{

/**
 * @brief Menu implementation that builds bubble type geometry and manages interaction for 
 * a structure of MenuItems
 */
class BubbleMenu : public MenuBase
{
        friend class BubbleMenuGeometry;
    public:
        BubbleMenu();
        virtual ~BubbleMenu();

        /**
         * @brief Method used to spawn menu
         */
        enum MenuTrigger
        {
            DOUBLECLICK,
            UPCLICK
        };

        /**
         * @brief Set the root SubMenu for this menu
         */
        virtual void setMenu(SubMenu * menu);

        /**
         * @brief Function called right before the processIsect calls happen
         */
        virtual void updateStart();

        /**
         * @brief Handle tracker/mouse interaction with this menu geometry
         */
        virtual bool processEvent(InteractionEvent * event);

        /**
         * @brief Check to see if this isect is with an item in this menu
         * @param isect geometry intersection to check
         * @param hand Hand for this intersection
         */
        virtual bool processIsect(IsectInfo & isect, int hand);

        /**
         * @brief Function called right after the processIsect calls happen
         */
        virtual void updateEnd();

        /**
         * @brief Called when a MenuItem is deleted
         */
        virtual void itemDelete(MenuItem * item);

        /**
         * @brief Removes all items from the menu
         */
        virtual void clear();

        /**
         * @brief Removes the menu from view
         */
        virtual void close();

        /**
         * @brief Set the scale for the menu geometry
         */
        virtual void setScale(float scale);

        /**
         * @brief Get the scale for the menu geometry
         */
        virtual float getScale();

       /**
         * @brief Set whether the favorites bar is always showing
         */

        virtual void setFavorites(bool showFav);

    protected:
        /**
         * @brief Update the menu geometry
         */
        void updateMenus();

        /**
         * @brief Set the currently selected menu item
         */
        void selectItem(BubbleMenuGeometry * mg);

        /**
         * @brief Open submenus from the menu root to this submenu
         */
        void openMenu(BubbleMenuSubMenuGeometry * smg);

        /**
         * @brief Close this submenu and its children submenus
         */
        void closeMenu(SubMenu * menu);


        std::string _iconDir; ///< base directory when looking for icons

        SubMenu * _myMenu; ///< root submenu for this menu
        osg::ref_ptr<osg::MatrixTransform> _menuRoot; ///< root scenegraph node for this menu
        osg::ref_ptr<osg::MatrixTransform> _menuScale; ///< menu scale node

        SubMenu * _favMenu; ///< favorites submenu for this menu
        osg::ref_ptr<osg::MatrixTransform> _favMenuRoot; ///< favorites menu scenegraph node for this menu
        osg::ref_ptr<osg::MatrixTransform> _favMenuScale; ///< favorites menu scale node

        MenuTrigger _trigger; ///< Method for spawning the menu

        int _activeHand;
        bool _menuActive; ///< if the menu is open
        BubbleMenuGeometry * _activeItem; ///< menu item currently being interacted with

        bool _clickActive; ///< if a menu item is currently being clicked on

        bool _foundItem; ///< if the isect checks have hit an item in this menu for the frame

        float _distance; ///< distance on the wand to spawn the menu
        float _height; ///< height of main menu above head
        float _scale; ///< menu scale
        int _primaryButton; ///< button for menu clicking
        int _secondaryButton; ///< button for menu spawning

        osg::Vec4 _sphereColor; ///< color of spheres
        float _border; ///< thickness of border around menu items
        float _radius; ///< radius of spheres
        float _subradius; ///< radius of circular submenu
        float _speed; ///< animation speed
        float _textSize; ///< text size

        float _timeLastButtonUp; ///< time in seconds since last button up, to distinguish double clicks from single clicks
        float _doubleClickCutoff; ///< time in seconds after button up before next button down for a double click
        float _hoverStartTime; ///< time in seconds since hover started
        float _hoverCutoff; ///< time in seconds for hover popup menu to appear

        int _tessellations; ///< tessellations of spheres

        bool _showFavMenu; ///< always show the favorites menu or not
        bool _soundEnabled; ///< whether to play sounds or not

        BubbleMenuGeometry * _activeIsect; ///< Bubble menu geometry that is being hovered over

        InteractionEvent * _prevEvent; ///< saved event to be processed after waiting for double click
        BubbleMenuGeometry * _prevActiveItem; ///< saved active item, to be processed after waiting for double click

        std::map<SubMenu*,float> _widthMap; ///< current width of the geometry of each SubMenu in this menu
        std::map<SubMenu*,osg::ref_ptr<osg::MatrixTransform> > _menuMap; ///< map of SubMenu to its geometry scenegraph root
        std::map<osg::Geode *,BubbleMenuGeometry*> _intersectMap; ///< map of menu item intersection geometry to the item
        std::map<MenuItem *,BubbleMenuGeometry *> _geometryMap; ///< map from a MenuItem to its geometry class
        std::map<SubMenu*,std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> > _menuGeometryMap; ///< map from SubMenu to geometry pair (line item, and menu head)

        std::map<MenuItem *,BubbleMenuGeometry *> _favGeometryMap; ///< map from a MenuItem to its geometry class
        std::map<BubbleMenuGeometry *, osg::ref_ptr<osg::MatrixTransform> > _maskMap; ///< map from geometry to its masking node
        std::map<BubbleMenuGeometry *, osg::ref_ptr<osg::MatrixTransform> > _favMaskMap; ///< map from geometry to its favorites menu masking node

        std::map<osg::ref_ptr<osg::MatrixTransform> , Lerp*> _lerpMap; ///< map of a node to its linear interpolation controller

        std::map<BubbleMenuGeometry*, osg::Vec3> _positionMap; ///< map of menu geometry to its original position
        std::map<BubbleMenuGeometry*, osg::Vec3> _rootPositionMap; ///< map of menu geometry to its top level parent's position

        std::stack<SubMenu*> _openMenus; ///< stack of all currently opened SubMenus

        oasclient::OASSound * click, * whoosh; ///< sound effects
};

}

#endif
