#ifndef CALVR_UI_BUTTON_H
#define CALVR_UI_BUTTON_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>

namespace cvr
{
	//Generic button class. Override 'onButtonPress' for simple functionality
	class CVRMENU_EXPORT UIButton : public UIElement
	{
	public:

		UIButton(unsigned int button = 0)
			: UIElement()
		{
			_button = button;
			_buttonDown = false;
		}

		virtual void createGeometry();
		virtual void updateGeometry();

		virtual bool processEvent(InteractionEvent * event);

		//Called when the button is pressed using _button, or the button is released
		virtual bool onButtonPress(bool pressed) { return true; }

		//Called when pointer enters and exits the intersection geode of this element
		virtual void processHover(bool enter);

	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		osg::ref_ptr<osg::Geode> _geode;

		unsigned int _button;
		bool _buttonDown;

	};
}

#endif