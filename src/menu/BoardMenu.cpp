#include <menu/BoardMenu.h>
#include <menu/MenuButton.h>
#include <menu/MenuCheckbox.h>
#include <util/Intersection.h>
#include <config/ConfigManager.h>
#include <kernel/SceneManager.h>
#include <kernel/PluginHelper.h>
#include <input/TrackingManager.h>

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

    _boarder = 10.0;

    _menuRoot = new osg::MatrixTransform();

    _activeInteractor = NONE;

    std::string s;

    _distance
            = ConfigManager::getFloat("distance",
                                      "MenuSystem.BoardMenu.Position", 2000.0);

    s = ConfigManager::getEntry("value", "MenuSystem.BoardMenu.Trigger",
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
        std::cerr << "Unknown menu trigger " << s << std::endl;
        return;
    }

    _primaryHand = ConfigManager::getInt("primaryHand",
                                         "MenuSystem.BoardMenu.Button", 0);
    _primaryButton = ConfigManager::getInt("primaryButton",
                                           "MenuSystem.BoardMenu.Button", 0);

    _secondaryHand = ConfigManager::getInt("secondaryHand",
                                           "MenuSystem.BoardMenu.Button", 0);
    _secondaryButton = ConfigManager::getInt("secondaryButton",
                                             "MenuSystem.BoardMenu.Button", 1);

    _primaryMouseButton = ConfigManager::getInt("primaryMouseButton",
                                                "MenuSystem.BoardMenu.Button",
                                                0);
    _secondaryMouseButton
            = ConfigManager::getInt("secondaryMouseButton",
                                    "MenuSystem.BoardMenu.Button", 1);

    if(TrackingManager::instance()->getUsingMouseTracker())
    {
	_primaryHand = _secondaryHand = 0;
	_primaryButton = _primaryMouseButton;
	_secondaryButton = _secondaryMouseButton;
    }

    _scale = ConfigManager::getFloat("MenuSystem.BoardMenu.Scale", 1.0);

    osg::StateSet * stateset = _menuRoot->getOrCreateStateSet();
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    _menuActive = false;
    _activeItem = NULL;
    //_openingMenu = NULL;
    _openingElapse = 0.0;

    _openingThreshold = ConfigManager::getFloat("MenuSystem.BoardMenu.OpenThreshold", 0.5);

    _clickActive = false;

    // TODO: read values from config file
    BoardMenuGeometry::_textColor = osg::Vec4(1.0, 1.0, 1.0, 1.0);
    BoardMenuGeometry::_textColorSelected = osg::Vec4(0.0, 1.0, 0.0, 1.0);
    BoardMenuGeometry::_backgroundColor = osg::Vec4(0.0, 0.0, 0.0, 1.0);
    BoardMenuGeometry::_boarder = 10.0;
    BoardMenuGeometry::_iconHeight = 30.0;
    //BoardMenuGeometry::_textSize = 65.0;

    std::string fontfile;

    _iconDir = CalVR::instance()->getHomeDir();
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
	std::cerr << "Warning: font file: " << fontfile << " not found." << std::endl;
    }
    BoardMenuGeometry::calibrateTextSize(65.0);
}

BoardMenu::~BoardMenu()
{
}

void BoardMenu::setMenu(SubMenu * menu)
{
    _myMenu = menu;
    _openMenus.push(menu);
    updateMenus();

    _menuRoot->addChild(_menuMap[_myMenu]);
}

void BoardMenu::updateStart()
{
    updateMenus();
    _foundItem = false;
    /*if(_menuActive && !_clickActive)
    {
        checkIntersection();
    }*/
}

void BoardMenu::updateEnd()
{
    if(_menuActive && !_clickActive)
    {
        if(!_foundItem && _activeItem)
	{
	    _activeInteractor = NONE;
	    selectItem(NULL);
	}
    }
}

bool BoardMenu::processEvent(InteractionEvent * event)
{
    if(!_myMenu)
    {
	return false;
    }

    if(!_menuActive)
    {
        if(_trigger == DOUBLECLICK)
        {
            if(event->type == BUTTON_DOUBLE_CLICK)
            {
                TrackingInteractionEvent * tie =
                        (TrackingInteractionEvent*)event;
                if(tie->hand == _secondaryHand && tie->button
                        == _secondaryButton)
                {
                    //attach menu position correctly in space
                    SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);

                    osg::Vec3 menuPoint = osg::Vec3(0, _distance, 0);

                    osg::Matrix r, t;
                    t.makeTranslate(tie->xyz[0], tie->xyz[1], tie->xyz[2]);
                    r.makeRotate(osg::Quat(tie->rot[0], tie->rot[1],
                                           tie->rot[2], tie->rot[3]));
                    osg::Matrix handmat = r * t;

                    menuPoint = menuPoint * handmat;

                    osg::Vec3
                            viewerPoint =
                                    TrackingManager::instance()->getHeadMat(0).getTrans();

		    osg::Vec3 viewerDir = viewerPoint - menuPoint;
		    viewerDir.z() = 0.0;

		    osg::Matrix menuRot;
		    menuRot.makeRotate(osg::Vec3(0,-1,0),viewerDir);

                    osg::Vec3 menuOffset = osg::Vec3(_widthMap[_myMenu] / 2.0,
                                                     0, 0);
                    //osg::Matrix m;
                    //m.makeTranslate(menuPoint);
                    _menuRoot->setMatrix(osg::Matrix::translate(-menuOffset) * menuRot * osg::Matrix::translate(menuPoint));

                    _menuActive = true;
                    return true;
                }
            }
            else if(event->type == MOUSE_DOUBLE_CLICK)
            {
                MouseInteractionEvent * mie = (MouseInteractionEvent *)event;
                if(mie->button == _secondaryMouseButton)
                {
                    //attach menu position correctly in space
                    SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);

                    osg::Vec3 menuPoint = osg::Vec3(0, _distance, 0);
                    menuPoint = menuPoint * mie->transform;

                    osg::Vec3 menuOffset = osg::Vec3(_widthMap[_myMenu] / 2.0,
                                                     0, 0);
                    osg::Matrix m;
                    m.makeTranslate(menuPoint);
                    _menuRoot->setMatrix(m);

                    _menuActive = true;
                    return true;
                }
            }
        }
        else if(_trigger == UPCLICK)
        {
            return false;
        }
    }
    else
    {
        if(_clickActive)
        {
            if(_activeInteractor == HAND)
            {
                if(event->type == BUTTON_DRAG || event->type == BUTTON_UP)
                {
                    TrackingInteractionEvent * tie = (TrackingInteractionEvent *)event;
                    if(tie->hand == _primaryHand && tie->button == _primaryButton)
                    {
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
		BoardMenuSubMenuGeometry * smg =
		    dynamic_cast<BoardMenuSubMenuGeometry *> (_activeItem);
		if(smg && !smg->isMenuHead())
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
        else if((event->type == BUTTON_DOWN
                && ((TrackingInteractionEvent*)event)->hand == _secondaryHand
                && ((TrackingInteractionEvent*)event)->button
                        == _secondaryButton) || (event->type
                == MOUSE_BUTTON_DOWN && ((MouseInteractionEvent*)event)->button
                == _secondaryMouseButton))
        {
            if(_activeItem)
            {
                selectItem(NULL);
            }
            SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
            _menuActive = false;
            return true;
        }
    }
    return false;
}

void BoardMenu::itemDelete(MenuItem * item)
{
    //std::cerr << "Removing item" << std::endl;
    
    std::vector<SubMenu*> searchList;
    searchList.push_back(_myMenu);
    std::vector<std::pair<SubMenu*,MenuItem*> > removeList;

    while(searchList.size())
    {
        for(std::vector<MenuItem*>::iterator it =
                searchList[0]->getChildren().begin(); it
                != searchList[0]->getChildren().end(); it++)
        {
            if((*it)->isSubMenu())
            {
		if((*it) == item)
		{
		     //it = searchList[0]->getChildren().erase(it);
		     removeList.push_back(std::pair<SubMenu*,MenuItem*>(searchList[0],(*it)));
		     continue;
		}
		else
		{
		    SubMenu * sm = dynamic_cast<SubMenu*> (*it);
		    searchList.push_back(sm);
		}
            }
	    else if((*it) == item)
	    {
		//it = searchList[0]->getChildren().erase(it);
		removeList.push_back(std::pair<SubMenu*,MenuItem*>(searchList[0],(*it)));
		continue;
	    }
	    //it++;
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
	for(std::map<osg::Geode *,BoardMenuGeometry*>::iterator it = _intersectMap.begin(); it != _intersectMap.end(); it++)
	{
	    if(it->second->getMenuItem() == item)
	    {
		_intersectMap.erase(it);
		removedItem = true;
	    }
	}
    } while(removedItem);

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

    if(_activeItem && item == _activeItem->getMenuItem())
    {
	_clickActive = false;
	_activeItem = NULL;
    }

    if(item == _myMenu)
    {
	_myMenu = NULL;
    }

    /*if(!_myMenu)
    {
	return;
    }

    std::vector<SubMenu*> searchList;
    searchList.push_back(_myMenu);

    while(searchList.size())
    {
        for(std::vector<MenuItem*>::iterator it =
                searchList[0]->getChildren().begin(); it
                != searchList[0]->getChildren().end();)
        {
            if((*it)->isSubMenu())
            {
		if((*it) == item)
		{
		     it = searchList[0]->getChildren().erase(it);
		     continue;
		}
		else
		{
		    SubMenu * sm = dynamic_cast<SubMenu*> (*it);
		    searchList.push_back(sm);
		}
            }
	    else if((*it) == item)
	    {
		it = searchList[0]->getChildren().erase(it);
		continue;
	    }
	    it++;
        }
        searchList.erase(searchList.begin());
    }
    updateMenus();*/
}

void BoardMenu::clear()
{
    close();
    _myMenu = NULL;
    _menuRoot->removeChildren(0,_menuRoot->getNumChildren());
    _activeInteractor = NONE;
    _menuActive = false;
    _activeItem = NULL;
    _clickActive = false;
    _widthMap.clear();
    _menuMap.clear();
    _intersectMap.clear();
    
    for(std::map<MenuItem *, BoardMenuGeometry *>::iterator it = _geometryMap.begin(); it != _geometryMap.end(); it++)
    {
	delete it->second;
    }
    _geometryMap.clear();

    for(std::map<SubMenu*,std::pair<BoardMenuGeometry*,BoardMenuGeometry*> >::iterator it = _menuGeometryMap.begin(); it != _menuGeometryMap.end(); it++)
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
    }
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
                searchList[0]->getChildren().begin(); it
                != searchList[0]->getChildren().end(); it++)
        {
            if((*it)->isSubMenu())
            {
                SubMenu * sm = dynamic_cast<SubMenu*> (*it);
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

    //std::cerr << "Found " << foundList.size() << " SubMenus." << std::endl;

    for(int i = 0; i < foundList.size(); i++)
    {
        if(!foundList[i]->isDirty())
        {
            continue;
        }

        //std::cerr << "Menu Gen." << std::endl;

        std::vector<BoardMenuGeometry *> geoList;
        if(_menuMap.find(foundList[i]) == _menuMap.end())
        {
            _menuMap[foundList[i]] = new osg::MatrixTransform();
        }

        _menuMap[foundList[i]]->removeChildren(
                                               0,
                                               _menuMap[foundList[i]]->getNumChildren());

        if(_menuGeometryMap.find(foundList[i]) == _menuGeometryMap.end())
        {
            _menuGeometryMap[foundList[i]] = std::pair<BoardMenuGeometry*,
                    BoardMenuGeometry*>(createGeometry(foundList[i], true),
                                        createGeometry(foundList[i]));
        }

        if(_menuGeometryMap[foundList[i]].first)
        {
            geoList.push_back(_menuGeometryMap[foundList[i]].first);
        }

        for(std::vector<MenuItem*>::iterator it =
                foundList[i]->getChildren().begin(); it
                != foundList[i]->getChildren().end(); it++)
        {
            BoardMenuGeometry * mg;

            if((*it)->isSubMenu())
            {
                if(_menuGeometryMap.find((SubMenu*)(*it))
                        == _menuGeometryMap.end())
                {
                    _menuGeometryMap[(SubMenu*)(*it)]
                            = std::pair<BoardMenuGeometry*,BoardMenuGeometry*>(
                                                                               createGeometry(
                                                                                              *it,
                                                                                              true),
                                                                               createGeometry(
                                                                                              *it));
                }
                mg = _menuGeometryMap[(SubMenu*)(*it)].second;
            }
            else if(_geometryMap.find(*it) == _geometryMap.end())
            {
                _geometryMap[*it] = createGeometry(*it);
                mg = _geometryMap[*it];
                (*it)->setDirty(false);
            }
            else
            {
                mg = _geometryMap[*it];
            }

            if(mg)
            {
                geoList.push_back(mg);
            }
            else
            {
                //std::cerr << "Geometry is null." << std::endl;
            }

            if(mg && !(*it)->isSubMenu() && (*it)->isDirty())
            {
                mg->updateGeometry();
                (*it)->setDirty(false);
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
        _widthMap[foundList[i]] = (width + 2.0 * _boarder) * _scale;

        // add invisible intersection test drawable
        for(int j = 0; j < geoList.size(); j++)
        {
            geoList[j]->resetIntersect(width);
        }

        // add line under menu title
        if(geoList.size() > 0)
        {
            BoardMenuSubMenuGeometry * smg =
                    dynamic_cast<BoardMenuSubMenuGeometry *> (geoList[0]);
            if(smg)
            {
                smg->resetMenuLine(width);
            }

        }

        osg::MatrixTransform * scaleMT = new osg::MatrixTransform();

        osg::Matrix scale;
        scale.makeScale(osg::Vec3(_scale, 1.0, _scale));

        scaleMT->setMatrix(scale);

        float offset = _boarder;
        for(int j = 0; j < geoList.size(); j++)
        {
            osg::Matrix m;
            m.makeTranslate(osg::Vec3(_boarder, 0, -offset));
            geoList[j]->getNode()->setMatrix(m);
            offset += geoList[j]->getHeight() + _boarder;
            scaleMT->addChild(geoList[j]->getNode());
            _intersectMap[geoList[j]->getIntersect()] = geoList[j];
        }

        // create menu board geometry
        osg::Geode * geode = new osg::Geode();
        geode->addDrawable(
                           BoardMenuGeometry::makeQuad(
                                                       width + 2.0 * _boarder,
                                                       -offset,
                                                       BoardMenuGeometry::_backgroundColor));
        geode->addDrawable(
                           BoardMenuGeometry::makeLine(
                                                       osg::Vec3(0, -2, 0),
                                                       osg::Vec3(width + 2.0
                                                               * _boarder, -2,
                                                                 0),
                                                       BoardMenuGeometry::_textColor));
        geode->addDrawable(
                           BoardMenuGeometry::makeLine(
                                                       osg::Vec3(0, -2, 0),
                                                       osg::Vec3(0, -2, -offset),
                                                       BoardMenuGeometry::_textColor));
        geode->addDrawable(
                           BoardMenuGeometry::makeLine(
                                                       osg::Vec3(0, -2, -offset),
                                                       osg::Vec3(width + 2.0
                                                               * _boarder, -2,
                                                                 -offset),
                                                       BoardMenuGeometry::_textColor));
        geode->addDrawable(
                           BoardMenuGeometry::makeLine(
                                                       osg::Vec3(width + 2.0
                                                               * _boarder, -2,
                                                                 0),
                                                       osg::Vec3(width + 2.0
                                                               * _boarder, -2,
                                                                 -offset),
                                                       BoardMenuGeometry::_textColor));
        scaleMT->addChild(geode);

	osg::LineWidth* linewidth = new osg::LineWidth(2.0);
	osg::StateSet * stateset = geode->getOrCreateStateSet();
	stateset->setAttributeAndModes(linewidth,osg::StateAttribute::ON);

        _menuMap[foundList[i]]->addChild(scaleMT);
        foundList[i]->setDirty(false);
    }

    std::stack<SubMenu*> revMenuStack;
    //std::cerr << "OpenMenus: " << _openMenus.size() << std::endl;
    //std::cerr << "MenuNum: " << _menuRoot->getNumChildren() << std::endl;
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

/*BoardMenuGeometry * BoardMenu::createGeometry(MenuItem * item, bool head)
{
    switch(item->getType())
    {
        case BUTTON:
        {
            BoardMenuGeometry * mg = new BoardMenuButtonGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case CHECKBOX:
        {
            BoardMenuGeometry * mg = new BoardMenuCheckboxGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case SUBMENU:
        {
            BoardMenuGeometry * mg = new BoardMenuSubMenuGeometry(head);
            mg->createGeometry(item);

            return mg;
            break;
        }
        case RANGEVALUE:
        {
            BoardMenuGeometry * mg = new BoardMenuRangeValueGeometry();
            mg->createGeometry(item);

            return mg;
            break;
        }
        case OTHER:
        default:
            break;
    }

    return NULL;
}*/

bool BoardMenu::processIsect(IsectInfo & isect, bool mouse)
{
    if(!_menuActive)
    {
	return false;
    }
    else if(_clickActive)
    {
	return true;
    }
    
    if(_intersectMap.find(isect.geode) != _intersectMap.end())
    {
	selectItem(_intersectMap[isect.geode]);
	if(mouse)
	{
	    _activeInteractor = MOUSE;
	}
	else
	{
	    _activeInteractor = HAND;
	}
	_foundItem = true;
	return true;
    }

    return false;
}

void BoardMenu::checkIntersection()
{
    osg::Vec3 pointerStart =
            TrackingManager::instance()->getHandMat(_primaryHand).getTrans(),
            pointerEnd;
    pointerEnd.set(0.0f, 10000.0f, 0.0f);
    pointerEnd = pointerEnd
            * TrackingManager::instance()->getHandMat(_primaryHand);

    std::vector<IsectInfo> isecvec =
            getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                                  pointerStart, pointerEnd);

    BoardMenuGeometry * intersect = NULL;

    for(int i = 0; i < isecvec.size(); i++)
    {
        if(_intersectMap.find(isecvec[i].geode) != _intersectMap.end())
        {
            intersect = _intersectMap[isecvec[i].geode];
            break;
        }
    }

    if(intersect)
    {
        selectItem(intersect);
        _activeInteractor = HAND;
        return;
    }

    pointerStart = InteractionManager::instance()->getMouseMat().getTrans();
    pointerEnd.set(0.0f, 10000.0f, 0.0f);
    pointerEnd = pointerEnd * InteractionManager::instance()->getMouseMat();

    isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                                    pointerStart, pointerEnd);

    for(int i = 0; i < isecvec.size(); i++)
    {
        if(_intersectMap.find(isecvec[i].geode) != _intersectMap.end())
        {
            intersect = _intersectMap[isecvec[i].geode];
            break;
        }
    }

    if(intersect)
    {
        _activeInteractor = MOUSE;
    }
    else
    {
        _activeInteractor = NONE;
    }

    selectItem(intersect);
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
	    /*BoardMenuSubMenuGeometry * smg =
		dynamic_cast<BoardMenuSubMenuGeometry *> (mg);
	    if(smg && !smg->isMenuHead())
	    {
		//openMenu(smg);
		_openingElapse = 0.0;
		//_openingMenu = smg;
	    }*/
	}

	_activeItem = mg;
    }
    /*else if(_activeItem)
    {
	BoardMenuSubMenuGeometry * smg =
		dynamic_cast<BoardMenuSubMenuGeometry *> (mg);
	if(smg && !smg->isMenuHead())
	{
	    if(_openingElapse < _openingThreshold)
	    {
		_openingElapse += PluginHelper::getLastFrameDuration();
		if(_openingElapse >= _openingThreshold)
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
	    }
	}
    }*/
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
    pos = pos + osg::Vec3(-_widthMap[(SubMenu*)smg->getMenuItem()], 0, 0);

    osg::Matrix m;
    m.makeTranslate(pos);

    _menuMap[(SubMenu*)smg->getMenuItem()]->setMatrix(m);
    _menuRoot->addChild(_menuMap[(SubMenu*)smg->getMenuItem()]);

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

	_menuRoot->removeChild(_menuMap[_openMenus.top()]);

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
