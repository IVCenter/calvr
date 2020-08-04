#ifndef CALVR_UI_DIAL_H
#define CALVR_UI_DIAL_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>

namespace cvr
{
	class CVRMENU_EXPORT UIDial : public UIElement
	{
	public:

		virtual void createGeometry();
		virtual void updateGeometry();

	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		osg::ref_ptr<osg::Geode> _geode;

	};
}

#endif