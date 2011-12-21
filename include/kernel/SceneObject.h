/**
 * @file SceneObject.h
 */
#ifndef CALVR_SCENE_OBJECT_H
#define CALVR_SCENE_OBJECT_H

#include <kernel/Export.h>
#include <menu/MenuItem.h>
#include <menu/PopupMenu.h>
#include <kernel/SceneManager.h>
#include <kernel/InteractionManager.h>

#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Matrix>
#include <osg/BoundingBox>

#include <string>
#include <iostream>

namespace cvr
{

class MenuCheckbox;
class MenuRangeValue;

/**
 * @brief Manages a set of nodes to attach to the scene.
 *
 * Handles movement, navigation and interaction.  These objects can be nested.
 */
class CVRKERNEL_EXPORT SceneObject : public MenuCallback
{
    friend class SceneManager;
    public:

        /**
         * @brief Specifies how to get the bounding box for this object.
         *
         * MANUAL - Bounding box updates are supplied by the user, AUTO - 
         * Bounding box is calculated automatically with a visitor
         */
        enum BoundsCalcMode
        {
            MANUAL = 1,
            AUTO
        };

        /**
         * @brief Constructor
         * @param name Name of this object, if there is a context menu it will have this as a title
         * @param navigation Should the user be able to navigate through this object
         * @param movable Should this object be movable with the wand
         * @param clip Should clip planes function on this object
         * @param contextMenu Should this object have a context menu
         * @param showBounds Should the bounding box be drawn
         */
        SceneObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
        virtual ~SceneObject();

        /**
         * @brief Get the name of this object
         */
        const std::string & getName() { return _name; }

        /**
         * @brief Get if we can navigate through this object
         */
        bool getNavigationOn();

        /**
         * @brief Set if we can navigate through this object
         */
        void setNavigationOn(bool nav);

        /**
         * @brief Get if this object is movable with the wand
         */
        bool getMovable() { return _movable;}

        /**
         * @brief Set if this object is movable with the wand
         */
        void setMovable(bool mov);

        /**
         * @brief Get if this object is clippable
         */
        bool getClipOn() { return _clip; }

        /**
         * @brief Set if this object is clippable
         */
        void setClipOn(bool clip);

        /**
         * @brief Get if this object has a context menu
         */
        bool getHasContextMenu() { return _contextMenu; }

        /**
         * @brief Get if the bounding box should be drawn
         */
        bool getShowBounds() { return _showBounds; }

        /**
         * @brief Set if the bounding box should be drawn
         */
        void setShowBounds(bool bounds);

        /**
         * @brief Get the position from this object transform
         */
        osg::Vec3 getPosition();

        /**
         * @brief Set the position in this object transform
         */
        void setPosition(osg::Vec3 pos);

        /**
         * @brief Get the rotation from this object transform
         */
        osg::Quat getRotation();

        /**
         * @brief Set the rotation in this object transform
         */
        void setRotation(osg::Quat rot);

        /**
         * @brief Get the object transform
         */
        osg::Matrix getTransform();

        /**
         * @brief Set the object transform
         */
        void setTransform(osg::Matrix m);

        /**
         * @brief Get the scale from this object transform
         */
        float getScale();

        /**
         * @brief Set the scale in this object transform
         */
        void setScale(float scale);

        /**
         * @brief Attach the object to the Scene
         *
         * Note: the object must be registed with the SceneManager for this to work
         */
        void attachToScene();

        /**
         * @brief Detach the object from the Scene
         */
        void detachFromScene();

        /**
         * @brief Get it the object is attached to the Scene
         */
        bool getAttached() { return _attached; }

        /**
         * @brief Add a node to this object
         */
        void addChild(osg::Node * node);

        /**
         * @brief Remove a node from this object
         */
        void removeChild(osg::Node * node);

        /**
         * @brief Add a SceneObject as a child to this object
         */
        void addChild(SceneObject * so);

        /**
         * @brief Remove a SceneObject child from this object
         */
        void removeChild(SceneObject * so);

        /**
         * @brief Get the number of osg::Node children that are part of this object
         */
        int getNumChildNodes() { return _childrenNodes.size(); }

        /**
         * @brief Get a pointer to an osg::Node child of this object by index
         * @return NULL if out of range
         */
        osg::Node * getChildNode(int node);

        /**
         * @brief Get the number of SceneObject children that are attached to this object
         */
        int getNumChildObjects() { return _childrenObjects.size(); }

        /**
         * @brief Get a pointer to a SceneObject child of this object by index
         * @return NULL if out of range
         */
        SceneObject * getChildObject(int obj);

        /**
         * @brief Add a menu item to this object's context menu
         */
        void addMenuItem(MenuItem * item);

        /**
         * @brief Remove a menu item from this object's context menu
         */
        void removeMenuItem(MenuItem * item);

        /**
         * @brief Add a MenuCheckbox to this object's context menu that toggles 
         * the object's movability
         * @param label Label to use for the MenuItem
         */
        void addMoveMenuItem(std::string label = "Movable");

        /**
         * @brief Remove movability MenuCheckbox if it has been added to the context menu
         */
        void removeMoveMenuItem();

        /**
         * @brief Add a MenuCheckbox to this object's context menu that toggles
         * if this object can be navigated through
         * @param label Label to use for the MenuItem
         */
        void addNavigationMenuItem(std::string label = "Navigation");

        /**
         * @brief Remove navigation MenuCheckbox if it has been added to the context menu
         */
        void removeNavigationMenuItem();

        /**
         * @brief Add a MenuRangeValue to this object's context menu that controls the scale
         * of the object
         * @param label MenuRangeValue label
         * @param min Min scale
         * @param max Max scale
         * @param value Current scale value
         */
        void addScaleMenuItem(std::string label, float min, float max, float value);

        /**
         * @brief Remove scale MenuRangeValue if it has been added to the context menu
         */
        void removeScaleMenuItem();

        /**
         * @brief Get the matrix transform that goes from this object's space to world space
         */
        osg::Matrix getObjectToWorldMatrix();

        /**
         * @brief Get the matrix transform that goes from world space to this object's space
         */
        osg::Matrix getWorldToObjectMatrix();

        /**
         * @brief Process an InteractionEvent from the event pipeline
         * @return Return true if the event has been consumed
         *
         * This function manages default movement/navigation operations
         */
        virtual bool processEvent(InteractionEvent * ie);

        /**
         * @brief Handles callback events from menu interaction
         *
         * Note: This function needs to get called for default menu items to work
         */
        virtual void menuCallback(MenuItem * item);

        /**
         * @brief Callback when a pointer enters then object bounds
         * @param handID The hand number that has entered the object, if >= 0 it is a wand, if -1 it is the mouse
         * @param mat Current orientation of the hand device
         */
        virtual void enterCallback(int handID, const osg::Matrix & mat) {}

        /**
         * @brief Callback called each frame the pointer device is in the object
         * @param handID The hand number for this update, if >= 0 it is a wand, if -1 it is the mouse
         * @param mat Current orientation of the hand device
         */
        virtual void updateCallback(int handID, const osg::Matrix & mat) {}

        /**
         * @brief Callback called when a pointer leaves the object bounds
         * @param handID The hand number that has left the object, if >= 0 it is a wand, if -1 it is the mouse
         */
        virtual void leaveCallback(int handID) {}

        /**
         * @brief Callback for events not handled by the default move/navigation operations
         * @return Return true if the event is consumed
         */
        virtual bool eventCallback(InteractionEvent * ie) { return false; }

        /**
         * @brief Set the bounding box for this object
         *
         * This is only valid if the BoundsCalcMode is set to MANUAL
         */
        void setBoundingBox(osg::BoundingBox bb);

        /**
         * @brief Get the bounding box for the object
         *
         * If the BoundsCalcMode is set the AUTO and the bounds is dirty, this will compute the new bounds
         */
        const osg::BoundingBox & getOrComputeBoundingBox();

        /**
         * @brief Flag the bounding box to be updated on the next getOrComputeBoundingBox() call
         *
         * This is only valid if the BoundsCalcMode is set to AUTO
         */
        void dirtyBounds() { _boundsDirty = true; }

        /**
         * @brief Set if the bounding box should be manually or automatically computed
         */
        void setBoundsCalcMode(BoundsCalcMode bcm) { _boundsCalcMode = bcm; }

        /**
         * @brief Get the current bounding box calculation mode
         */
        BoundsCalcMode getBoundsCalcMode() { return _boundsCalcMode; }

        /**
         * @brief Close this object's context menu
         */
        void closeMenu();

    protected:
        bool getRegistered() { return _registered; }
        void setRegistered(bool reg);
        bool getEventActive() { return _eventActive; }
        int getActiveHand() { return _activeHand; }
        void setActiveHand(int hand) { _activeHand = hand; }
        void processMove(osg::Matrix & mat);
        void moveCleanup();

        void computeBoundingBox();

        bool intersectsFast(osg::Vec3 & start, osg::Vec3 & end);
        bool intersects(osg::Vec3 & start, osg::Vec3 & end, osg::Vec3 & itersect1, bool & neg1, osg::Vec3 & intersect2, bool & neg2);

        void createBoundsGeometry();
        void updateBoundsGeometry();
        void updateMatrices();
        void splitMatrix();

        void interactionCountInc();
        void interactionCountDec();

        osg::ref_ptr<osg::MatrixTransform> _root;
        osg::ref_ptr<osg::ClipNode> _clipRoot;
        osg::ref_ptr<osg::MatrixTransform> _boundsTransform;
        osg::ref_ptr<osg::Geode> _boundsGeode;
        osg::ref_ptr<osg::Geode> _boundsGeodeActive;
        osg::Matrix _transMat, _scaleMat;

        osg::Matrix _obj2root, _root2obj;
        osg::Matrix _invTransform;

        PopupMenu * _myMenu;
        MenuCheckbox * _moveMenuItem;
        MenuCheckbox * _navMenuItem;
        MenuRangeValue * _scaleMenuItem;

        std::string _name;
        bool _navigation;
        bool _movable;
        bool _clip;
        bool _contextMenu;
        bool _showBounds;
        bool _registered;
        bool _attached;
        bool _eventActive;

        int _moveButton;
        int _menuButton;

        int _activeHand;
        int _activeButton;
        bool _moving;
        osg::Matrix _lastHandMat;
        osg::Matrix _lastHandInv;
        osg::Matrix _lastobj2world;

        osg::BoundingBox _bb, _bbLocal;
        bool _boundsDirty;
        BoundsCalcMode _boundsCalcMode;

        std::vector<osg::ref_ptr<osg::Node> > _childrenNodes;
        std::vector<SceneObject*> _childrenObjects;
        SceneObject * _parent;

        int _interactionCount;
};

}

#endif
