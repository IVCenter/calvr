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

namespace cvr
{

class CVRViewer;

/**
 * @brief Creates and manages the main scenegraph
 */
class CVRKERNEL_EXPORT SceneManager
{
    friend class CVRViewer;
    public:
        virtual ~SceneManager();

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
         * @brief Get a pointer to the DepthPartitionNode
         *
         * This node is located above the sceen root but does nothing if not set to active
         */
        DepthPartitionNode * getDepthPartitionNode();

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

    protected:
        SceneManager();

        void initPointers();
        void initLights();
        void initSceneState();
        void initAxis();

        static SceneManager * _myPtr;   ///< static self pointer

        bool _showAxis;     ///< should debug axis be shown

        osg::ref_ptr<osg::MatrixTransform> _sceneRoot;      ///< root node of the scene
        osg::ref_ptr<DepthPartitionNode> _depthPartition;   ///< partitions availible depth, good for large scenes
        osg::ref_ptr<osg::MatrixTransform> _menuRoot;       ///< root node for menu implementation
        osg::ref_ptr<osg::ClipNode> _objectRoot;            ///< root of object space
        osg::ref_ptr<osg::MatrixTransform> _objectTransform;///< object space translation/rotation
        osg::ref_ptr<osg::MatrixTransform> _objectScale;    ///< object space scale transform
        osg::ref_ptr<osg::Group> _axisNode;                 ///< holds the debug axis geometry

        osg::Matrixd _obj2world; ///< object to world space transform
        osg::Matrixd _world2obj; ///< world to object space transform

        std::vector<osg::ref_ptr<osg::MatrixTransform> > _headAxisTransforms;   ///< head location debug axis transforms
        std::vector<osg::ref_ptr<osg::MatrixTransform> > _handTransforms;       ///< current hand transforms
        float _scale;                                                           ///< current scale of object space
};

}

#endif
