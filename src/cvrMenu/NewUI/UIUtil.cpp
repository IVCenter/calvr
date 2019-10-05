#include "cvrMenu/NewUI/UIUtil.h"
#include <osgDB/readFile>
#include <iostream>

using namespace cvr;

osg::ref_ptr<osgText::Font> UIUtil::_font;
std::map<std::string, osg::observer_ptr<osg::Texture2D> > UIUtil::_imageCache;
std::map<std::string, osg::observer_ptr<osg::Node> > UIUtil::_modelCache;

osg::Vec3 UIUtil::multiplyComponents(osg::Vec3 lhs, osg::Vec3 rhs)
{
	return osg::Vec3(lhs.x() * rhs.x(), lhs.y() * rhs.y(), lhs.z() * rhs.z());
}

osg::Geometry * UIUtil::makeQuad(float width, float height, osg::Vec4 color, osg::Vec3 pos)
{
	osg::Geometry * geo = new osg::Geometry();
	//geo->setUseDisplayList(false);
	geo->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->push_back(pos + osg::Vec3(0, 0, 0));
	verts->push_back(pos + osg::Vec3(0, 0, -height));
	verts->push_back(pos + osg::Vec3(width, 0, -height));
	verts->push_back(pos + osg::Vec3(width, 0, 0));

	geo->setVertexArray(verts);

	geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(color);
	geo->setColorArray(colors);
	geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec2Array* texcoords = new osg::Vec2Array;
	texcoords->push_back(osg::Vec2(0, 0));
	texcoords->push_back(osg::Vec2(0, 1));
	texcoords->push_back(osg::Vec2(1, 1));
	texcoords->push_back(osg::Vec2(1, 0));

	geo->setTexCoordArray(0, texcoords);

	return geo;
}

osg::Geometry * UIUtil::makeLine(osg::Vec3 p1, osg::Vec3 p2, osg::Vec4 color)
{
	osg::Geometry * geo = new osg::Geometry();
	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->push_back(p1);
	verts->push_back(p2);

	geo->setVertexArray(verts);

	osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
		osg::PrimitiveSet::LINES, 0);

	ele->push_back(0);
	ele->push_back(1);
	geo->addPrimitiveSet(ele);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(color);
	geo->setColorArray(colors);
	geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	return geo;
}

osg::ref_ptr<osg::Texture2D> UIUtil::loadImage(std::string path)
{
	if (_imageCache.find(path) != _imageCache.end())
	{
		osg::ref_ptr<osg::Texture2D> img;
		_imageCache[path].lock(img);
		if (img.valid())
		{
			return img.get();
		}
		else
		{
			//return NULL;
		}
	}

	//std::cerr << "Trying to load icon: " << file << std::endl;
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(path);
	if (image)
	{
		osg::Texture2D* texture;
		texture = new osg::Texture2D;
		texture->setImage(image);

		//Cache texture in observer_ptr so that as long as it is being used it will be shared
		_imageCache[path] = texture;

		texture->setResizeNonPowerOfTwoHint(false);

		return texture;
	}
	std::cerr << "Could not load image: " << path << std::endl;
	return NULL;
}

osg::ref_ptr<osg::Node> UIUtil::loadModel(std::string path)
{
	if (_modelCache.find(path) != _modelCache.end())
	{
		osg::ref_ptr<osg::Node> model;
		_modelCache[path].lock(model);
		if (model.valid())
		{
			return model.get();
		}
		else
		{
			//return NULL;
		}
	}

	osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(path);
	if (model)
	{
		//Cache model in observer_ptr so that as long as it is being used it will be shared
		_modelCache[path] = model;


		return model;
	}
	std::cerr << "Could not load model: " << path << std::endl;
	return NULL;
}


osgText::Text * UIUtil::makeText(std::string text, float size,
	osg::Vec3 pos, osg::Vec4 color, osgText::Text::AlignmentType align, osgText::Font* font)
{
	osgText::Text * textNode = new osgText::Text();

	textNode->setCharacterSize(size);
	textNode->setAlignment(align);
	textNode->setPosition(pos);
	textNode->setColor(color);
	//textNode->setBackdropColor(osg::Vec4(0,0,0,0));
	//textNode->setBackdropType(osgText::Text::BackdropType::NONE);
	textNode->setAxisAlignment(osgText::Text::XZ_PLANE);
	if (font)
	{
		textNode->setFont(font);
	}
	else if (_font.valid())
	{
		textNode->setFont(_font);
	}
	textNode->setText(text);
	//textNode->setEnableDepthWrites(false);
	return textNode;
}