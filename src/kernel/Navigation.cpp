#include <kernel/Navigation.h>
#include <input/TrackingManager.h>
#include <kernel/SceneManager.h>
#include <kernel/ComController.h>
#include <kernel/ScreenConfig.h>

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace cvr;

Navigation * Navigation::_myPtr = NULL;

Navigation::Navigation()
{
    _activeHand = 0;
    _scale = 1.0;
}

Navigation::~Navigation()
{
}

Navigation * Navigation::instance()
{
    if(!_myPtr)
    {
        _myPtr = new Navigation();
    }
    return _myPtr;
}

bool Navigation::init()
{
    _eventActive = false;

    // TODO: read button mapping from config file
    for(int i = 0; i < TrackingManager::instance()->getNumButtons(); i++)
    {
        _buttonMap[i] = NONE;
    }
    _buttonMap[0] = DRIVE;
    _buttonMap[1] = FLY;
    _buttonMap[2] = DRIVE;
    //_buttonMap[3] = SCALE;

    return true;
}

void Navigation::setPrimaryButtonMode(NavMode nm)
{
    _buttonMap[0] = nm;
}

NavMode Navigation::getPrimaryButtonMode()
{
    return _buttonMap[0];
}

void Navigation::setScale(float scale)
{
    if(scale > 0.0)
    {
        _scale = scale;
    }
}

float Navigation::getScale()
{
    return _scale;
}

void Navigation::update()
{
    if(ComController::instance()->getIsSyncError())
    {
        return;
    }

    if(_eventActive
            && TrackingManager::instance()->getHandNavType(_activeHand)
                    == MOUSE_NAV)
    {
        processMouseNav(_eventMode,NULL);
    }
}

void Navigation::processEvent(InteractionEvent * iEvent)
{

    //TODO: use tracking system nav type to allow custom nav systems

    if(iEvent->asMouseEvent())
    {
        processMouseEvent(iEvent->asMouseEvent());
        return;
    }

    TrackedButtonInteractionEvent * event = iEvent->asTrackedButtonEvent();

    if(!event)
    {
        return;
    }

    if(event->getInteraction() == BUTTON_UP && !_eventActive)
    {
        return;
    }

    if(_eventActive
            && (_eventID != event->getButton()
                    || _activeHand != event->getHand()))
    {
        return;
    }

    //std::cerr << "Event type " << event->type << std::endl;

    if(!_eventActive
            && (event->getInteraction() == BUTTON_DOWN
                    || event->getInteraction() == BUTTON_DOUBLE_CLICK))
    {
        //std::cerr << "In button down." << std::endl;
        _eventMode = _buttonMap[event->getButton()];
        _eventID = event->getButton();
        _startScale = SceneManager::instance()->getObjectScale();
        _startXForm =
                SceneManager::instance()->getObjectTransform()->getMatrix();
        switch(_eventMode)
        {
            case WALK:
            case DRIVE:
            case FLY:
            case MOVE_WORLD:
            case SCALE:
                //std::cerr << "Starting event." << std::endl;
                _eventActive = true;
                _activeHand = event->getHand();
                _eventPos = event->getTransform().getTrans();
                _eventRot = event->getTransform().getRotate();
                break;
            case NONE:
            default:
                break;
        }
    }
    else if(_eventActive && event->getInteraction() == BUTTON_UP)
    {
        processNav(_eventMode,event->getTransform());
        _eventActive = false;
    }
    else if(_eventActive && event->getInteraction() == BUTTON_DRAG)
    {
        processNav(_eventMode,event->getTransform());
    }
}

void Navigation::processMouseEvent(MouseInteractionEvent * event)
{
    if(event->getInteraction() == BUTTON_UP && !_eventActive)
    {
        return;
    }

    if(_eventActive
            && (_eventID != event->getButton()
                    || _activeHand != event->getHand()))
    {
        return;
    }

    //std::cerr << "Event type " << event->type << std::endl;

    if(!_eventActive
            && (event->getInteraction() == BUTTON_DOWN
                    || event->getInteraction() == BUTTON_DOUBLE_CLICK))
    {
        //std::cerr << "In button down." << std::endl;
        _eventMode = _buttonMap[event->getButton()];
        _eventID = event->getButton();
        _startScale = SceneManager::instance()->getObjectScale();
        _startXForm =
                SceneManager::instance()->getObjectTransform()->getMatrix();
        switch(_eventMode)
        {
            case WALK:
            case DRIVE:
            case FLY:
            case MOVE_WORLD:
            case SCALE:
                //std::cerr << "Starting event." << std::endl;
                _eventX = event->getX();
                _eventY = event->getY();
                _activeHand = event->getHand();
                _eventActive = true;
                break;
            case NONE:
            default:
                break;
        }
    }
    else if(_eventActive && event->getInteraction() == BUTTON_UP)
    {
        processMouseNav(_eventMode,event);
        _eventActive = false;
    }
}

void Navigation::processNav(NavMode nm, osg::Matrix & mat)
{

    switch(nm)
    {
        case WALK:
        case DRIVE:
        {
            osg::Vec3 offset = -(mat.getTrans() - _eventPos) / 10.0;
            offset = offset * _scale;
            osg::Matrix m;

            osg::Matrix r;
            r.makeRotate(_eventRot);
            osg::Vec3 pointInit = osg::Vec3(0,1,0);
            pointInit = pointInit * r;
            pointInit.z() = 0.0;

            r.makeRotate(mat.getRotate());
            osg::Vec3 pointFinal = osg::Vec3(0,1,0);
            pointFinal = pointFinal * r;
            pointFinal.z() = 0.0;

            osg::Matrix turn;
            if(pointInit.length2() > 0 && pointFinal.length2() > 0)
            {
                pointInit.normalize();
                pointFinal.normalize();
                float dot = pointInit * pointFinal;
                float angle = acos(dot) / 15.0;
                if(dot > 1.0 || dot < -1.0)
                {
                    angle = 0.0;
                }
                else if((pointInit ^ pointFinal).z() < 0)
                {
                    angle = -angle;
                }
                turn.makeRotate(-angle,osg::Vec3(0,0,1));
            }

            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();

            osg::Vec3 origin = mat.getTrans();
            m.makeTranslate(offset + origin);
            m = objmat * osg::Matrix::translate(-origin) * turn * m;
            SceneManager::instance()->setObjectMatrix(m);
            break;
        }
        case FLY:
        {
            osg::Matrix rotOffset = osg::Matrix::rotate(_eventRot.inverse())
                    * osg::Matrix::rotate(mat.getRotate());
            osg::Quat rot = rotOffset.getRotate();
            rot = rot.inverse();
            //rot.w() = 1.0;//(rot.w() + 99.0) / 100.0;
            double angle;
            osg::Vec3 vec;
            rot.getRotate(angle,vec);
            rot.makeRotate(angle / 20.0,vec);
            rotOffset.makeRotate(rot);
            osg::Vec3 posOffset = (mat.getTrans() - _eventPos) / 20.0;
            posOffset = posOffset * _scale;
            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();
            objmat = objmat * osg::Matrix::translate(-_eventPos) * rotOffset
                    * osg::Matrix::translate(_eventPos - posOffset);
            SceneManager::instance()->setObjectMatrix(objmat);
            break;
        }
        case MOVE_WORLD:
        {
            osg::Matrix objmat = _startXForm
                    * osg::Matrix::translate(-_eventPos)
                    * osg::Matrix::rotate(_eventRot.inverse()) * mat;
            SceneManager::instance()->setObjectMatrix(objmat);
            break;
        }
        case SCALE:
        {
            osg::Vec3 pos = mat.getTrans();
            float xdiff = pos.x() - _eventPos.x();
            xdiff = xdiff / 300.0;
            float newScale;
            osg::Vec3 objectPos =
                    (_eventPos * osg::Matrix::inverse(_startXForm))
                            / _startScale;
            //std::cerr << "Pos x: " << objectPos.x() << " y: " << objectPos.y() << " z: " << objectPos.z() << std::endl;
            if(xdiff >= 0)
            {
                newScale = _startScale * (1.0 + xdiff);
            }
            else
            {
                newScale = _startScale / (1.0 + fabs(xdiff));
            }
            SceneManager::instance()->setObjectScale(newScale);
            //std::cerr << "StartScale: " << _startScale << " newScale: " << newScale << std::endl;
            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();
            osg::Vec3 offset = -((objectPos * newScale * objmat)
                    - (objectPos * _startScale * objmat));
            osg::Matrix m;
            m.makeTranslate(offset);
            m = _startXForm * m;
            SceneManager::instance()->setObjectMatrix(m);
            break;
        }
        case NONE:
        default:
            break;
    }
}

void Navigation::processMouseNav(NavMode nm, MouseInteractionEvent * mie)
{
    float localX, localY;
    if(mie)
    {
        localX = mie->getX();
        localY = mie->getY();
    }
    else
    {
        localX = InteractionManager::instance()->getMouseX();
        localY = InteractionManager::instance()->getMouseY();
    }

    switch(nm)
    {
        case FLY:
        case WALK:
        case DRIVE:
        {
            float xOffset = localX - _eventX;
            float yOffset = localY - _eventY;

            osg::Matrix m;
            m.makeRotate(xOffset / 7000.0,osg::Vec3(0,0,1));

            osg::Matrix m2;
            m2.makeTranslate(osg::Vec3(0,(-yOffset / 5.0) * _scale,0));

            osg::Vec3 viewerPos =
                    TrackingManager::instance()->getHeadMat().getTrans();

            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();
            objmat = objmat * osg::Matrix::translate(-viewerPos) * m * m2
                    * osg::Matrix::translate(viewerPos);
            SceneManager::instance()->setObjectMatrix(objmat);
            break;
        }
        case MOVE_WORLD:
        {
            //TODO use screen transform in trackball calc
            ScreenInfo * si;
            if(mie)
            {
                si = ScreenConfig::instance()->getMasterScreenInfo(
                        mie->getMasterScreenNum());
            }
            else
            {
                si = ScreenConfig::instance()->getMasterScreenInfo(
                        CVRViewer::instance()->getActiveMasterScreen());
            }

            if(!si)
            {
                break;
            }

            float vwidth = std::min(si->myChannel->width,si->myChannel->height);

            float widthOffset = (si->myChannel->width - vwidth) / 2.0;
            float heightOffset = (si->myChannel->height - vwidth) / 2.0;

            float x, y, z;

            float currentX = _eventX - widthOffset;
            float currentY = _eventY - heightOffset;

            if(currentX < 0)
            {
                currentX = 0;
            }
            else if(currentX > vwidth)
            {
                currentX = vwidth;
            }

            if(currentY < 0)
            {
                currentY = 0;
            }
            else if(currentY > vwidth)
            {
                currentY = vwidth;
            }

            x = (currentX) / (vwidth / 2.0);
            z = (currentY) / (vwidth / 2.0);

            x = x - 1.0;
            z = z - 1.0;

            y = 1.0 - x * x - z * z;
            y = y > 0 ? -sqrt(y) : 0;

            osg::Vec3 originalPos = osg::Vec3(x,y,z);
            originalPos.normalize();

            currentX = localX - widthOffset;
            currentY = localY - heightOffset;

            if(currentX < 0)
            {
                currentX = 0;
            }
            else if(currentX > vwidth)
            {
                currentX = vwidth;
            }

            if(currentY < 0)
            {
                currentY = 0;
            }
            else if(currentY > vwidth)
            {
                currentY = vwidth;
            }

            x = (currentX) / (vwidth / 2.0);
            z = (currentY) / (vwidth / 2.0);
            x = x - 1.0;
            z = z - 1.0;

            y = 1.0 - x * x - z * z;
            y = y > 0 ? -sqrt(y) : 0;

            osg::Vec3 currentPos = osg::Vec3(x,y,z);
            currentPos.normalize();

            osg::Vec3 screenCenter = si->xyz;

            if(si->myChannel->stereoMode == "HMD")
            {
                screenCenter = screenCenter
                        * TrackingManager::instance()->getHeadMat(
                                si->myChannel->head);
            }

            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();
            objmat = (objmat * osg::Matrix::translate(-screenCenter)
                    * osg::Matrix::rotate(originalPos,currentPos)
                    * osg::Matrix::translate(screenCenter));
            SceneManager::instance()->setObjectMatrix(objmat);
            _eventX = (int)localX;
            _eventY = (int)localY;
            break;
        }
        case SCALE:
        {
            ScreenInfo * si;
            if(mie)
            {
                si = ScreenConfig::instance()->getMasterScreenInfo(
                        mie->getMasterScreenNum());
            }
            else
            {
                si = ScreenConfig::instance()->getMasterScreenInfo(
                        CVRViewer::instance()->getActiveMasterScreen());
            }

            if(!si)
            {
                break;
            }

            osg::Vec3 screenCenter = si->xyz;

            if(si->myChannel->stereoMode == "HMD")
            {
                screenCenter = screenCenter
                        * TrackingManager::instance()->getHeadMat(
                                si->myChannel->head);
            }

            osg::Vec3 pos = screenCenter;
            float xdiff = _eventY - localY;
            xdiff = -xdiff / 150.0;
            float newScale;
            osg::Vec3 objectPos = (pos * osg::Matrix::inverse(_startXForm))
                    / _startScale;
            //std::cerr << "Pos x: " << objectPos.x() << " y: " << objectPos.y() << " z: " << objectPos.z() << std::endl;
            if(xdiff >= 0)
            {
                newScale = _startScale * (1.0 + xdiff);
            }
            else
            {
                newScale = _startScale / (1.0 + fabs(xdiff));
            }
            SceneManager::instance()->setObjectScale(newScale);
            //std::cerr << "StartScale: " << _startScale << " newScale: " << newScale << std::endl;
            osg::Matrix objmat =
                    SceneManager::instance()->getObjectTransform()->getMatrix();
            osg::Vec3 offset = -((objectPos * newScale * objmat)
                    - (objectPos * _startScale * objmat));
            osg::Matrix m;
            m.makeTranslate(offset);
            m = _startXForm * m;
            SceneManager::instance()->setObjectMatrix(m);
            break;
        }
        case NONE:
        default:
            break;
    }
}
