#ifndef CALVR_UI_EMPTY_ELEMENT_H
#define CALVR_UI_EMPTY_ELEMENT_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>

namespace cvr
{
	class CVRMENU_EXPORT UIEmptyElement : public UIElement
	{
	public:

		UIEmptyElement();
		virtual ~UIEmptyElement();

	protected:
		virtual void updateElement(osg::Vec3 pos, osg::Vec3 size);
	};
}

#endif