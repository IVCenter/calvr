#ifndef CALVR_UI_CHECKBOX_H
#define CALVR_UI_CHECKBOX_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UISwitch.h>
#include <cvrMenu/NewUI/UITexture.h>

namespace cvr
{
	//Generic toggle class. Override 'onToggle' for simple functionality
	class CVRMENU_EXPORT UICheckbox : public UISwitch
	{
	public:

		UICheckbox(std::string offTexture, std::string onTexture);

		UICheckbox(osg::Vec4 off = osg::Vec4(1, 0, 0, 1), osg::Vec4 on = osg::Vec4(0, 1, 0, 1));

		virtual ~UICheckbox() {};


		UITexture* offElement;
		UITexture* onElement;

	protected:
	};
}

#endif