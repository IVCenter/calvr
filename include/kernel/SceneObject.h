#ifndef CALVR_SCENE_OBJECT_H
#define CALVR_SCENE_OBJECT_H

#include <menu/MenuItem.h>
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

class SceneObject : public MenuCallback
{
    friend class SceneManager;
    public:

        enum BoundsCalcMode
        {
            MANUAL = 1,
            AUTO
        };

        SceneObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
        virtual ~SceneObject();

        const std::string & getName() { return _name; }
        bool getNavigationOn();
        void setNavigationOn(bool nav);
        bool getMovable() { return _movable;}
        void setMovable(bool mov);
        bool getClipOn() { return _clip; }
        void setClipOn(bool clip);
        bool getHasContextMenu() { return _contextMenu; }
        bool getShowBounds() { return _showBounds; }
        void setShowBounds(bool bounds);

        osg::Vec3 getPosition();
        void setPosition(osg::Vec3 pos);
        osg::Quat getRotation();
        void setRotation(osg::Quat rot);
        osg::Matrix getTransform();
        void setTransform(osg::Matrix m);
        float getScale();
        void setScale(float scale);

        void attachToScene();
        void detachFromScene();

        void addChild(osg::Node * node);
        void removeChild(osg::Node * node);
        void addChild(SceneObject * so);
        void removeChild(SceneObject * so);

        void addMenuItem(MenuItem * item);
        void removeMenuItem(MenuItem * item);

        osg::Matrix getObjectToWorldMatrix();
        osg::Matrix getWorldToObjectMatrix();

        bool processEvent(InteractionEvent * ie);

        virtual void menuCallback(MenuItem * item);

        virtual void enterCallback(int handID, const osg::Matrix & mat) { std::cerr << "Object enter." << std::endl; }
        virtual void updateCallback(int handID, const osg::Matrix & mat) {}
        virtual void leaveCallback(int handID) { std::cerr << "Object leave." << std::endl; }
        virtual bool eventCallback(int type, int hand, int button, const osg::Matrix & mat) { return false; }

        void setBoundingBox(osg::BoundingBox bb);
        const osg::BoundingBox & getOrComputeBoundingBox();
        void dirtyBounds() { _boundsDirty = true; }
        void setBoundsCalcMode(BoundsCalcMode bcm) { _boundsCalcMode = bcm; }
        BoundsCalcMode getBoundsCalcMode() { return _boundsCalcMode; } 

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

        osg::ref_ptr<osg::MatrixTransform> _root;
        osg::ref_ptr<osg::ClipNode> _clipRoot;
        osg::ref_ptr<osg::MatrixTransform> _boundsTransform;
        osg::ref_ptr<osg::Geode> _boundsGeode;
        osg::Matrix _transMat, _scaleMat;

        osg::Matrix _obj2root, _root2obj;
        osg::Matrix _invTransform;

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
        int _moveMouseButton;
        int _menuMouseButton;

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
};

}

#endif
