#include <cvrMenu/BoardMenu.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuCollection.h>
#include <cvrUtil/Intersection.h>
#include <cvrUtil/OsgMath.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrInput/TrackingManager.h>

#include <string>
#include <iostream>

#include <osg/Geode>
#include <osgText/Text>
#include <osg/Image>
#include <osgDB/ReadFile>
#include <osg/LineWidth>

using namespace cvr;

BoardMenu::BoardMenu()
{
    _myMenu = NULL;

    _border = 10.0;

    _menuRoot = new osg::MatrixTransform();

    _activeHand = -1;

    std::string s;

    _distance = ConfigManager::getFloat("distance",
            "MenuSystem.BoardMenu.Position",2000.0);

    s = ConfigManager::getEntry("value","MenuSystem.BoardMenu.Trigger",
            "DOUBLECLICK");

    if(s == "DOUBLECLICK")
    {
        _trigger = DOUBLECLICK;
    }
    else if(s == "UPCLICK")
    {
        _trigger = UPCLICK;
    }
    else
    {
        std::cerr << "Unknown menu trigger " << s << ", using DOUBLECLICK"
                << std::endl;
        _trigger = DOUBLECLICK;
    }

    _primaryButton = ConfigManager::getInt("select",
            "MenuSystem.BoardMenu.Buttons",0);

    _secondaryButton = ConfigManager::getInt("open",
            "MenuSystem.BoardMenu.Buttons",1);

    _scale = ConfigManager::getFloat("MenuSystem.BoardMenu.Scale",1.0);

    _menuScale = new osg::MatrixTransform();
    osg::Matrix scale;
    scale.makeScale(osg::Vec3(_scale,1.0,_scale));
    _menuScale->setMatrix(scale);

    _menuRoot->addChild(_menuScale);

    osg::StateSet * stateset = _menuRoot->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    _menuActive = false;
    _activeItem = NULL;

    _clickActive = false;

    // TODO: read values from config file
    BoardMenuGeometry::_textColor = osg::Vec4(1.0,1.0,1.0,1.0);
    BoardMenuGeometry::_textColorSelected = osg::Vec4(0.0,1.0,0.0,1.0);
    BoardMenuGeometry::_backgroundColor = osg::Vec4(0.0,0.0,0.0,1.0);
    BoardMenuGeometry::_border = 10.0;
    BoardMenuGeometry::_iconHeight = 30.0;
    //BoardMenuGeometry::_textSize = 65.0;

    std::string fontfile;

    _iconDir = CalVR::instance()->getResourceDir();
    BoardMenuGeometry::_iconDir = _iconDir;
    fontfile = _iconDir;

    fontfile = fontfile + "/resources/ArenaCondensed.ttf";

    osgText::Font * font = osgText::readFontFile(fontfile);
    if(font)
    {
        BoardMenuGeometry::_font = font;
    }
    else
    {
        std::cerr << "Warning: font file: " << fontfile << " not found."
                << std::endl;
    }
    BoardMenuGeometry::calibrateTextSize(65.0);
}

BoardMenu::~BoardMenu()
{
    if(_menuActive)
    {
        SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
    }

    for(std::map<MenuItem *,BoardMenuGeometry *>::iterator it =
            _geometryMap.begin(); it != _geometryMap.end(); it++)
    {
        delete it->second;
    }
    _geometryMap.clear();

    for(std::map<SubMenu*,std::pair<BoardMenuGeometry*,BoardMenuGeometry*> >::iterator it =
            _menuGeometryMap.begin(); it != _menuGeometryMap.end(); it++)
    {
        delete it->second.first;
        delete it->second.second;
    }
    _menuGeometryMap.clear();
}

void BoardMenu::setMenu(SubMenu * menu)
{
    _myMenu = menu;
    _openMenus.push(menu);
    updateMenus();

    _menuScale->addChild(_menuMap[_myMenu]);
}

void BoardMenu::updateStart()
{
    updateMenus();
    _foundItem = false;
}

void BoardMenu::updateEnd()
{
    if(_menuActive && !_clickActive)
    {
        if(!_foundItem && _activeItem)
        {
            _activeHand = -1;
            selectItem(NULL);
        }
        else if(_activeItem)
        {
            osg::Vec3 pStart(0,0,0);
            osg::Vec3 pEnd(0,10000,0);
            pStart = pStart
                    * TrackingManager::instance()->getHandMat(_activeHand);
            pEnd = pEnd * TrackingManager::instance()->getHandMat(_activeHand);

            _activeItem->update(pStart,pEnd);
        }
    }
}

bool BoardMenu::processEvent(InteractionEvent * event)
{
    if(!_myMenu)
    {
        return false;
    }
	TrackedButtonInteractionEvent * tie = event->asTrackedButtonEvent();
	if (tie) {

		if (!_menuActive)
		{
			if (_trigger == DOUBLECLICK)
			{
				if (event->getInteraction() == BUTTON_DOUBLE_CLICK)
				{
					if (tie->getButton() == _secondaryButton)
					{
						SceneManager::instance()->getMenuRoot()->addChild(
							_menuRoot);

						osg::Vec3 menuPoint = osg::Vec3(0, _distance, 0);
						menuPoint = menuPoint * tie->getTransform();

						if (event->asMouseEvent())
						{
							osg::Vec3 menuOffset = osg::Vec3(
								_widthMap[_myMenu] / 2.0, 0, 0);
							osg::Matrix m;
							m.makeTranslate(menuPoint);
							_menuRoot->setMatrix(m);
						}
						else if (event->asPointerEvent())
						{
							//TODO add rotation
							SceneManager::instance()->getPointOnTiledWall(
								tie->getTransform(), menuPoint);
							osg::Vec3 menuOffset = osg::Vec3(
								_widthMap[_myMenu] / 2.0, 0, 0);
							osg::Matrix m;
							m.makeTranslate(menuPoint);
							_menuRoot->setMatrix(m);
						}
						else
						{
							osg::Vec3 viewerPoint =
								TrackingManager::instance()->getHeadMat(0).getTrans();

							osg::Vec3 viewerDir = viewerPoint - menuPoint;
							viewerDir.z() = 0.0;

							osg::Matrix menuRot;
							menuRot.makeRotate(osg::Vec3(0, -1, 0), viewerDir);

							osg::Vec3 menuOffset = osg::Vec3(
								_widthMap[_myMenu] * _scale / 2.0, 0, 0);
							_menuRoot->setMatrix(
								osg::Matrix::translate(-menuOffset) * menuRot
								* osg::Matrix::translate(menuPoint));
						}

						_menuActive = true;
						SceneManager::instance()->closeOpenObjectMenu();
						return true;
					}
				}
			}
			else if (_trigger == UPCLICK)
			{
				return false;
			}
		}
		else
		{
			if (_clickActive)
			{
				if (tie->getHand() == _activeHand)
				{
					if (tie->getButton() == _primaryButton)
					{
						if (tie->getInteraction() == BUTTON_DRAG
							|| tie->getInteraction() == BUTTON_UP)
						{
							BoardMenuSubMenuGeometry * smg =
								dynamic_cast<BoardMenuSubMenuGeometry *>(_activeItem);
							if (smg && smg->isMenuHead())
							{
								updateMovement(tie);
							}
						}

						_activeItem->processEvent(event);
						if (tie->getInteraction() == BUTTON_UP)
						{
							_clickActive = false;
						}
					}
					return true;
				}
				return false;
			}
			else if (tie->getHand() == _activeHand)
			{
				if (tie->getButton() == _primaryButton
					&& (tie->getInteraction() == BUTTON_DOWN
						|| tie->getInteraction() == BUTTON_DOUBLE_CLICK))
				{
					// do click
					if (_activeItem)
					{
						BoardMenuSubMenuGeometry * smg =
							dynamic_cast<BoardMenuSubMenuGeometry *>(_activeItem);
						if (smg && smg->isMenuHead())
						{
							osg::Vec3 ray;
							ray = _currentPoint[tie->getHand()]
								- tie->getTransform().getTrans();

							if (!tie->asPointerEvent())
							{
								_moveDistance = ray.length();
							}
							else
							{
								_moveDistance = ray.y();
							}
							_menuPoint = _currentPoint[tie->getHand()]
								* osg::Matrix::inverse(_menuRoot->getMatrix());
							updateMovement(tie);
						}
						if (smg && !smg->isMenuHead())
						{
							if (smg->isMenuOpen())
							{
								closeMenu((SubMenu*)smg->getMenuItem());
							}
							else
							{
								openMenu(smg);
							}
						}
						_clickActive = true;
						_activeItem->processEvent(event);
						return true;
					}
					return false;
				}
				else {
					if (_activeItem)
					{
						_activeItem->processEvent(event);
					}
				}
			}

			if (tie->getButton() == _secondaryButton
				&& tie->getInteraction() == BUTTON_DOWN)
			{
				/*if(_activeItem)
					{
					selectItem(NULL);
					}
					SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
					_menuActive = false;*/
				close();
				return true;
			}
		}
	}
	ValuatorInteractionEvent * val = event->asValuatorEvent();
	if(val){
		if (_menuActive)
		{
			if (_activeItem)
			{
				_activeItem->processEvent(event);
			}
		}
	}

    return false;
}

void BoardMenu::itemDelete(MenuItem * item)
{
    if(!_myMenu)
    {
        return;
    }

    std::vector<MenuCollection*> searchList;
    searchList.push_back(_myMenu);
    std::vector<std::pair<MenuCollection*,MenuItem*> > removeList;

    while(searchList.size())
    {
        for(std::vector<MenuItem*>::iterator it =
                searchList[0]->getChildren().begin();
                it != searchList[0]->getChildren().end(); it++)
        {
            if((*it)->isCollection())
            {
                if((*it) == item)
                {
                    removeList.push_back(
                            std::pair<MenuCollection*,MenuItem*>(searchList[0],(*it)));
                    continue;
                }
                else
                {
                    MenuCollection * sm = dynamic_cast<MenuCollection*>(*it);
                    searchList.push_back(sm);
                }
            }
            else if((*it) == item)
            {
                removeList.push_back(
                        std::pair<MenuCollection*,MenuItem*>(searchList[0],(*it)));
                continue;
            }
        }
        searchList.erase(searchList.begin());
    }

    for(int i = 0; i < removeList.size(); i++)
    {
        removeList[i].first->removeItem(removeList[i].second);
    }

    updateMenus();

    bool removedItem;
    do
    {
        removedItem = false;
        for(std::map<osg::Geode *,BoardMenuGeometry*>::iterator it =
                _intersectMap.begin(); it != _intersectMap.end(); it++)
        {
            if(it->second->getMenuItem() == item)
            {
                _intersectMap.erase(it);
                removedItem = true;
            }
        }
    }
    while(removedItem);

    if(_activeItem && item == _activeItem->getMenuItem())
    {
        _clickActive = false;
        _activeItem = NULL;
    }

    if(item->isSubMenu())
    {
        SubMenu * sm = (SubMenu*)item;
        if(_widthMap.find(sm) != _widthMap.end())
        {
            _widthMap.erase(sm);
        }
        if(_menuMap.find(sm) != _menuMap.end())
        {
            if(_menuMap[sm]->getNumParents())
            {
                closeMenu(sm);
            }
            _menuMap.erase(sm);
        }
        if(_menuGeometryMap.find(sm) != _menuGeometryMap.end())
        {
            delete _menuGeometryMap[sm].first;
            delete _menuGeometryMap[sm].second;
            _menuGeometryMap.erase(sm);
        }
    }
    else
    {
        if(_geometryMap.find(item) != _geometryMap.end())
        {
            delete _geometryMap[item];
            _geometryMap.erase(item);
        }
    }

    if(item == _myMenu)
    {
        _myMenu = NULL;
    }
}

void BoardMenu::clear()
{
    close();
    _myMenu = NULL;
    _menuScale->removeChildren(0,_menuScale->getNumChildren());
    _menuActive = false;
    _activeItem = NULL;
    _clickActive = false;
    _widthMap.clear();
    _menuMap.clear();
    _intersectMap.clear();

    for(std::map<MenuItem *,BoardMenuGeometry *>::iterator it =
            _geometryMap.begin(); it != _geometryMap.end(); it++)
    {
        delete it->second;
    }
    _geometryMap.clear();

    for(std::map<SubMenu*,std::pair<BoardMenuGeometry*,BoardMenuGeometry*> >::iterator it =
            _menuGeometryMap.begin(); it != _menuGeometryMap.end(); it++)
    {
        delete it->second.first;
        delete it->second.second;
    }
    _menuGeometryMap.clear();

    while(_openMenus.size() > 0)
    {
        _openMenus.pop();
    }
}

void BoardMenu::close()
{
    if(_menuActive)
    {
        _clickActive = false;
        selectItem(NULL);
        SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
        _menuActive = false;
        _activeHand = -1;
    }
}

void BoardMenu::setScale(float scale)
{
    _scale = scale;
    osg::Matrix m;
    m.makeScale(osg::Vec3(_scale,1.0,_scale));
    _menuScale->setMatrix(m);
}

float BoardMenu::getScale()
{
    return _scale;
}

BoardMenuGeometry * BoardMenu::getItemGeometry(MenuItem * item)
{
    if(item->isSubMenu())
    {
	if(_menuGeometryMap.find((SubMenu*)item) != _menuGeometryMap.end())
	{
	    return _menuGeometryMap[(SubMenu*)item].first;
	}
    }
    else if(_geometryMap.find(item) != _geometryMap.end())
    {
	return _geometryMap[item];
    }

    return NULL;
}

void BoardMenu::updateMenus()
{
    if(!_myMenu)
    {
        return;
    }

    std::vector<SubMenu*> searchList;
    std::vector<SubMenu*> foundList;

    searchList.push_back(_myMenu);
    foundList.push_back(_myMenu);

    // search through menu tree, make list of all submenus, dirty all submenus with a dirty child
    while(searchList.size())
    {
        for(std::vector<MenuItem*>::iterator it =
                searchList[0]->getChildren().begin();
                it != searchList[0]->getChildren().end(); it++)
        {
            if((*it)->isSubMenu())
            {
                SubMenu * sm = dynamic_cast<SubMenu*>(*it);
                searchList.push_back(sm);
                foundList.push_back(sm);
            }
            else if((*it)->isDirty())
            {
                // if an item is dirty, dirty its submenu for recalc
                searchList[0]->setDirty(true);
            }
        }
        searchList.erase(searchList.begin());
    }

    for(int i = 0; i < foundList.size(); i++)
    {
        if(!foundList[i]->isDirty())
        {
            continue;
        }

        std::vector<BoardMenuGeometry *> geoList;
        if(_menuMap.find(foundList[i]) == _menuMap.end())
        {
            _menuMap[foundList[i]] = new osg::MatrixTransform();
        }

        _menuMap[foundList[i]]->removeChildren(0,
                _menuMap[foundList[i]]->getNumChildren());

        if(_menuGeometryMap.find(foundList[i]) == _menuGeometryMap.end())
        {
            _menuGeometryMap[foundList[i]] = std::pair<BoardMenuGeometry*,
                    BoardMenuGeometry*>(createGeometry(foundList[i],this,true),
                    createGeometry(foundList[i],this));
        }

        if(_menuGeometryMap[foundList[i]].first)
        {
            geoList.push_back(_menuGeometryMap[foundList[i]].first);
        }

        for(std::vector<MenuItem*>::iterator it =
                foundList[i]->getChildren().begin();
                it != foundList[i]->getChildren().end(); it++)
        {
	    std::list<MenuItem*> itemList;
	    itemList.push_back((*it));
	    if(!(*it)->isSubMenu() && (*it)->isCollection())
	    {
		std::list<MenuCollection*> collectionList;
		collectionList.push_back((MenuCollection*)(*it));
		while(collectionList.size())
		{
		    for(int i = 0; i < collectionList.front()->getNumChildren(); ++i)
		    {
			MenuItem * mitem = collectionList.front()->getChild(i);
			itemList.push_front(mitem);
			if(mitem->isCollection() && !mitem->isSubMenu())
			{
			    collectionList.push_back((MenuCollection*)mitem);
			}
		    }
		    collectionList.pop_front();
		}
	    }

	    BoardMenuGeometry * mg;

	    for(std::list<MenuItem*>::iterator itemIt = itemList.begin(); itemIt != itemList.end(); ++itemIt)
	    {
		if((*itemIt)->isSubMenu())
		{
		    if(_menuGeometryMap.find((SubMenu*)(*itemIt))
			    == _menuGeometryMap.end())
		    {
			_menuGeometryMap[(SubMenu*)(*itemIt)] = std::pair<
			    BoardMenuGeometry*,BoardMenuGeometry*>(
				    createGeometry(*itemIt,this,true),createGeometry(*itemIt,this));
			_intersectMap[_menuGeometryMap[(SubMenu*)(*itemIt)].first->getIntersect()] = _menuGeometryMap[(SubMenu*)(*itemIt)].first;
			_intersectMap[_menuGeometryMap[(SubMenu*)(*itemIt)].second->getIntersect()] = _menuGeometryMap[(SubMenu*)(*itemIt)].second;
		    }
		    mg = _menuGeometryMap[(SubMenu*)(*itemIt)].second;
		}
		else if(_geometryMap.find(*itemIt) == _geometryMap.end())
		{
		    _geometryMap[*itemIt] = createGeometry(*itemIt,this);
		    mg = _geometryMap[*itemIt];
		    (*itemIt)->setDirty(false);
		    _intersectMap[mg->getIntersect()] = mg;
		}
		else
		{
		    mg = _geometryMap[*itemIt];
		}

		if(mg && !(*itemIt)->isSubMenu() && (*itemIt)->isDirty())
		{
		    mg->updateGeometry();
		    (*itemIt)->setDirty(false);
		}
	    }

	    if(mg)
            {
                geoList.push_back(mg);
            }
            else
            {
                //std::cerr << "Geometry is null." << std::endl;
            }
        }

        float width = 0;
        for(int j = 0; j < geoList.size(); j++)
        {
            if(geoList[j]->getWidth() > width)
            {
                width = geoList[j]->getWidth();
            }
        }
        _widthMap[foundList[i]] = (width + 2.0 * _border);

        // add invisible intersection test drawable
        for(int j = 0; j < geoList.size(); j++)
        {
            geoList[j]->resetIntersect(width);
        }

        // add line under menu title
        if(geoList.size() > 0)
        {
            BoardMenuSubMenuGeometry * smg =
                    dynamic_cast<BoardMenuSubMenuGeometry *>(geoList[0]);
            if(smg)
            {
                smg->resetMenuLine(width);
            }

        }

        float offset = _border;
        for(int j = 0; j < geoList.size(); j++)
        {
            osg::Matrix m;
            m.makeTranslate(osg::Vec3(_border,0,-offset));
            geoList[j]->getNode()->setMatrix(m);
            offset += geoList[j]->getHeight() + _border;
            _menuMap[foundList[i]]->addChild(geoList[j]->getNode());
            _intersectMap[geoList[j]->getIntersect()] = geoList[j];
        }

        // create menu board geometry
        osg::Geode * geode = new osg::Geode();
		
		
        geode->addDrawable(
                BoardMenuGeometry::makeQuad(width + 2.0 * _border,-offset,
                        BoardMenuGeometry::_backgroundColor));
        geode->addDrawable(
                BoardMenuGeometry::makeLine(osg::Vec3(0,-2,0),
                        osg::Vec3(width + 2.0 * _border,-2,0),
                        BoardMenuGeometry::_textColor));
        geode->addDrawable(
                BoardMenuGeometry::makeLine(osg::Vec3(0,-2,0),
                        osg::Vec3(0,-2,-offset),BoardMenuGeometry::_textColor));
        geode->addDrawable(
                BoardMenuGeometry::makeLine(osg::Vec3(0,-2,-offset),
                        osg::Vec3(width + 2.0 * _border,-2,-offset),
                        BoardMenuGeometry::_textColor));
        geode->addDrawable(
                BoardMenuGeometry::makeLine(
                        osg::Vec3(width + 2.0 * _border,-2,0),
                        osg::Vec3(width + 2.0 * _border,-2,-offset),
                        BoardMenuGeometry::_textColor));
						
						
        //scaleMT->addChild(geode);
        _menuMap[foundList[i]]->addChild(geode);

        osg::LineWidth* linewidth = new osg::LineWidth(2.0);
        osg::StateSet * stateset = geode->getOrCreateStateSet();
        stateset->setAttributeAndModes(linewidth,osg::StateAttribute::ON);

        //_menuMap[foundList[i]]->addChild(scaleMT);
        foundList[i]->setDirty(false);
    }

    std::stack<SubMenu*> revMenuStack;
    while(_openMenus.size())
    {
        bool found = false;
        for(int i = 0; i < foundList.size(); i++)
        {
            if(foundList[i] == _openMenus.top())
            {
                revMenuStack.push(_openMenus.top());
                found = true;
            }
        }

        if(!found)
        {
            closeMenu(_openMenus.top());
            //std::cerr << "Removing open menu." << std::endl;
        }
        else
        {
            //std::cerr << "Not removing open menu." << std::endl;
        }

        _openMenus.pop();
    }

    float offset = 0;
    osg::Matrix m;
    m.makeTranslate(osg::Vec3(-offset,0,0));
    if(revMenuStack.size())
    {
        _menuMap[revMenuStack.top()]->setMatrix(m);
        _openMenus.push(revMenuStack.top());
        revMenuStack.pop();
    }

    while(revMenuStack.size())
    {
        offset += _widthMap[revMenuStack.top()];
        m.makeTranslate(osg::Vec3(-offset,0,0));
        _menuMap[revMenuStack.top()]->setMatrix(m);
        _openMenus.push(revMenuStack.top());
        revMenuStack.pop();
    }
}

bool BoardMenu::processIsect(IsectInfo & isect, int hand)
{
    if(!_menuActive)
    {
        return false;
    }
    else if(_clickActive)
    {
        _currentPoint[hand] = isect.point;
        return true;
    }

    if(_intersectMap.find(isect.geode) != _intersectMap.end())
    {
        TrackerBase::TrackerType ttype =
                TrackingManager::instance()->getHandTrackerType(hand);
        if(_activeHand >= 0 && _activeHand != hand)
        {
            if(ttype
                    >= TrackingManager::instance()->getHandTrackerType(
                            _activeHand))
            {
                _currentPoint[hand] = isect.point;
                return true;
            }
        }
        _activeHand = hand;
        selectItem(_intersectMap[isect.geode]);
        _foundItem = true;
        _currentPoint[hand] = isect.point;
        return true;
    }

    return false;
}

void BoardMenu::selectItem(BoardMenuGeometry * mg)
{
    if(mg != _activeItem)
    {
        if(_activeItem)
        {
            _activeItem->selectItem(false);
        }

        if(mg)
        {
            mg->selectItem(true);
        }

        _activeItem = mg;
    }
}

void BoardMenu::openMenu(BoardMenuSubMenuGeometry * smg)
{
    bool foundItem = false;
    while(_openMenus.size() > 0)
    {
        for(int i = 0; i < _openMenus.top()->getNumChildren(); i++)
        {
            if(_openMenus.top()->getChild(i) == smg->getMenuItem())
            {
                foundItem = true;
                break;
            }
        }
        if(foundItem)
        {
            break;
        }
        closeMenu(_openMenus.top());
        _openMenus.pop();
    }

    if(_openMenus.size() == 0)
    {
        std::cerr << "Could not find SubMenu item." << std::endl;
        return;
    }

    smg->openMenu(true);

    osg::Vec3 pos = _menuMap[_openMenus.top()]->getMatrix().getTrans();
    pos = pos + osg::Vec3(-_widthMap[(SubMenu*)smg->getMenuItem()],0,0);

    osg::Matrix m;
    m.makeTranslate(pos);

    _menuMap[(SubMenu*)smg->getMenuItem()]->setMatrix(m);
    _menuScale->addChild(_menuMap[(SubMenu*)smg->getMenuItem()]);

    _openMenus.push((SubMenu*)smg->getMenuItem());
}

void BoardMenu::closeMenu(SubMenu * menu)
{
    while(_openMenus.size() > 0)
    {
        BoardMenuSubMenuGeometry * smg = NULL;
        for(std::map<osg::Geode *,BoardMenuGeometry*>::iterator it =
                _intersectMap.begin(); it != _intersectMap.end(); it++)
        {
            if(it->second->getMenuItem() == _openMenus.top())
            {
                smg = (BoardMenuSubMenuGeometry*)(it->second);
                if(smg && !smg->isMenuHead())
                {
                    break;
                }
                else
                {
                    smg = NULL;
                }
            }
        }

        if(!smg)
        {
            return;
        }

        smg->openMenu(false);

        _menuScale->removeChild(_menuMap[_openMenus.top()]);

        if(_openMenus.top() == menu)
        {
            break;
        }
        else
        {
            _openMenus.pop();
        }
    }
}

void BoardMenu::updateMovement(TrackedButtonInteractionEvent * tie)
{
    if(!tie->asPointerEvent())
    {
        osg::Vec3 menuPoint = osg::Vec3(0,_moveDistance,0);
        //std::cerr << "move dist: " << _moveDistance << std::endl;
        menuPoint = menuPoint * tie->getTransform();

        //TODO: add hand/head mapping
        osg::Vec3 viewerPoint =
                TrackingManager::instance()->getHeadMat(0).getTrans();

        osg::Vec3 viewerDir = viewerPoint - menuPoint;
        viewerDir.z() = 0.0;

        osg::Matrix menuRot;
        menuRot.makeRotate(osg::Vec3(0,-1,0),viewerDir);

        _menuRoot->setMatrix(
                osg::Matrix::translate(-_menuPoint) * menuRot
                        * osg::Matrix::translate(menuPoint));
    }
    else
    {
        osg::Vec3 point1, point2(0,1000,0), planePoint, planeNormal(0,-1,0),
                intersect;
        float w;
        point1 = point1 * tie->getTransform();
        point2 = point2 * tie->getTransform();
        planePoint = osg::Vec3(0,
                _moveDistance + tie->getTransform().getTrans().y(),0);

        if(linePlaneIntersectionRef(point1,point2,planePoint,planeNormal,
                intersect,w))
        {
            _menuRoot->setMatrix(
                    osg::Matrix::translate(intersect - _menuPoint));
        }
    }
}
