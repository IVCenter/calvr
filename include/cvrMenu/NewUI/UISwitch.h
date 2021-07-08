#ifndef CALVR_UI_SWITCH_H
#define CALVR_UI_SWITCH_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIEmptyElement.h>

namespace cvr
{
	//Generic toggle class. Override 'onToggle' for simple functionality
	class CVRMENU_EXPORT UISwitch : public UIEmptyElement
	{
	public:

		UISwitch(int index = -1)
			: UIEmptyElement()
		{
			_index = index;
		}

		virtual void updateElement(osg::Vec3 pos, osg::Vec3 size);

		virtual void setDisplayed(int index);
		virtual int getDisplayed();

		virtual void addChild(UIElement* e);

	protected:
		int _index;
	};
}

#endif