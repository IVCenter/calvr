/**
 * @file BoardMenu.h
 */
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

/**
 * @brief Menu implementation that builds board type geometry and manages interaction for 
 * a structure of MenuItems
 */
class BoardMenu : public MenuBase
{
    friend class BoardMenuGeometry;
    public:
        BoardMenu();
        virtual ~BoardMenu();

        /**
         * @brief Method used to spawn menu
         */
        enum MenuTrigger
        {
            DOUBLECLICK,
            UPCLICK
        };

        /**
         * @brief What is currently interacting with the menu
         */
        enum ActiveInteractor
        {
            HAND,
            MOUSE,
            NONE
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
         * @param mouse is this intersection from the mouse
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

    protected:
        /**
         * @brief Update the menu geometry
         */
        void updateMenus();

        /**
         * @brief Set the currently selected menu item
         */
        void selectItem(BoardMenuGeometry * mg);

        /**
         * @brief Open submenus from the menu root to this submenu
         */
        void openMenu(BoardMenuSubMenuGeometry * smg);

        /**
         * @brief Close this submenu and its children submenus
         */
        void closeMenu(SubMenu * menu);

        std::string _iconDir; ///< base directory when looking for icons

        SubMenu * _myMenu; ///< root submenu for this menu
        osg::ref_ptr<osg::MatrixTransform> _menuRoot; ///< root scenegraph node for this menu
        osg::ref_ptr<osg::MatrixTransform> _menuScale; ///< menu scale node

        MenuTrigger _trigger; ///< Method for spawning the menu

        ActiveInteractor _activeInteractor; ///< currently active interaction type
        int _activeHand;

        bool _menuActive; ///< if the menu is open
        BoardMenuGeometry * _activeItem; ///< menu item currently being interacted with

        bool _clickActive; ///< if a menu item is currently being clicked on

        bool _foundItem; ///< if the isect checks have hit an item in this menu for the frame

        float _distance; ///< distance on the wand to spawn the menu
        float _scale; ///< menu scale
        int _primaryHand; ///< hand for menu clicking
        int _primaryButton; ///< button for menu clicking
        int _secondaryHand; ///< hand for menu spawning
        int _secondaryButton; ///< button for menu spawning
        int _primaryMouseButton; ///< button used for mouse clicking
        int _secondaryMouseButton; ///< mouse button used to spawn menu

        int _primaryIntersectHand; ///< hand used for menu intersection

        float _boarder; ///< thickness of boarder around menu items

        std::map<SubMenu*,float> _widthMap; ///< current width of the geometry of each SubMenu in this menu
        std::map<SubMenu*,osg::ref_ptr<osg::MatrixTransform> > _menuMap; ///< map of SubMenu to its geometry scenegraph root
        std::map<osg::Geode *,BoardMenuGeometry*> _intersectMap; ///< map of menu item intersection geometry to the item
        std::map<MenuItem *, BoardMenuGeometry *> _geometryMap; ///< map from a MenuItem to its geometry class
        std::map<SubMenu*, std::pair<BoardMenuGeometry*,BoardMenuGeometry*> > _menuGeometryMap; ///< map from SubMenu to geometry pair (line item, and menu head)

        std::stack<SubMenu*> _openMenus; ///< stack of all currently opened SubMenus
};

}

#endif
