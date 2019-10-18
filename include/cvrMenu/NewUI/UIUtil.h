#ifndef CALVR_UI_UTIL_H
#define CALVR_UI_UTIL_H

#include <cvrMenu/Export.h>

#include <osg/Geometry>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Texture2D>
#include <osgText/Text>

#include <osgDB/ReadFile>

namespace cvr
{
	class CVRMENU_EXPORT UIUtil
	{
	public:
		static osg::Vec3 multiplyComponents(osg::Vec3 lhs, osg::Vec3 rhs);

		static osg::Geometry* makeQuad(float width, float height, osg::Vec4 color, osg::Vec3 pos);

		static osg::Geometry* makeLine(osg::Vec3 p1, osg::Vec3 p2, osg::Vec4 color);

		static osg::ref_ptr<osg::Texture2D> loadImage(std::string name);

		static osg::ref_ptr<osg::Node> loadModel(std::string name);

		static osgText::Text * makeText(std::string text, float size, osg::Vec3 pos,
			osg::Vec4 color, osgText::Text::AlignmentType align =
			osgText::Text::LEFT_CENTER, osgText::Font* font = NULL);

		static void setDefaultFont(osgText::Font* font) { _font = font; }


	protected:
		static std::map<std::string, osg::observer_ptr<osg::Texture2D> > _imageCache;
		static std::map<std::string, osg::observer_ptr<osg::Node> > _modelCache;
		static osg::ref_ptr<osgText::Font> _font;

	};
}

#endif
