/**
 * @file SceneManager.h
 */

#ifndef CALVR_SCENE_MANAGER_H
#define CALVR_SCENE_MANAGER_H

#include <kernel/Export.h>
#include <osg/ClipNode>
#include <osg/MatrixTransform>
#include <util/DepthPartitionNode.h>

#include <vector>
#include <map>

namespace cvr
{

class CVRViewer;
class SceneObject;
class CVRPlugin;
struct InteractionEvent;

/**
 * @brief Creates and manages the main scenegraph
 */
class CVRKERNEL_EXPORT SceneManager
{
        friend class CVRViewer;
        friend class CVRPlugin;
        friend class SceneObject;
    public:
        virtual ~SceneManager();

        /**
         * @brief Types of graphics to use to represent a pointer
         */
        enum PointerGraphicType
        {
            CONE = 0,
            NONE
        };

        /**
         * @brief Get pointer to static class instance
         */
        static SceneManager * instance();

        /**
         * @brief Creates and sets up scene
         */
        bool init();

        /**
         * @brief Do per frame operations
         */
        void update();

        /**
         * @brief Second update called after the interaction events are processed
         */
        void postEventUpdate();

        /**
         * @brief Get root node of Scene (world space root)
         */
        osg::MatrixTransform * getScene();

        /**
         * @brief Get root of object space
         */
        osg::ClipNode * getObjectsRoot();

        /**
         * @brief Get object space translation/rotation transform
         */
        const osg::MatrixTransform * getObjectTransform();

        /**
         * @brief Set the matrix for object space orientation/position
         */
        void setObjectMatrix(osg::Matrixd & mat);

        /**
         * @brief Get object space scale
         */
        double getObjectScale();

        /**
         * @brief Set object space scale
         */
        void setObjectScale(double scale);

        /**
         * @brief Get the matrix transform from world space to object space
         */
        const osg::Matrixd & getWorldToObjectTransform();

        /**
         * @brief Get the matrix transform from object space to world space
         */
        const osg::Matrixd & getObjectToWorldTransform();

        /**
         * @brief Get the root node of menu space
         */
        osg::MatrixTransform * getMenuRoot();

        /**
         * @brief Set of debug axis should be shown on hand and head locations
         */
        void setAxis(bool on);

        /**
         * @brief Set if the hand pointer graphic should be hidden
         */
        void setHidePointer(bool b);

        /**
         * @brief Get if the hand pointer graphic is hidden
         */
        bool getHidePointer()
        {
            return _hidePointer;
        }

        /**
         * @brief Get a pointer to the DepthPartitionNode for the left eye rendering
         *
         * This node is located above the sceen root but does nothing if not set to active
         */
        DepthPartitionNode * getDepthPartitionNodeLeft();

        /**
         * @brief Get a pointer to the DepthPartitionNode for the right eye rendering
         *
         * This node is located above the sceen root but does nothing if not set to active
         */
        DepthPartitionNode * getDepthPartitionNodeRight();

        /**
         * @brief Set if depth partitioning is used or not
         */
        void setDepthPartitionActive(bool active);

        /**
         * @brief Get if depth partitioning is being used
         */
        bool getDepthPartitionActive();

        /**
         * @brief Sets the scenegraph root for the viewer
         */
        void setViewerScene(CVRViewer * cvrviewer);

        /**
         * @brief Handle interaction events for SceneObjects
         * @return return true if the event should be consumed from the pipeline
         */
        bool processEvent(InteractionEvent * ie);

        /**
         * @brief Register a SceneObject with the SceneManager
         * @param object SceneObject to register
         * @param plugin optional plugin name to associate with the object
         *
         * A SceneObject must be registered before it can be attached to the scene
         */
        void registerSceneObject(SceneObject * object, std::string plugin = "");

        /**
         * @brief Unregister a SceneObject with the SceneManager
         */
        void unregisterSceneObject(SceneObject * object);

        /**
         * @brief Set which SceneObject has the open context menu
         */
        void setMenuOpenObject(SceneObject * object);

        /**
         * @brief Get the SceneObject with the open context menu
         * @return returns NULL if no menu is open
         */
        SceneObject * getMenuOpenObject();

        /**
         * @brief Close any active context menu
         */
        void closeOpenObjectMenu();

    protected:
        SceneManager();

        void initPointers();
        void initLights();
        void initSceneState();
        void initAxis();

        void updateActiveObject();
        SceneObject * findChildActiveObject(SceneObject * object,
                osg::Vec3 & start, osg::Vec3 & end);
        void removePluginObjects(CVRPlugin * plugin);

        static SceneManager * _myPtr;   ///< static self pointer

        bool _showAxis;     ///< should debug axis be shown
        bool _hidePointer;

        osg::ref_ptr<osg::MatrixTransform> _sceneRoot; ///< root node of the scene
        osg::ref_ptr<osg::Group> _actualRoot; ///< node assigned as viewer root
        osg::ref_ptr<DepthPartitionNode> _depthPartitionLeft; ///< partitions availible depth, good for large scenes, left eye
        osg::ref_ptr<DepthPartitionNode> _depthPartitionRight; ///< partitions availible depth, good for large scenes, right eye
        osg::ref_ptr<osg::MatrixTransform> _menuRoot; ///< root node for menu implementation
        osg::ref_ptr<osg::ClipNode> _objectRoot;       ///< root of object space
        osg::ref_ptr<osg::MatrixTransform> _objectTransform; ///< object space translation/rotation
        osg::ref_ptr<osg::MatrixTransform> _objectScale; ///< object space scale transform
        osg::ref_ptr<osg::Group> _axisNode;   ///< holds the debug axis geometry

        osg::Matrixd _obj2world; ///< object to world space transform
        osg::Matrixd _world2obj; ///< world to object space transform

        std::vector<osg::ref_ptr<osg::MatrixTransform> > _headAxisTransforms; ///< head location debug axis transforms
        std::vector<osg::ref_ptr<osg::MatrixTransform> > _handTransforms; ///< current hand transforms
        float _scale;                         ///< current scale of object space

        SceneObject * _menuOpenObject; ///< object with an open menu
        std::map<int,SceneObject*> _activeObjects; ///< current active SceneObject for each hand
        std::map<SceneObject*,int> _uniqueActiveObjects;
        std::map<std::string,std::vector<SceneObject*> > _pluginObjectMap; ///< set of all registered SceneObjects grouped by plugin name

        float _menuScale;
        float _menuMinDistance;
        float _menuMaxDistance;
        int _menuDefaultOpenButton;
};

}

#endif
