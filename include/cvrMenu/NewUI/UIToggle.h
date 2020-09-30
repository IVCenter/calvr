#ifndef CALVR_UI_TOGGLE_H
#define CALVR_UI_TOGGLE_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIButton.h>

namespace cvr
{
	//Generic toggle class. Override 'onToggle' for simple functionality
	class CVRMENU_EXPORT UIToggle : public UIButton
	{
	public:

		UIToggle(unsigned int button = 0)
			: UIButton(button)
		{
			_on = false;
		}

		//Called when the button is pressed using _button, or the button is released
		virtual bool onButtonPress(bool pressed);

		virtual bool onToggle() { return true; }

		virtual bool isOn() { return _on; }
		virtual bool toggle();

		virtual void turnOn() { _on = true; }
		virtual void turnOff() { _on = false; }

	protected:
		bool _on;
	};
}

#endif