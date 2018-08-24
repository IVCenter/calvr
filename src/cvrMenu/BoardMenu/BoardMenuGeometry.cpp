#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuButtonGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuCheckboxGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuListGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuRangeValueGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuRangeValueCompactGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuTextGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuTextButtonSetGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuScrollTextGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuImageGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuBarGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuSubMenuGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuSubMenuClosableGeometry.h>
#include <cvrMenu/BoardMenu/BoardMenuItemGroupGeometry.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuList.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrMenu/SubMenu.h>
#include <cvrUtil/Bounds.h>

#include <osg/Version>
#include <osgText/Text>
#include <osg/Geometry>
#include <osgDB/ReadFile>

#include <iostream>
#include <cstdlib>

using namespace cvr;

osg::Vec4 BoardMenuGeometry::_textColor;
osg::Vec4 BoardMenuGeometry::_textColorSelected;
osg::Vec4 BoardMenuGeometry::_backgroundColor;
float BoardMenuGeometry::_border;
float BoardMenuGeometry::_iconHeight;
float BoardMenuGeometry::_textSize;
std::string BoardMenuGeometry::_iconDir;
osg::ref_ptr<osgText::Font> BoardMenuGeometry::_font;
std::map<std::string,osg::ref_ptr<osg::Texture2D> > BoardMenuGeometry::_iconCache;

BoardMenuGeometry * cvr::createGeometry(MenuItem * item, BoardMenu * menu, bool head)
{
    switch(item->getType())
    {
        case BUTTON:
        {
            BoardMenuGeometry * mg = new BoardMenuButtonGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case CHECKBOX:
        {
            BoardMenuGeometry * mg = new BoardMenuCheckboxGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case SUBMENU:
        {
            BoardMenuGeometry * mg = new BoardMenuSubMenuGeometry(head);
            mg->createGeometry(item);

            return mg;
        }
        case SUBMENU_CLOSABLE:
        {
            BoardMenuGeometry * mg = new BoardMenuSubMenuClosableGeometry(head);
            mg->createGeometry(item);

            return mg;
        }
	case ITEM_GROUP:
	{
	    BoardMenuGeometry * mg = new BoardMenuItemGroupGeometry(menu);
            mg->createGeometry(item);

	    return mg;
	}
        case RANGEVALUE:
        {
            BoardMenuGeometry * mg = new BoardMenuRangeValueGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case RANGEVALUECOMPACT:
        {
            BoardMenuGeometry * mg = new BoardMenuRangeValueCompactGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case TEXT:
        {
            BoardMenuGeometry * mg = new BoardMenuTextGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case TEXTBUTTONSET:
        {
            BoardMenuGeometry * mg = new BoardMenuTextButtonSetGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case TEXTSCROLL:
        {
            BoardMenuGeometry * mg = new BoardMenuScrollTextGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case IMAGE:
        {
            BoardMenuGeometry * mg = new BoardMenuImageGeometry();
            mg->createGeometry(item);

            return mg;
        }
	case BAR:
	{
	    BoardMenuGeometry * mg = new BoardMenuBarGeometry();
	    mg->createGeometry(item);

	    return mg;
	}
        case LIST:
        {
            BoardMenuGeometry * mg = new BoardMenuListGeometry();
            mg->createGeometry(item);

            return mg;
        }
        case OTHER:
        default:
            break;
    }

    return NULL;
}

osg::Geometry * BoardMenuGeometry::makeQuad(float width, float height,
        osg::Vec4 color, osg::Vec3 pos)
{
    osg::Geometry * geo = new osg::Geometry();
    geo->setUseDisplayList(false);
    geo->setUseVertexBufferObjects(true);

    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(pos);
	verts->push_back(pos + osg::Vec3(0, 0, height));
	verts->push_back(pos + osg::Vec3(width, 0, height));
    verts->push_back(pos + osg::Vec3(width,0,0));

    geo->setVertexArray(verts);

    geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0,0));
	texcoords->push_back(osg::Vec2(0, 1));
	texcoords->push_back(osg::Vec2(1, 1));
    texcoords->push_back(osg::Vec2(1,0));
    
    geo->setTexCoordArray(0,texcoords);

    return geo;
}

osg::Geometry * BoardMenuGeometry::makeLine(osg::Vec3 p1, osg::Vec3 p2,
        osg::Vec4 color)
{
    osg::Geometry * geo = new osg::Geometry();
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(p1);
    verts->push_back(p2);

    geo->setVertexArray(verts);

    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
            osg::PrimitiveSet::LINES,0);

    ele->push_back(0);
    ele->push_back(1);
    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_OVERALL);

    return geo;
}

osg::Texture2D * BoardMenuGeometry::loadIcon(std::string name)
{
    if(_iconCache.find(name) != _iconCache.end())
    {
        if(_iconCache[name])
        {
            return _iconCache[name].get();
        }
        else
        {
            return NULL;
        }
    }

    std::string file = _iconDir + name;
    //std::cerr << "Trying to load icon: " << file << std::endl;
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(file);
    if(image)
    {
        osg::Texture2D* texture;
        texture = new osg::Texture2D;
        texture->setImage(image);

        // do not cache very large textures
        if(image->s() <= 512 && image->t() <= 512)
        {
            _iconCache[name] = texture;
        }

        texture->setResizeNonPowerOfTwoHint(false);

        return texture;
    }
    else
    {
        _iconCache[name] = NULL;
    }
    std::cerr << "Icon: " << file << " not found." << std::endl;
    return NULL;
}

osgText::Text * BoardMenuGeometry::makeText(std::string text, float size,
        osg::Vec3 pos, osg::Vec4 color, osgText::Text::AlignmentType align)
{
    osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(size);
    textNode->setAlignment(align);
    textNode->setPosition(pos);
    textNode->setColor(color);
    textNode->setBackdropColor(osg::Vec4(0,0,0,0));
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    if(_font.valid())
    {
        textNode->setFont(_font);
    }
    textNode->setText(text);
    return textNode;
}

void BoardMenuGeometry::calibrateTextSize(float textSize)
{
    osgText::Text * textNode = new osgText::Text();
    textNode->setCharacterSize(textSize);
    textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
    if(_font.valid())
    {
        textNode->setFont(_font);
    }
    textNode->setText("A");

    osg::BoundingBox bb;
    bb = cvr::getBound(textNode);

    float height = bb.zMax() - bb.zMin();

    float ratio = 24.375 / height;

    _textSize = textSize * ratio;

    textNode->unref();
}

BoardMenuGeometry::BoardMenuGeometry()
{
    _width = 0;
    _height = 0;
    _intersect = NULL;
    _item = NULL;
}

BoardMenuGeometry::~BoardMenuGeometry()
{
}

MenuItem * BoardMenuGeometry::getMenuItem()
{
    return _item;
}

void BoardMenuGeometry::resetIntersect(float width)
{
    _intersect->removeDrawables(0,_intersect->getNumDrawables());
    _intersect->addDrawable(
            makeQuad(width + 2.0 * _border,-(_height + _border),
                    osg::Vec4(0,0,0,0),osg::Vec3(-_border,0,_border / 2.0)));
}

osg::Geode * BoardMenuGeometry::getIntersect()
{
    return _intersect;
}

float BoardMenuGeometry::getWidth()
{
    return _width;
}

float BoardMenuGeometry::getHeight()
{
    return _height;
}

osg::MatrixTransform * BoardMenuGeometry::getNode()
{
    return _node.get();
}
