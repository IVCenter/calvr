#ifndef CALVR_TILED_WALL_SCENE_OBJECT
#define CALVR_TILED_WALL_SCENE_OBJECT

#include <cvrKernel/SceneObject.h>

namespace cvr
{

class CVRKERNEL_EXPORT TiledWallSceneObject : public SceneObject
{
    public:
        TiledWallSceneObject(std::string name, bool navigation, bool movable,
                bool clip, bool contextMenu, bool showBounds = false);
        virtual ~TiledWallSceneObject();

        void setTiledWallMovement(bool b);
        bool getTiledWallMovement()
        {
            return _tiledWallMovement;
        }

        virtual bool processEvent(InteractionEvent * ie);
    protected:
        virtual void moveCleanup();

        bool _tiledWallMovement;
        osg::Vec3 _movePoint;
};

}

#endif
