#include "cvrMenu/NewUI/UICheckbox.h"

using namespace cvr;

UICheckbox::UICheckbox(std::string offTexture, std::string onTexture)
	: UISwitch()
{
	offElement = new UITexture(offTexture);
	addChild(offElement);

	onElement = new UITexture(onTexture);
	addChild(onElement);
}

UICheckbox::UICheckbox(osg::Vec4 off, osg::Vec4 on)
	: UISwitch()
{
	offElement = new UITexture(off);
	addChild(offElement);

	onElement = new UITexture(on);
	addChild(onElement);
}