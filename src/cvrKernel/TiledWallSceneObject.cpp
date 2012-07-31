#include <cvrKernel/TiledWallSceneObject.h>

#include <cvrUtil/OsgMath.h>

using namespace cvr;

TiledWallSceneObject::TiledWallSceneObject(std::string name, bool navigation, bool movable, bool clip,
                bool contextMenu, bool showBounds) : SceneObject(name,false,movable,clip,contextMenu,showBounds)
{
    // set default center to tiled wall?
}

TiledWallSceneObject::~TiledWallSceneObject()
{
}

void TiledWallSceneObject::setTiledWallMovement(bool b)
{
    if(b != _tiledWallMovement)
    {
	_tiledWallMovement = b;
	_moving = false;
	_eventActive = false;
    }
}

bool TiledWallSceneObject::processEvent(InteractionEvent * ie)
{
    if(!_tiledWallMovement)
    {
	return SceneObject::processEvent(ie);
    }
    TrackedButtonInteractionEvent * tie = ie->asTrackedButtonEvent();

    if(tie)
    {
        if(_eventActive && _activeHand != tie->getHand())
        {
            return false;
        }

        if(_movable && tie->getButton() == _moveButton)
        {
	    //sort of a hack, need to find a better way to disable navigation for this object type
	    if(getNavigationOn())
	    {
		setNavigationOn(false);
	    }

	    if(tie->getInteraction() == BUTTON_DOWN || (_moving
                    && (tie->getInteraction() == BUTTON_DRAG
                            || tie->getInteraction() == BUTTON_UP)))
	    {
		if(tie->getInteraction() == BUTTON_DOWN)
		{
		    osg::Vec3 lineP1,lineP2(0,1000.0,0),planePoint,planeNormal(0,-1,0);
		    float w;

		    lineP1 = lineP1 * tie->getTransform() * getWorldToObjectMatrix();
		    lineP2 = lineP2 * tie->getTransform() * getWorldToObjectMatrix();

		    linePlaneIntersectionRef(lineP1,lineP2,planePoint,planeNormal,_movePoint,w);

		    _eventActive = true;
		    _moving = true;
		    _activeHand = tie->getHand();
		    return true;
		}
		if(!_moving)
		{
		    return false;
		}
		// top level
		if(!_parent)
		{
		    osg::Vec3 wallPoint;
		    if(SceneManager::instance()->getPointOnTiledWall(tie->getTransform(),wallPoint))
		    {
			osg::Vec3 soPoint = _movePoint * getObjectToWorldMatrix();
			osg::Matrix m;
			m.makeTranslate(wallPoint - soPoint);
			setTransform(getTransform() * m);
		    }
		}
		else
		{
		    osg::Vec3 lineP1,lineP2(0,1000.0,0),planePoint,planeNormal(0,-1,0),intersect;
		    float w;

		    lineP1 = lineP1 * tie->getTransform() * _parent->getWorldToObjectMatrix();
		    lineP2 = lineP2 * tie->getTransform() * _parent->getWorldToObjectMatrix();

		    if(linePlaneIntersectionRef(lineP1,lineP2,planePoint,planeNormal,intersect,w))
		    {
			osg::Vec3 soPoint = _movePoint * _parent->getObjectToWorldMatrix();
			osg::Matrix m;
			m.makeTranslate(intersect - soPoint);
			setTransform(getTransform() * m);
		    }
		}

		if(tie->getInteraction() == BUTTON_UP)
                {
                    _eventActive = false;
                    _moving = false;
                    _activeHand = -2;
                }
                return true;
	    }
        }
    }
    return SceneObject::processEvent(ie);
}

void TiledWallSceneObject::moveCleanup()
{
    if(!_tiledWallMovement)
    {
	SceneObject::moveCleanup();
    }
}
