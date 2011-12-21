#include <menu/BoardMenu/BoardMenuTextButtonSetGeometry.h>
#include <menu/MenuTextButtonSet.h>

#include <util/LocalToWorldVisitor.h>
#include <util/Intersection.h>

#include <kernel/SceneManager.h>
#include <kernel/NodeMask.h>

#include <osg/Geometry>

#include <iostream>

using namespace cvr;

BoardMenuTextButtonSetGeometry::BoardMenuTextButtonSetGeometry() :
        BoardMenuGeometry()
{
    _intersectedButton = "";
}

BoardMenuTextButtonSetGeometry::~BoardMenuTextButtonSetGeometry()
{
    for(std::map<std::string,TextButtonGeometry*>::iterator it =
            _buttonMap.begin(); it != _buttonMap.end(); it++)
    {
        delete it->second;
    }
}

void BoardMenuTextButtonSetGeometry::selectItem(bool on)
{
    MenuTextButtonSet * mb = dynamic_cast<MenuTextButtonSet*>(_item);
    if(!on)
    {
        std::map<std::string,TextButtonGeometry*>::iterator oldButton =
                _buttonMap.find(_intersectedButton);
        if(oldButton != _buttonMap.end())
        {
            if(!mb->getValue(_intersectedButton))
            {
                oldButton->second->textTransform->removeChild(
                        oldButton->second->text);
                oldButton->second->textTransform->removeChild(
                        oldButton->second->textSelected);
                oldButton->second->textTransform->addChild(
                        oldButton->second->text);
            }
        }

        _intersectedButton = "";
    }
}

void BoardMenuTextButtonSetGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _item = item;

    _spacing = 3.0;

    MenuTextButtonSet * mb = dynamic_cast<MenuTextButtonSet*>(item);
    if(!mb)
    {
        std::cerr << "Error creating BoardMenuTextButtonSetGeometry."
                << std::endl;
    }

    _rowHeight = mb->getRowHeight();
    _buttonWidth = (mb->getWidth() - (mb->getButtonsPerRow() - 1) * _spacing)
            / ((float)(mb->getButtonsPerRow()));

    for(int i = 0; i < mb->getNumButtons(); i++)
    {
        _buttonMap[mb->getButton(i)] = createButtonGeometry(mb->getButton(i));
        _node->addChild(_buttonMap[mb->getButton(i)]->root);
    }

    updateButtons(mb);

    //_text = makeText(mb->getText(), _textSize, osg::Vec3(_rowHeight + _boarder, -2, -_rowHeight / 2.0), _textColor);

    //osg::BoundingBox bb = _text->getBound();
    _width = mb->getWidth();
    //mg->height = bb.zMax() - bb.zMin();
    //_height = _rowHeight;
}

void BoardMenuTextButtonSetGeometry::processEvent(InteractionEvent * event)
{
    MenuTextButtonSet * mb = dynamic_cast<MenuTextButtonSet*>(_item);

    switch(event->getInteraction())
    {
        case BUTTON_DOWN:
        case BUTTON_DOUBLE_CLICK:
        {
            std::map<std::string,TextButtonGeometry*>::iterator currentButton =
                    _buttonMap.find(_intersectedButton);
            if(currentButton != _buttonMap.end())
            {
                int index = mb->getButtonNumber(currentButton->first);
                if(index != -1)
                {
                    mb->setValue(index,!mb->getValue(index));
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }

            if(_item->getCallback())
            {
                _item->getCallback()->menuCallback(_item);
            }
            break;
        }
        default:
            break;
    }
}

void BoardMenuTextButtonSetGeometry::updateGeometry()
{
    MenuTextButtonSet * mb = dynamic_cast<MenuTextButtonSet*>(_item);
    std::vector<std::string> eraseList;
    for(std::map<std::string,TextButtonGeometry *>::iterator it =
            _buttonMap.begin(); it != _buttonMap.end(); it++)
    {
        if(mb->getButtonNumber(it->first) == -1)
        {
            eraseList.push_back(it->first);
            delete it->second;
        }
    }

    for(int i = 0; i < eraseList.size(); i++)
    {
        _buttonMap.erase(eraseList[i]);
    }

    for(int i = 0; i < mb->getNumButtons(); i++)
    {
        if(_buttonMap.find(mb->getButton(i)) == _buttonMap.end())
        {
            _buttonMap[mb->getButton(i)] = createButtonGeometry(
                    mb->getButton(i));
            _node->addChild(_buttonMap[mb->getButton(i)]->root);
        }
    }

    updateButtons(mb);
}

void BoardMenuTextButtonSetGeometry::update(osg::Vec3 & pointerStart,
        osg::Vec3 & pointerEnd)
{
    MenuTextButtonSet * mb = dynamic_cast<MenuTextButtonSet*>(_item);
    //std::cerr << "pointer start x: " << pointerStart.x() << " y: " << pointerStart.y() << " z: " << pointerStart.z() << std::endl;
    //std::cerr << "pointer end x: " << pointerEnd.x() << " y: " << pointerEnd.y() << " z: " << pointerEnd.z() << std::endl;
    /*osg::Matrix l2w = getLocalToWorldMatrix(_node) * osg::Matrix::inverse(_node->getMatrix());
     osg::Matrix w2l = osg::Matrix::inverse(l2w);

     std::vector<IsectInfo> isecvec;

     pointerStart = pointerStart * w2l;
     pointerEnd = pointerEnd * w2l;

     //std::cerr << "button space pointer start x: " << pointerStart.x() << " y: " << pointerStart.y() << " z: " << pointerStart.z() << std::endl;
     //std::cerr << "button space pointer end x: " << pointerEnd.x() << " y: " << pointerEnd.y() << " z: " << pointerEnd.z() << std::endl;

     osg::Vec3 norm = pointerEnd - pointerStart;
     norm.normalize();
     float t = -pointerStart.y() / norm.y();
     osg::Vec3 screenpoint = pointerStart + (norm * t);
     std::cerr << "Screen Point x: " << screenpoint.x() << " y: " << screenpoint.y() << " z: " << screenpoint.z() << std::endl;

     isecvec = getObjectIntersection(_node,pointerStart, pointerEnd);*/
    std::vector<IsectInfo> isecvec;
    isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
            pointerStart,pointerEnd);
    //std::cerr << "isec size: " << isecvec.size() << std::endl;
    for(int i = 0; i < isecvec.size(); i++)
    {
        for(std::map<std::string,TextButtonGeometry*>::iterator it =
                _buttonMap.begin(); it != _buttonMap.end(); it++)
        {
            if(isecvec[i].geode == it->second->quad
                    || isecvec[i].geode == it->second->quadSelected)
            {
                std::map<std::string,TextButtonGeometry*>::iterator oldButton =
                        _buttonMap.find(_intersectedButton);
                if(oldButton != _buttonMap.end())
                {
                    if(!mb->getValue(_intersectedButton))
                    {
                        oldButton->second->textTransform->removeChild(
                                oldButton->second->text);
                        oldButton->second->textTransform->removeChild(
                                oldButton->second->textSelected);
                        oldButton->second->textTransform->addChild(
                                oldButton->second->text);
                    }
                }

                _intersectedButton = it->first;

                if(!mb->getValue(it->first))
                {
                    it->second->textTransform->removeChild(
                            it->second->textSelected);
                    it->second->textTransform->removeChild(it->second->text);
                    it->second->textTransform->addChild(
                            it->second->textSelected);
                }

                return;
            }
        }
    }

    std::map<std::string,TextButtonGeometry*>::iterator oldButton =
            _buttonMap.find(_intersectedButton);
    if(oldButton != _buttonMap.end())
    {
        if(!mb->getValue(_intersectedButton))
        {
            oldButton->second->textTransform->removeChild(
                    oldButton->second->text);
            oldButton->second->textTransform->removeChild(
                    oldButton->second->textSelected);
            oldButton->second->textTransform->addChild(oldButton->second->text);
        }
    }

    _intersectedButton = "";
}

BoardMenuTextButtonSetGeometry::TextButtonGeometry * BoardMenuTextButtonSetGeometry::createButtonGeometry(
        std::string text)
{
    TextButtonGeometry * tbg = new TextButtonGeometry;

    tbg->root = new osg::MatrixTransform();
    tbg->textTransform = new osg::MatrixTransform();
    tbg->quad = new osg::Geode();
    tbg->quadSelected = new osg::Geode();
    tbg->text = new osg::Geode();
    tbg->textSelected = new osg::Geode();

    tbg->root->addChild(tbg->textTransform);

    tbg->textTransform->setNodeMask(
            tbg->textTransform->getNodeMask() & ~(INTERSECT_MASK));

    osgText::Text * textd = makeText(text,_textSize,osg::Vec3(0,0,0),_textColor,
            osgText::Text::CENTER_CENTER);
    tbg->text->addDrawable(textd);

    textd = makeText(text,_textSize,osg::Vec3(0,0,0),_textColorSelected,
            osgText::Text::CENTER_CENTER);
    tbg->textSelected->addDrawable(textd);

    osg::BoundingBox bb = textd->getBound();
    osg::Vec3 scale, trans;
    float scalef;
    if((_buttonWidth * 0.85) / (bb.xMax() - bb.xMin())
            > (_rowHeight * 0.85) / (bb.zMax() - bb.zMin()))
    {
        scalef = (_rowHeight * 0.85) / (bb.zMax() - bb.zMin());
    }
    else
    {
        scalef = (_buttonWidth * 0.85) / (bb.xMax() - bb.xMin());
    }

    scale = osg::Vec3(scalef,1.0,scalef);
    trans = osg::Vec3(_buttonWidth / 2.0,-3,-_rowHeight / 2.0);

    osg::Matrix s, t;
    s.makeScale(scale);
    t.makeTranslate(trans);

    tbg->textTransform->setMatrix(s * t);

    osg::Geometry * geo = makeQuad(_buttonWidth,_rowHeight,
            osg::Vec4(0.2,0.2,0.2,1.0),osg::Vec3(0,-2,-_rowHeight));

    tbg->quad->addDrawable(geo);

    geo = makeQuad(_buttonWidth,_rowHeight,osg::Vec4(0.7,0.7,0.7,1.0),
            osg::Vec3(0,-2,-_rowHeight));

    tbg->quadSelected->addDrawable(geo);

    return tbg;
}

void BoardMenuTextButtonSetGeometry::updateButtons(MenuTextButtonSet * tbs)
{
    float hposition = 0.0;
    float vposition = 0.0;
    for(int i = 0; i < tbs->getNumButtons(); i++)
    {
        TextButtonGeometry * tbg = _buttonMap[tbs->getButton(i)];
        tbg->root->removeChild(tbg->quad);
        tbg->root->removeChild(tbg->quadSelected);
        if(tbs->getValue(i))
        {
            tbg->root->addChild(tbg->quadSelected);
        }
        else
        {
            tbg->root->addChild(tbg->quad);
        }

        tbg->textTransform->removeChild(tbg->text);
        tbg->textTransform->removeChild(tbg->textSelected);

        if(tbs->getValue(i) || tbs->getButton(i) == _intersectedButton)
        {
            tbg->textTransform->addChild(tbg->textSelected);
        }
        else
        {
            tbg->textTransform->addChild(tbg->text);
        }

        if(i && !(i % tbs->getButtonsPerRow()))
        {
            hposition = 0;
            vposition += _rowHeight + _spacing;
        }

        osg::Matrix trans;
        trans.makeTranslate(osg::Vec3(hposition,0,-vposition));
        tbg->root->setMatrix(trans);

        hposition += _buttonWidth + _spacing;
    }

    _height = vposition + _rowHeight;
}
