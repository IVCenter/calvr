#include <cvrMenu/BubbleMenu.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrUtil/Intersection.h>
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

BubbleMenu::BubbleMenu()
{
    _myMenu = NULL;

    _border = 10.0;

    _doubleClickCutoff = 0.2;
    _timeLastButtonUp = 0.0;

    _hoverStartTime = 0.0;
    _hoverCutoff = 1.0;
    _activeIsect = NULL;
    _menuRoot = new osg::MatrixTransform();

    _activeHand = -1;

    std::string s;

    _distance = ConfigManager::getFloat("distance",
            "MenuSystem.BubbleMenu.Position",2000.0);
    _height = ConfigManager::getFloat("height","MenuSystem.BubbleMenu.Position",
        500.0);

    _radius = ConfigManager::getFloat("radius", 
        "MenuSystem.BubbleMenu.Spheres", 100.0);
    _tessellations = ConfigManager::getInt("tessellations", 
        "MenuSystem.BubbleMenu.Spheres", 12);
    _subradius = ConfigManager::getFloat("subradius", 
        "MenuSystem.BubbleMenu.Spheres", 500.0);
    _speed = ConfigManager::getFloat("value", 
        "MenuSystem.BubbleMenu.AnimationSpeed", 1.0);
    _textSize = ConfigManager::getFloat("value", 
        "MenuSystem.BubbleMenu.TextSize", 50.0);

    float r = ConfigManager::getFloat("r","MenuSystem.BubbleMenu.SphereColor",0.0),
          g = ConfigManager::getFloat("g","MenuSystem.BubbleMenu.SphereColor",1.0),
          b = ConfigManager::getFloat("b","MenuSystem.BubbleMenu.SphereColor",0.0),
          a = ConfigManager::getFloat("a","MenuSystem.BubbleMenu.SphereColor",1.0);
    _sphereColor = osg::Vec4(r,g,b,a);

    std::string sound = ConfigManager::getEntry("value", "MenuSystem.BubbleMenu.Sound", "off");

    _soundEnabled = (sound == "on"); 



    s = ConfigManager::getEntry("value","MenuSystem.BubbleMenu.Trigger",
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

    _primaryButton = ConfigManager::getInt("select",
            "MenuSystem.BubbleMenu.Buttons",0);

    _secondaryButton = ConfigManager::getInt("open",
            "MenuSystem.BubbleMenu.Buttons",1);

    _scale = ConfigManager::getFloat("MenuSystem.BubbleMenu.Scale",1.0);

    _menuScale = new osg::MatrixTransform();
    osg::Matrix scale;
    scale.makeScale(osg::Vec3(_scale,1.0,_scale));
    _menuScale->setMatrix(scale);

    _menuRoot->addChild(_menuScale);
    

    // Favorites menu
    _favMenuRoot = new osg::MatrixTransform();
    _favMenuScale = new osg::MatrixTransform();
    osg::Matrix favScale;
    favScale.makeScale(osg::Vec3(_scale,1.0,_scale));
    _favMenuScale->setMatrix(favScale);

    _favMenuRoot->addChild(_favMenuScale);
    
    _favMenu = new SubMenu("Favorites", "Favorites");

    osg::StateSet * favStateset = _favMenuRoot->getOrCreateStateSet();
    favStateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::StateSet * stateset = _menuRoot->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    _menuActive = false;
    _activeItem = NULL;

    _clickActive = false;

    // TODO: read values from config file
    BubbleMenuGeometry::_textColor = osg::Vec4(1.0,1.0,1.0,1.0);//osg::Vec4(1.0,1.0,1.0,1.0);
    BubbleMenuGeometry::_wireframeColor = osg::Vec4(0.0,1.0,0.0,1.0);
    BubbleMenuGeometry::_textColorSelected = osg::Vec4(0.0,1.0,0.0,1.0);
    BubbleMenuGeometry::_backgroundColor = osg::Vec4(0.0,0.0,0.0,1.0);
    BubbleMenuGeometry::_border = 10.0;
    BubbleMenuGeometry::_iconHeight = 30.0;
    BubbleMenuGeometry::_textSize = _textSize;
    BubbleMenuGeometry::_radius = _radius;
    BubbleMenuGeometry::_tessellations = _tessellations;
    BubbleMenuGeometry::_sphereColor = _sphereColor;

    std::string fontfile;

    _iconDir = CalVR::instance()->getHomeDir();
    BubbleMenuGeometry::_iconDir = _iconDir;
    fontfile = _iconDir;

    fontfile = fontfile + "/resources/ArenaCondensed.ttf";

    osgText::Font * font = osgText::readFontFile(fontfile);
    if(font)
    {
        BubbleMenuGeometry::_font = font;
    }
    else
    {
        std::cerr << "Warning: font file: " << fontfile << " not found."
                << std::endl;
    }
    BubbleMenuGeometry::calibrateTextSize(65.0);



/*
    std::string server = ConfigManager::getEntry("value", "MenuSystem.BubbleMenu.Sound.Server", "");
    int port = ConfigManager::getInt("value","MenuSystem.BubbleMenu.Sound.Port", 0);
            

    if (!oasclient::OASClientInterface::initialize(server, port))
    {
        std::cerr << "Could not set up connection to sound server!\n";
        _soundEnabled = false;
    }


    std::string path, file;
    path = ConfigManager::getEntry("path", "MenuSystem.BubbleMenu.Sound.ClickSound", "");
    file = ConfigManager::getEntry("file", "MenuSystem.BubbleMenu.Sound.ClickSound", "");

    // Set up click sound
    click = new oasclient::OASSound(path, file);
    if (!click->isValid())
    {
        std::cerr << "Could not create click sound!\n";
        _soundEnabled = false;
    }

    click->setGain(1.5);

    path = ConfigManager::getEntry("path", "MenuSystem.BubbleMenu.Sound.MenuSound", "");
    file = ConfigManager::getEntry("file", "MenuSystem.BubbleMenu.Sound.MenuSound", "");

    // Set up whoosh sound
    whoosh = new oasclient::OASSound(path, file);
    if (!whoosh->isValid())
    {
        std::cerr << "Could not create whoosh sound!\n";
        _soundEnabled = false;
    }

    */
}

BubbleMenu::~BubbleMenu()
{
    if(_menuActive)
    {
        SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
    }

    for(std::map<MenuItem *,BubbleMenuGeometry *>::iterator it =
            _geometryMap.begin(); it != _geometryMap.end(); it++)
    {
        delete it->second;
    }
    _geometryMap.clear();

    for(std::map<SubMenu*,std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> >::iterator it =
            _menuGeometryMap.begin(); it != _menuGeometryMap.end(); it++)
    {
        delete it->second.first;
        delete it->second.second;
    }
    _menuGeometryMap.clear();
}

void BubbleMenu::setMenu(SubMenu * menu)
{
    _myMenu = menu;
    _openMenus.push(menu);
    updateMenus();

    _menuScale->addChild(_menuMap[_myMenu]);
}

void BubbleMenu::updateStart()
{
    updateMenus();
    _foundItem = false;
}

void BubbleMenu::updateEnd()
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

bool BubbleMenu::processEvent(InteractionEvent * event)
{
    if(!_myMenu || !event->asTrackedButtonEvent())
    {
        return false;
    }

    TrackedButtonInteractionEvent * tie = event->asTrackedButtonEvent();

    if(!_menuActive && !_showFavMenu)
    {
        if(_trigger == DOUBLECLICK)
        {
            if(event->getInteraction() == BUTTON_DOUBLE_CLICK)
            {
                if(tie->getButton() == _secondaryButton)
                {
                    SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);

                    if (!_showFavMenu)
                    {
                        SceneManager::instance()->getMenuRoot()->addChild(_favMenuRoot);
                    }

                    osg::Vec3 menuPoint = osg::Vec3(0, _distance, 0);
                    osg::Vec3 menuStartPos = osg::Vec3(0, _distance, _height);
                    menuPoint = menuStartPos;

                    if(event->asMouseEvent())
                    {
                        osg::Vec3 menuOffset = osg::Vec3(
                                _widthMap[_myMenu] / 2.0,0,0);
                        osg::Matrix m;
                        m.makeTranslate(menuPoint);

                        m.makeTranslate(menuStartPos);
                        _menuRoot->setMatrix(m);
                    }
                    else
                    {
                        osg::Vec3 viewerPoint =
                                TrackingManager::instance()->getHeadMat(0).getTrans();

                        osg::Vec3 viewerDir = viewerPoint - menuPoint;
                        viewerDir.z() = 0.0;

                        osg::Vec3 menuOffset = osg::Vec3(
                                _widthMap[_myMenu] * _scale / 2.0,0,0);
                        _menuRoot->setMatrix(osg::Matrix::translate(menuPoint));
                    }

                    _menuActive = true;
                    SceneManager::instance()->closeOpenObjectMenu();
                    return true;
                }
            }
        }
        else if(_trigger == UPCLICK)
        {
            return false;
        }
    }

    else if (!_menuActive && _showFavMenu)
    {
        if(_trigger == DOUBLECLICK)
        {
            if(event->getInteraction() == BUTTON_DOUBLE_CLICK)
            {
                if(tie->getButton() == _secondaryButton)
                {
                    SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);

                    if (!_showFavMenu)
                    {
                        SceneManager::instance()->getMenuRoot()->addChild(_favMenuRoot);
                    }

                    osg::Vec3 menuPoint = osg::Vec3(0, _distance, 0);
                    osg::Vec3 menuStartPos = osg::Vec3(0, _distance, _height);
                    menuPoint = menuStartPos;

                    if(event->asMouseEvent())
                    {
                        osg::Vec3 menuOffset = osg::Vec3(
                                _widthMap[_myMenu] / 2.0,0,0);
                        osg::Matrix m;
                        m.makeTranslate(menuPoint);

                        m.makeTranslate(menuStartPos);
                        _menuRoot->setMatrix(m);
                    }
                    else
                    {
                        osg::Vec3 viewerPoint =
                                TrackingManager::instance()->getHeadMat(0).getTrans();

                        osg::Vec3 viewerDir = viewerPoint - menuPoint;
                        viewerDir.z() = 0.0;

                        osg::Vec3 menuOffset = osg::Vec3(
                                _widthMap[_myMenu] * _scale / 2.0,0,0);
                        _menuRoot->setMatrix(osg::Matrix::translate(menuPoint));
                    }

                    _menuActive = true;
                    SceneManager::instance()->closeOpenObjectMenu();
                    return true;
                }
            }
        }

        if(tie->getButton() == _primaryButton
                    && (tie->getInteraction() == BUTTON_UP))
        {
            _timeLastButtonUp = PluginHelper::getProgramDuration();
            _clickActive = false;
        }

        if(0)//_clickActive)
        {
            if(tie->getHand() == _activeHand)
            {
                if(tie->getButton() == _primaryButton)
                {
                    _activeItem->processEvent(event);
                    if(tie->getInteraction() == BUTTON_UP)
                    {
                        _clickActive = false;
                    }
                    return true;
                }
            }
            return false;
        }
        else if(tie->getHand() == _activeHand)
        {
            if (tie->getButton() == _primaryButton
                    && (tie->getInteraction() == BUTTON_DOWN))
            {
                _timeLastButtonUp = PluginHelper::getProgramDuration();
                _prevEvent = new InteractionEvent();
                _prevEvent->setInteraction(event->getInteraction());
                _prevActiveItem = _activeItem;
                _clickActive = true;
                return true;
            }

            if(tie->getButton() == _primaryButton
                    && tie->getInteraction() == BUTTON_DOUBLE_CLICK)
            {
                // do click
                //std::cout << "Double click" << std::endl;
                _prevEvent = NULL;
                
                MenuItem * item = _activeItem->getMenuItem();

                if (_favMenu->getItemPosition(item) < 0)
                {
                    _favMenu->addItem(item);
                }
                else
                {
                    _favMenu->removeItem(item);
                    
                    osg::MatrixTransform * mat = _favMaskMap[_favGeometryMap[item]];
            
                    osg::Matrix m = _favMenuRoot->getMatrix();

                    _lerpMap[mat] = new Lerp(mat->getMatrix().getTrans(), 
                        - m.getTrans() - osg::Vec3(0, -m.getTrans()[1], 0) 
                        + osg::Vec3(0, 0, _height) 
                        + _rootPositionMap[_favGeometryMap[item]],
                        _speed, 0, true, true);
                }

                return true;
            }
            return false;
        }


        if(tie->getButton() == _secondaryButton
                && tie->getInteraction() == BUTTON_DOWN)
        {
            if(_activeItem)
            {
                selectItem(NULL);
            }
            SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
            
            if (!_showFavMenu)
            {
                SceneManager::instance()->getMenuRoot()->removeChild(_favMenuRoot);
            }
           
            _menuActive = false;
            return true;
        }
    }
    
    else
    {
        if(tie->getButton() == _primaryButton
                    && (tie->getInteraction() == BUTTON_UP))
        {
            _timeLastButtonUp = PluginHelper::getProgramDuration();
            _clickActive = false;
        }

        if(0)//_clickActive)
        {
            if(tie->getHand() == _activeHand)
            {
                if(tie->getButton() == _primaryButton)
                {
                    _activeItem->processEvent(event);
                    if(tie->getInteraction() == BUTTON_UP)
                    {
                        _clickActive = false;
                    }
                    return true;
                }
            }
            return false;
        }
        else if(tie->getHand() == _activeHand)
        {
            if (tie->getButton() == _primaryButton
                    && (tie->getInteraction() == BUTTON_DOWN))
            {
                _timeLastButtonUp = PluginHelper::getProgramDuration();
                _prevEvent = new InteractionEvent();
                _prevEvent->setInteraction(event->getInteraction());
                _prevActiveItem = _activeItem;
                _clickActive = true;
                
                if (_soundEnabled)
                {
//                    click->play();
                }

                return true;
            }

            if(tie->getButton() == _primaryButton
                    && tie->getInteraction() == BUTTON_DOUBLE_CLICK)
            {
                // do click
                //std::cout << "Double click" << std::endl;
                _prevEvent = NULL;
                
                MenuItem * item = _activeItem->getMenuItem();

                if (_favMenu->getItemPosition(item) < 0)
                {
                    _favMenu->addItem(item);
                }
                else
                {
                    _favMenu->removeItem(item);

                    osg::MatrixTransform * mat = _favMaskMap[_favGeometryMap[item]];
            
                    /*
                    _lerpMap[mat] = new Lerp(mat->getMatrix().getTrans(), 
                        -_favMenuRoot->getMatrix().getTrans() + 
                        _rootPositionMap[_favGeometryMap[item]] + osg::Vec3(0,0,_radius),
                        _speed, 0, true, true);
                     */
                     
                    if (mat)
                    {
                     osg::Matrix m = _favMenuRoot->getMatrix();

                     _lerpMap[mat] = new Lerp(mat->getMatrix().getTrans(), 
                        - m.getTrans() - osg::Vec3(0, -m.getTrans()[1], 0) 
                        + osg::Vec3(0, 0, _height) 
                        + _rootPositionMap[_favGeometryMap[item]],
                        _speed, 0, true, true);
                    }
                    
                }

                /*
                if(_activeItem)
                {
                    BubbleMenuSubMenuGeometry * smg =
                            dynamic_cast<BubbleMenuSubMenuGeometry *>(_activeItem);
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
                }*/
                return true;
            }
            return false;
        }


        if(tie->getButton() == _secondaryButton
                && tie->getInteraction() == BUTTON_DOWN)
        {
            if(_activeItem)
            {
                selectItem(NULL);
            }
            SceneManager::instance()->getMenuRoot()->removeChild(_menuRoot);
            
            if (!_showFavMenu)
            {
                SceneManager::instance()->getMenuRoot()->removeChild(_favMenuRoot);
            }
           
            _menuActive = false;
            return true;
        }
    }
    return false;
}

void BubbleMenu::itemDelete(MenuItem * item)
{
    if(!_myMenu)
    {
        return;
    }

    std::vector<SubMenu*> searchList;
    searchList.push_back(_myMenu);
    std::vector<std::pair<SubMenu*,MenuItem*> > removeList;

    while(searchList.size())
    {
        for(std::vector<MenuItem*>::iterator it =
                searchList[0]->getChildren().begin();
                it != searchList[0]->getChildren().end(); it++)
        {
            if((*it)->isSubMenu())
            {
                if((*it) == item)
                {
                    removeList.push_back(
                            std::pair<SubMenu*,MenuItem*>(searchList[0],(*it)));
                    continue;
                }
                else
                {
                    SubMenu * sm = dynamic_cast<SubMenu*>(*it);
                    searchList.push_back(sm);
                }
            }
            else if((*it) == item)
            {
                removeList.push_back(
                        std::pair<SubMenu*,MenuItem*>(searchList[0],(*it)));
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
        for(std::map<osg::Geode *,BubbleMenuGeometry*>::iterator it =
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
}

void BubbleMenu::clear()
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

    for(std::map<MenuItem *,BubbleMenuGeometry *>::iterator it =
            _geometryMap.begin(); it != _geometryMap.end(); it++)
    {
        delete it->second;
    }
    _geometryMap.clear();

    for(std::map<SubMenu*,std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> >::iterator it =
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

void BubbleMenu::close()
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

void BubbleMenu::setScale(float scale)
{
    _scale = scale;
    osg::Matrix m;
    m.makeScale(osg::Vec3(_scale,1.0,_scale));
    _menuScale->setMatrix(m);
}

float BubbleMenu::getScale()
{
    return _scale;
}

void BubbleMenu::updateMenus()
{
    if(!_myMenu)
    {
        return;
    }
    
    // hide hover text on non-hover items
    for (std::map<MenuItem *,BubbleMenuGeometry *>::iterator it = 
        _geometryMap.begin(); it != _geometryMap.end(); ++it)
    {
        if (it->second)
            it->second->hideHoverText();
    }

    for (std::map<SubMenu*,std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> >::iterator
        it = _menuGeometryMap.begin(); it != _menuGeometryMap.end(); ++it)
    {
        if (it->second.second)
            it->second.second->hideHoverText();
    }

    // process saved event if double click cutoff time has passed
    if (((PluginHelper::getProgramDuration() - _timeLastButtonUp) >
         _doubleClickCutoff) && _prevEvent && !_clickActive)
    {
        if(_prevActiveItem)
        {
            BubbleMenuSubMenuGeometry * smg =
                    dynamic_cast<BubbleMenuSubMenuGeometry *>(_prevActiveItem);
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
            _prevActiveItem->processEvent(_prevEvent);
        }
        _prevEvent = NULL;
    }
   
    // Update sphere animations
    for(std::map<osg::ref_ptr<osg::MatrixTransform>,Lerp*>::iterator it =
        _lerpMap.begin(); it != _lerpMap.end(); it++)
    {
        osg::ref_ptr<osg::MatrixTransform> node = it->first;
        if (it->second->isDone())
        {
            osg::Matrix m = node->getMatrix();
            m.setTrans(it->second->getEnd());
            node->setMatrix(m);
            if (it->second->isHideOnFinish())
            {
                node->setNodeMask(0x0);
            }
            else
            {
                node->setNodeMask(0xFFFFFF);
            }

            delete it->second;
            _lerpMap.erase(it);
        }
        else if (it->second->isDelayed())
        {
            if (it->second->isHideOnDelay())
            {
                node->setNodeMask(0x0);
            }
            else 
            {
                node->setNodeMask(0xFFFFFF);
            }
        }
        else
        {
            osg::Matrix m = node->getMatrix();
            m.setTrans(it->second->getPosition());
            node->setMatrix(m);
            node->setNodeMask(0xFFFFFF);
        }
    }

    std::vector<SubMenu*> searchList;
    std::vector<SubMenu*> foundList;

    searchList.push_back(_myMenu);
    foundList.push_back(_myMenu);

    // search through menu tree, make list of all submenus, 
    // dirty all submenus with a dirty child
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

        std::vector<BubbleMenuGeometry *> geoList;

        // foundList[i] not in menuMap, so add it
        if(_menuMap.find(foundList[i]) == _menuMap.end())
        {
            _menuMap[foundList[i]] = new osg::MatrixTransform();
        }
        
        // remove all children
        _menuMap[foundList[i]]->removeChildren(0,
                _menuMap[foundList[i]]->getNumChildren());
        
        // create uncreated submenu geometry (?)
        if(_menuGeometryMap.find(foundList[i]) == _menuGeometryMap.end())
        {
            _menuGeometryMap[foundList[i]] = std::pair<BubbleMenuGeometry*,
               BubbleMenuGeometry*>(createBubbleMenuGeometry(foundList[i],true),
               createBubbleMenuGeometry(foundList[i]));
        }
        
        if(_menuGeometryMap[foundList[i]].first)
        {
            geoList.push_back(_menuGeometryMap[foundList[i]].first);
        }
        
        // create children
        for(std::vector<MenuItem*>::iterator it =
                foundList[i]->getChildren().begin();
                it != foundList[i]->getChildren().end(); it++)
        {
            BubbleMenuGeometry * mg;

            if((*it)->isSubMenu())
            {
                if(_menuGeometryMap.find((SubMenu*)(*it))
                        == _menuGeometryMap.end())
                {
                    _menuGeometryMap[(SubMenu*)(*it)] = std::pair<
                            BubbleMenuGeometry*,BubbleMenuGeometry*>(
                            createBubbleMenuGeometry(*it,true),
                            createBubbleMenuGeometry(*it));
                }
                mg = _menuGeometryMap[(SubMenu*)(*it)].second;
            }
            else if(_geometryMap.find(*it) == _geometryMap.end())
            {
                _geometryMap[*it] = createBubbleMenuGeometry(*it);
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
        float padding = _radius / 3;

        for(int j = 0; j < geoList.size(); j++)
        {
            width += 2 * _radius + padding;
        }

        width -= padding;
        _widthMap[foundList[i]] = width;

        // add invisible intersection test drawable
        for(int j = 0; j < geoList.size(); j++)
        {
            geoList[j]->resetIntersect(2*_radius);
        }

        // add line under menu title
        if(geoList.size() > 0)
        {
            BubbleMenuSubMenuGeometry * smg =
                    dynamic_cast<BubbleMenuSubMenuGeometry *>(geoList[0]);
            if(smg)
            {
                // show the menu title only for the first menu
                if (i != 0)
                    smg->getNode()->setNodeMask(0x0);
            }
        }

        float offset = _border;
        float interval = (M_PI * 2) / (geoList.size() - 1);
        float theta; 
        osg::Vec3 center;

        if (i == 0)
        {
            center = osg::Vec3(0, 0, 0);
            offset = -width/2;
            
            osg::Matrix m;
            m.makeTranslate(osg::Vec3(offset, _distance, _height - _radius - padding));
            _favMenuRoot->setMatrix(m);
        }
        else
        {
            // possibly make this height configurable
            center = osg::Vec3(0, 0, -_subradius * 1.5);
        }
        for(int j = 0; j < geoList.size(); j++)
        {
            osg::Matrix m;
            osg::Matrix rootMat;
            osg::MatrixTransform * mat = new osg::MatrixTransform();

            // main menu - horizontal
            if (i == 0)
            {
                m.makeTranslate(center + osg::Vec3(offset,0,0));
                mat->setMatrix(m);

                offset += 2*_radius + padding;
                _rootPositionMap[geoList[j]] = m.getTrans();
            }
            // submenus - circular
            else
            {
                if (j == 0)
                {
                    m.makeTranslate(center);
                }
                else
                {
                    theta = j * interval; 
                    m.makeTranslate(center + 
                        osg::Vec3(_subradius * cos(theta), 0, _subradius * sin(theta)));

                }
                mat->setMatrix(m);

                _rootPositionMap[geoList[j]] = 
                    _rootPositionMap[_menuGeometryMap[foundList[i]].second];
            }

            _positionMap[geoList[j]] = m.getTrans();

            mat->addChild(geoList[j]->getNode());
            _menuMap[foundList[i]]->addChild(mat);

            _maskMap[geoList[j]] = mat;
            _intersectMap[geoList[j]->getIntersect()] = geoList[j];
        }
        foundList[i]->setDirty(false);
    }

    float padding = _radius / 3;
    float offset = 0;

/*
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


    osg::Matrix m;
    m.makeTranslate(osg::Vec3(-offset,0,0));

    if(revMenuStack.size())
    {
        _menuMap[revMenuStack.top()]->setMatrix(m);
        _openMenus.push(revMenuStack.top());
        revMenuStack.pop();
    }

    int count = revMenuStack.size();
    int max = revMenuStack.size();
    offset += 2*_radius + padding;

    while(revMenuStack.size())
    {
        m.makeTranslate(osg::Vec3(0,0,0));

        if (count == 1)
        {
            offset += 2*_radius + padding;
        }
        else
        {
        }

        _openMenus.push(revMenuStack.top());
        revMenuStack.pop();
        count--;
    }
*/


    // favorites menu
    offset = _radius + padding;
    _favMenuRoot->removeChild(0, _favMenuRoot->getNumChildren());
    
    // find and remove menu items that still have geometry, but are not menu children
    for (std::map<MenuItem*, BubbleMenuGeometry*>::iterator it =
         _favGeometryMap.begin(); it != _favGeometryMap.end(); ++it)
    {
        // moving out of menu
        if (_favMenu->getItemPosition(it->first) < 0)
        {
            if (_lerpMap.find(_favMaskMap[it->second]) != _lerpMap.end())
            {
                BubbleMenuGeometry * mg = it->second;
                osg::ref_ptr<osg::MatrixTransform> mat;

                mat = _favMaskMap[mg];

                mg->resetIntersect(2*_radius);
                _intersectMap[mg->getIntersect()] = mg;
               
                mat->addChild(mg->getNode());
                _favMenuRoot->addChild(mat);
            }
            else
            {
                _favGeometryMap.erase(it);
            }
        }
    }

    for(std::vector<MenuItem*>::iterator it =
        _favMenu->getChildren().begin();
        it != _favMenu->getChildren().end(); it++)

    {
        osg::Matrix m;
        osg::Matrix rootMat = _favMenuRoot->getMatrix();
        BubbleMenuGeometry * mg;
        osg::ref_ptr<osg::MatrixTransform> mat;

        if (_geometryMap.find(*it) == _geometryMap.end() &&
            _menuGeometryMap.find((SubMenu*)*it) == _menuGeometryMap.end())
        {
            continue;
        }
        
        if ((*it)->isSubMenu())
        {
            std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> bmgpair = 
                _menuGeometryMap[(SubMenu*)(*it)]; 
            mg = bmgpair.second;
        }
        else
        {
            mg = _geometryMap[*it];
        }
        
        // added to menu
        if (_favGeometryMap.find(*it) == _favGeometryMap.end())
        {
            mat = new osg::MatrixTransform();

            osg::Vec3 vec(0, 0, -offset);
            _lerpMap[mat] = new Lerp(-rootMat.getTrans() - osg::Vec3(0,-rootMat.getTrans()[1],0)
                    + osg::Vec3(0,0,_height) + _maskMap[mg]->getMatrix().getTrans(),
                    vec, _speed);

            _favGeometryMap[*it] = mg;
            _favMaskMap[mg] = mat;
        }
        
        // in menu - not moving
        if (_lerpMap.find(_favMaskMap[mg]) == _lerpMap.end())
        {
            mat = new osg::MatrixTransform();
            m.makeTranslate(osg::Vec3(0, 0, -offset));
            mat->setMatrix(m);
            _favMaskMap[mg] = mat;
        }
        // moving into menu
        else
        {
            mat = _favMaskMap[mg];
        }

        mg->resetIntersect(2*_radius);
        _intersectMap[mg->getIntersect()] = mg;
       
        mat->addChild(mg->getNode());
        _favMenuRoot->addChild(mat);
       
        offset += 2 * _radius + padding;
    }
}

bool BubbleMenu::processIsect(IsectInfo & isect, int hand)
{
    if(!_menuActive && !_showFavMenu)
    {
        return false;
    }
    else if(_clickActive)
    {
        return true;
    }
    
    if (_menuActive)
    {
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
                    return true;
                }
            }

            if (_activeIsect != _intersectMap[isect.geode])
            {
                _hoverStartTime = PluginHelper::getProgramDuration();
                _activeIsect = _intersectMap[isect.geode];
            }

            if (PluginHelper::getProgramDuration() - _hoverStartTime > _hoverCutoff)
            {
                _intersectMap[isect.geode]->showHoverText();
                _activeIsect = _intersectMap[isect.geode];
            }

            _activeHand = hand;
            selectItem(_intersectMap[isect.geode]);
            _foundItem = true;
            return true;
        }
    }

    else if (!_menuActive && _showFavMenu)
    {
        if(_intersectMap.find(isect.geode) != _intersectMap.end() 
           && _favGeometryMap[_intersectMap[isect.geode]->getMenuItem()] == _intersectMap[isect.geode])
        {
            TrackerBase::TrackerType ttype =
                    TrackingManager::instance()->getHandTrackerType(hand);
            if(_activeHand >= 0 && _activeHand != hand)
            {
                if(ttype
                        >= TrackingManager::instance()->getHandTrackerType(
                                _activeHand))
                {
                    return true;
                }
            }

            if (_activeIsect != _intersectMap[isect.geode])
            {
                _hoverStartTime = PluginHelper::getProgramDuration();
                _activeIsect = _intersectMap[isect.geode];
            }

            if (PluginHelper::getProgramDuration() - _hoverStartTime > _hoverCutoff)
            {
                _intersectMap[isect.geode]->showHoverText();
                _activeIsect = _intersectMap[isect.geode];
            }

            _activeHand = hand;
            selectItem(_intersectMap[isect.geode]);
            _foundItem = true;
            return true;
        }
    }


    return false;
}

void BubbleMenu::selectItem(BubbleMenuGeometry * mg)
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

void BubbleMenu::openMenu(BubbleMenuSubMenuGeometry * smg)
{
    bool isOpenMenu = false;

    if (_soundEnabled)
    {
//        whoosh->play();
    }

    osg::Vec3 centerPos = osg::Vec3(0, 0, -_subradius*1.5);

    // a menu is open
    if (_openMenus.size() > 1)
    {
        closeMenu(_openMenus.top());

        // Lerp smg to center with a delay
        _lerpMap[_maskMap[smg]] = new Lerp(_maskMap[smg]->getMatrix().getTrans(),
            centerPos, _speed, _speed);
        isOpenMenu = true;
    }
    // no menu open
    else
    {
        // Lerp smg to center immediately
        _lerpMap[_maskMap[smg]] = new Lerp(_maskMap[smg]->getMatrix().getTrans(),
            centerPos, _speed);
    }

    // open children of smg
    for (int i = 0; i < ((SubMenu*)smg->getMenuItem())->getNumChildren(); i++)
    {
        BubbleMenuGeometry * bmg = 
            _geometryMap[((SubMenu*)smg->getMenuItem())->getChild(i)]; 
        osg::Vec3 pos = _positionMap[bmg];

        if (bmg)
        {
            if (isOpenMenu)
            {
                _lerpMap[_maskMap[bmg]] = new Lerp(centerPos, pos, _speed, 
                    2*_speed, false, true);
            }
            else
            {
                _lerpMap[_maskMap[bmg]] = new Lerp(centerPos, pos, _speed, 
                    _speed, false, true);
            }
        }
        else // is a submenu
        {
            std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> bmgpair = 
                _menuGeometryMap[(SubMenu*)((SubMenu*)smg->getMenuItem())->getChild(i)]; 
            
            osg::Vec3 pos = bmgpair.second->getNode()->getMatrix().getTrans();
            pos = _positionMap[bmgpair.second];

            if (isOpenMenu)
            {
                _lerpMap[_maskMap[bmgpair.second]] = new Lerp(centerPos, 
                    pos, _speed, 2*_speed, false, true);
            }
            else
            {
                _lerpMap[_maskMap[bmgpair.second]] = new Lerp(centerPos,
                    pos, _speed, _speed, false, true);
            }

        }
    }

    _openMenus.push((SubMenu*)smg->getMenuItem());

    osg::Vec3 pos = _menuMap[_openMenus.top()]->getMatrix().getTrans();

    osg::Matrix m;
    m.makeTranslate(pos);

    _menuMap[(SubMenu*)smg->getMenuItem()]->setMatrix(m);
    _menuScale->addChild(_menuMap[(SubMenu*)smg->getMenuItem()]);

    smg->openMenu(true);
}

void BubbleMenu::closeMenu(SubMenu * menu)
{
    
    if (_soundEnabled)
    {
//        whoosh->play();
    }

    // find the geometry corresponding to menu
    BubbleMenuSubMenuGeometry * smg = NULL;
    for(std::map<osg::Geode *,BubbleMenuGeometry*>::iterator it =
            _intersectMap.begin(); it != _intersectMap.end(); it++)
    {
        if(it->second->getMenuItem() == _openMenus.top())
        {
            smg = (BubbleMenuSubMenuGeometry*)(it->second);
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

    std::map<osg::ref_ptr<osg::MatrixTransform>,Lerp*>::iterator it;
    it = _lerpMap.find(_maskMap[smg]);
    if (it == _lerpMap.end())
    {
        osg::Vec3 startPos = _maskMap[smg]->getMatrix().getTrans();
        osg::Vec3 newPos = _rootPositionMap[smg];

        bool isRoot = false; 

        for (std::vector<MenuItem*>::iterator it = _myMenu->getChildren().begin();
             it != _myMenu->getChildren().end(); ++it)
        {
            if ((*it) == smg->getMenuItem())
            {
                isRoot = true;
            }
        }
        if (isRoot)
        {
            _lerpMap[_maskMap[smg]] = 
                new Lerp(startPos, newPos, _speed, _speed);
        }
        else
        {
            _lerpMap[_maskMap[smg]] = 
                new Lerp(startPos, newPos, _speed, _speed, true, false);

        }
    }

    for (int i = 0; i < ((SubMenu*)smg->getMenuItem())->getNumChildren(); i++)
    {
       osg::Vec3 centerPos = osg::Vec3(0, 0, -_subradius * 1.5);
       BubbleMenuGeometry * bmg = 
         _geometryMap[((SubMenu*)smg->getMenuItem())->getChild(i)]; 
       if (bmg)
       {
           _lerpMap[_maskMap[bmg]] = 
               new Lerp(_maskMap[bmg]->getMatrix().getTrans(), centerPos, _speed, 0, true);
       }
       else // is a submenu
       {
           std::pair<BubbleMenuGeometry*,BubbleMenuGeometry*> bmgpair = 
               _menuGeometryMap[(SubMenu*)((SubMenu*)smg->getMenuItem())->getChild(i)]; 
            
           _lerpMap[_maskMap[bmgpair.second]] = 
               new Lerp(_maskMap[bmgpair.second]->getMatrix().getTrans(),
                    centerPos, _speed, 0, true);
       }
    }

    smg->openMenu(false);
    _openMenus.pop();
}

void BubbleMenu::setFavorites(bool showFav)
{
    _showFavMenu = showFav;
}

