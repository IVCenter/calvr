#include <cvrMenu/BoardMenu/BoardMenuItemGroupGeometry.h>
#include <cvrMenu/BoardMenu.h>
#include <cvrMenu/MenuItemGroup.h>

using namespace cvr;

BoardMenuItemGroupGeometry::BoardMenuItemGroupGeometry(BoardMenu * menu)
{
    _menu = menu;
    _mig = NULL;
}

BoardMenuItemGroupGeometry::~BoardMenuItemGroupGeometry()
{
}

void BoardMenuItemGroupGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();
    _group = new osg::Group();
    _node->addChild(_group);

    _item = item;

    MenuItemGroup * mig = dynamic_cast<MenuItemGroup*>(item);
    if(!mig)
    {
	std::cerr
                << "BoardMenuItemGroupGeometry: Error, item is not of MenuItemGroup type."
                << std::endl;
        return;
    }
    _mig = mig;

    updateLayout(0.0f);
}

void BoardMenuItemGroupGeometry::processEvent(InteractionEvent * event)
{
}

void BoardMenuItemGroupGeometry::updateGeometry()
{
    updateLayout(0.0f);
}

void BoardMenuItemGroupGeometry::resetIntersect(float width)
{
    updateLayout(width);
    BoardMenuGeometry::resetIntersect(width);
}

void BoardMenuItemGroupGeometry::updateLayout(float width)
{
    _group->removeChildren(0,_group->getNumChildren());

    std::vector<BoardMenuGeometry*> geoList;
    std::vector<bool> colStatus;
    for(int i = 0; i < _mig->getChildren().size(); ++i)
    {
	BoardMenuGeometry * bmg = _menu->getItemGeometry(_mig->getChild(i));
	if(bmg)
	{
	    geoList.push_back(bmg);
	    colStatus.push_back(_mig->getChild(i)->isCollection() && !_mig->getChild(i)->isSubMenu());
	}
    }
    
    if(_mig->getLayoutHint() == MenuItemGroup::ROW_LAYOUT)
    {
	if(width > 0.0f)
	{
	    // use width to place items
	    
	    float spacing;
	    float left;

	    switch(_mig->getAlignmentHint())
	    {
		case MenuItemGroup::ALIGN_CENTER:
		{
		    float twidth = 0.0;
		    float theight = 0.0;
		    // calc width/height
		    for(int i = 0; i < geoList.size(); ++i)
		    {
			if(geoList[i]->getHeight() > theight)
			{
			    theight = geoList[i]->getHeight();
			}
			twidth += geoList[i]->getWidth();
		    }

		    spacing = (width - twidth);
		    if(geoList.size())
		    {
			spacing /= ((float)(geoList.size()));
		    }
		    left = spacing / 2.0;
		    break;
		}
		case MenuItemGroup::ALIGN_LEFT:
		    spacing = _iconHeight * 0.5;
		    left = spacing / 2.0;
		    break;
		case MenuItemGroup::ALIGN_LEFT_INDENT:
		    spacing = _iconHeight * 0.5;
		    left = _iconHeight + _border + (spacing / 2.0);
		    break;
		default:
		    spacing = _iconHeight * 0.5;
		    left = spacing / 2.0;
		    break;
	    }

	    float boundsLeft = 0;

	    for(int i = 0; i < geoList.size(); ++i)
	    {
		osg::Matrix m;
		m.makeTranslate(osg::Vec3(left,0,0));
		

		float boundsRight;
		if(i == geoList.size()-1)
		{
		    boundsRight = width;
		}
		else
		{
		    boundsRight = left + geoList[i]->getWidth() + (spacing / 2.0);
		}

		if(colStatus[i])
		{
		    geoList[i]->resetIntersect(boundsRight-boundsLeft);
		    m.makeTranslate(osg::Vec3(boundsLeft,0,0));
		}
		else
		{
		    geoList[i]->getIntersect()->removeDrawables(0,geoList[i]->getIntersect()->getNumDrawables());
		    geoList[i]->getIntersect()->addDrawable(
			    makeQuad(boundsRight-boundsLeft,-(_height + _border),
				osg::Vec4(0,0,0,0),osg::Vec3(boundsLeft-left,0,_border / 2.0)));
		    m.makeTranslate(osg::Vec3(left,0,0));
		}

		geoList[i]->getNode()->setMatrix(m);
		_group->addChild(geoList[i]->getNode());

		boundsLeft = left + geoList[i]->getWidth() + (spacing / 2.0);

		left += geoList[i]->getWidth() + spacing;
	    }
	}
	else
	{
	    float twidth = 0.0;
	    float theight = 0.0;
	    // calc width/height
	    for(int i = 0; i < geoList.size(); ++i)
	    {
		if(geoList[i]->getHeight() > theight)
		{
		    theight = geoList[i]->getHeight();
		}
		twidth += geoList[i]->getWidth();
	    }
	    if(geoList.size())
	    {
		twidth += (geoList.size()) * _iconHeight * 0.5;
	    }
	    if(_mig->getAlignmentHint() == MenuItemGroup::ALIGN_LEFT_INDENT)
	    {
		twidth += _iconHeight + _border;
	    }
	    _width = twidth;
	    _height = theight;
	}
    }
    else if(_mig->getLayoutHint() == MenuItemGroup::COL_LAYOUT)
    {
	if(width > 0.0f)
	{
	    float top = 0;
	    for(int i = 0; i < geoList.size(); ++i)
	    {
		float left;

		geoList[i]->resetIntersect(width);
		switch(_mig->getAlignmentHint())
		{
		    case MenuItemGroup::ALIGN_LEFT_INDENT:
			left = _iconHeight + _border;
			break;
		    case MenuItemGroup::ALIGN_LEFT:
			left = 0.0;
			break;
		    case MenuItemGroup::ALIGN_CENTER:
		    {
			float space = width - geoList[i]->getWidth();
			space /= 2.0;
			left = space;
			break;
		    }
		    default:
			left = 0.0;
			break;
		}

		osg::Matrix m;

		if(colStatus[i])
		{
		    m.makeTranslate(osg::Vec3(0,0,top));
		}
		else
		{
		    geoList[i]->getIntersect()->removeDrawables(0,geoList[i]->getIntersect()->getNumDrawables());
		    geoList[i]->getIntersect()->addDrawable(
			    makeQuad(width,-(geoList[i]->getHeight() + _border),
				osg::Vec4(0,0,0,0),osg::Vec3(-left,0,_border / 2.0)));
		    m.makeTranslate(osg::Vec3(left,0,top));
		}

		geoList[i]->getNode()->setMatrix(m);
		_group->addChild(geoList[i]->getNode());
		

		top -= (geoList[i]->getHeight() + _border);
	    }
	}
	else
	{
	    float twidth = 0.0;
	    float theight = 0.0;
	    // calc width/height
	    for(int i = 0; i < geoList.size(); ++i)
	    {
		if(geoList[i]->getWidth() > twidth)
		{
		    twidth = geoList[i]->getWidth();
		}
		theight += geoList[i]->getHeight();
	    }
	    if(geoList.size())
	    {
		theight += (geoList.size()-1) * _border;
	    }
	    if(_mig->getAlignmentHint() == MenuItemGroup::ALIGN_LEFT_INDENT)
	    {
		twidth += _iconHeight + _border;
	    }
	    _width = twidth;
	    _height = theight;
	}
    }
}
