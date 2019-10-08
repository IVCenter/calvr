#ifndef CALVR_UI_QUAD_ELEMENT_H
#define CALVR_UI_QUAD_ELEMENT_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>

namespace cvr
{
	class CVRMENU_EXPORT UIQuadElement : public UIElement
	{
	public:
		UIQuadElement(osg::Vec4 color = osg::Vec4(1, 1, 1, 1))
			: UIElement()
		{
			_color = color;
			_geode = new osg::Geode();
		}

		virtual void createGeometry();
		virtual void updateGeometry();

		virtual void setColor(osg::Vec4 color);

		virtual void setTransparent(bool transparent);

	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		osg::ref_ptr<osg::Geode> _geode;

		osg::Vec4 _color;
	};
}

#endif