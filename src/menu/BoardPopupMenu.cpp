#include <menu/BoardPopupMenu.h>

#include <kernel/SceneManager.h>
#include <input/TrackingManager.h>

#include <iostream>

using namespace cvr;

BoardPopupMenu::BoardPopupMenu() : BoardMenu()
{
}

BoardPopupMenu::~BoardPopupMenu()
{
}

void BoardPopupMenu::setMenu(SubMenu * menu)
{
    BoardMenu::setMenu(menu);
}

void BoardPopupMenu::updateStart()
{
    BoardMenu::updateStart();
}

bool BoardPopupMenu::processIsect(IsectInfo & isect, int hand)
{
    if(BoardMenu::processIsect(isect,hand))
    {
	_currentPoint = isect.point;
	return true;
    }
    return false;
}

void BoardPopupMenu::updateEnd()
{
    BoardMenu::updateEnd();
}

bool BoardPopupMenu::processEvent(InteractionEvent * event)
{
    if(!_menuActive || !_myMenu)
    {
	return false;
    }
    else
    {
	/*if(event->type == MOUSE_BUTTON_DOWN)
	{
	    std::cerr << "Mouse down." << std::endl;
	    MouseInteractionEvent * mie = (MouseInteractionEvent*)event;
	    std::cerr << "Button: " << mie->button << std::endl;

	}
	if(event->type == MOUSE_DOUBLE_CLICK)
	{
	    std::cerr << "Mouse double click." << std::endl;
	    MouseInteractionEvent * mie = (MouseInteractionEvent*)event;
	    std::cerr << "Button: " << mie->button << std::endl;
	}*/

        if(_clickActive)
        {
            if(_activeInteractor == HAND)
            {
                if(event->type == BUTTON_DRAG || event->type == BUTTON_UP)
		{
		    TrackingInteractionEvent * tie = (TrackingInteractionEvent *)event;
		    if(tie->hand == _primaryHand && tie->button == _primaryButton)
		    {
			BoardMenuSubMenuGeometry * smg = dynamic_cast<BoardMenuSubMenuGeometry *>(_activeItem);
			if(smg && smg->isMenuHead())
			{
			    updateMovement(event);
			}

			_activeItem->processEvent(event);
			if(event->type == BUTTON_UP)
			{
			    _clickActive = false;
			}
			return true;
		    }
		}
            }
            else if(_activeInteractor == MOUSE)
            {
                if(event->type == MOUSE_BUTTON_UP || event->type == MOUSE_DRAG)
                {
                    MouseInteractionEvent * mie = (MouseInteractionEvent*)event;
                    if(mie->button == _primaryMouseButton)
                    {
			BoardMenuSubMenuGeometry * smg = dynamic_cast<BoardMenuSubMenuGeometry *>(_activeItem);
			if(smg && smg->isMenuHead())
			{
			    updateMovement(event);
			}

                        _activeItem->processEvent(event);
                        if(event->type == MOUSE_BUTTON_UP)
                        {
                            _clickActive = false;
                        }
                        return true;
                    }
                }
            }

            return false;
        }

        if(((event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK)
                && ((TrackingInteractionEvent*)event)->hand == _primaryHand
                && ((TrackingInteractionEvent*)event)->button == _primaryButton)
                || ((event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK)
                        && ((MouseInteractionEvent*)event)->button
                                == _primaryMouseButton))
        {
            if(((event->type == BUTTON_DOWN || event->type == BUTTON_DOUBLE_CLICK) && _activeInteractor != HAND)
                    || ((event->type == MOUSE_BUTTON_DOWN || event->type == MOUSE_DOUBLE_CLICK) && _activeInteractor
                            != MOUSE))
            {
                return false;
            }

            // do click
            if(_activeItem)
            {
		BoardMenuSubMenuGeometry * smg = dynamic_cast<BoardMenuSubMenuGeometry *>(_activeItem);
		if(smg && smg->isMenuHead())
		{
		    osg::Vec3 ray;
		    if(_activeInteractor == MOUSE)
		    {
			ray = _currentPoint - InteractionManager::instance()->getMouseMat().getTrans();
		    }
		    else
		    {
			ray = _currentPoint - TrackingManager::instance()->getHandMat(_primaryHand).getTrans();
		    }
		    _moveDistance = ray.length();
		    _menuPoint = _currentPoint * osg::Matrix::inverse(_menuRoot->getMatrix());
		    updateMovement(event);
		}
		else if(smg && !smg->isMenuHead())
		{
		    if(smg->isMenuOpen())
		    {
			closeMenu((SubMenu*)smg->getMenuItem());
		    }
		    else
		    {
			openMenu(smg);
		    }
		}
                _activeItem->processEvent(event);
                _clickActive = true;
                return true;
            }

            return false;
        }
    }
    return false;
}

void BoardPopupMenu::itemDelete(MenuItem * item)
{
    BoardMenu::itemDelete(item);
}

void BoardPopupMenu::clear()
{
    BoardMenu::clear();
}

void BoardPopupMenu::close()
{
    setVisible(false);
}

void BoardPopupMenu::setScale(float scale)
{
    BoardMenu::setScale(scale);
}

float BoardPopupMenu::getScale()
{
    return BoardMenu::getScale();
}

void BoardPopupMenu::setPosition(osg::Vec3 pos)
{
    osg::Matrix m = _menuRoot->getMatrix();
    m.setTrans(pos);
    _menuRoot->setMatrix(m);
}

osg::Vec3 BoardPopupMenu::getPosition()
{
    return _menuRoot->getMatrix().getTrans();
}

void BoardPopupMenu::setRotation(osg::Quat rot)
{
    osg::Matrix m = _menuRoot->getMatrix();
    m.setRotate(rot);
    _menuRoot->setMatrix(m);
}

osg::Quat BoardPopupMenu::getRotation()
{
    return _menuRoot->getMatrix().getRotate();
}

void BoardPopupMenu::setTransform(osg::Matrix m)
{
    _menuRoot->setMatrix(m);
}

osg::Matrix BoardPopupMenu::getTransform()
{
    return _menuRoot->getMatrix();
}


void BoardPopupMenu::setVisible(bool v)
{
    if(v == _menuActive)
    {
	return;
    }

    if(v)
    {
	SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);
    }
    else
    {
	SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
    }

    _menuActive = v;
}

bool BoardPopupMenu::isVisible()
{
    return _menuActive;
}

void BoardPopupMenu::updateMovement(InteractionEvent * event)
{
    osg::Vec3 menuPoint = osg::Vec3(0, _moveDistance, 0);
    //std::cerr << "move dist: " << _moveDistance << std::endl;
    if(_activeInteractor == HAND)
    {
	TrackingInteractionEvent * tie =
	    (TrackingInteractionEvent*)event;

	osg::Matrix r, t;
	t.makeTranslate(tie->xyz[0], tie->xyz[1], tie->xyz[2]);
	r.makeRotate(osg::Quat(tie->rot[0], tie->rot[1],
		    tie->rot[2], tie->rot[3]));
	osg::Matrix handmat = r * t;

	menuPoint = menuPoint * handmat;
    }
    else if(_activeInteractor == MOUSE)
    {
	MouseInteractionEvent * mie = (MouseInteractionEvent *)event;
	menuPoint = menuPoint * mie->transform;
    }

    osg::Vec3
	viewerPoint =
	TrackingManager::instance()->getHeadMat(0).getTrans();

    osg::Vec3 viewerDir = viewerPoint - menuPoint;
    viewerDir.z() = 0.0;

    osg::Matrix menuRot;
    menuRot.makeRotate(osg::Vec3(0,-1,0),viewerDir);

    _menuRoot->setMatrix(osg::Matrix::translate(-_menuPoint) * menuRot * osg::Matrix::translate(menuPoint));
}
