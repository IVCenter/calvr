#ifndef CALVR_UI_TEXT_H
#define CALVR_UI_TEXT_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>
#include <cvrMenu/NewUI/UIUtil.h>

#include <osgText/Text>

namespace cvr
{
	class CVRMENU_EXPORT UIText : public UIElement
	{
	public:

		UIText(std::string text, float size = 20, osgText::Text::AlignmentType align = osgText::Text::LEFT_TOP, osgText::Font* font = NULL)
			: UIElement()
		{
			_text = UIUtil::makeText(text, size, osg::Vec3(0, 0, 0), osg::Vec4(1, 1, 1, 1), align, font);
			_string = text;
		}

		virtual void createGeometry() override;
		virtual void updateGeometry() override;

		virtual osgText::Text::AlignmentType getTextAlign();
		virtual void setTextAlign(osgText::Text::AlignmentType);

		virtual osgText::Font* getFont();
		virtual void setFont(osgText::Font*);

		virtual const std::string& getText();
		virtual void setText(const std::string& text);

		osgText::Text* getTextObject() { return _text; }

		virtual void setColor(osg::Vec4 color = osg::Vec4(1, 1, 1, 1));
		osg::ref_ptr<osg::Geode> _geode;

	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		
		osg::ref_ptr<osgText::Text> _text;
		std::string _string;

	};
}

#endif