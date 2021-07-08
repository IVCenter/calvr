#ifndef CALVR_UI_RADIAL_H
#define CALVR_UI_RADIAL_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIButton.h>

namespace cvr
{
	class UIRadialButton;
	class UIRadial;


	//Extension of generic button class that alerts radial parent when the button is pressed
	class CVRMENU_EXPORT UIRadialButton : public UIButton
	{
	public:
		UIRadialButton(UIRadial* parent, int button = 0);
		UIRadialButton(int button = 0);

		virtual bool onButtonPress(bool pressed);

		UIRadial* getRadial();

		void setRadial(UIRadial* r);

	protected:
		std::shared_ptr<UIRadial> _parent;
	};

	//Generic radial class. Add buttons with 'addButton' and override 'onSelectionChange' for functionality
	class CVRMENU_EXPORT UIRadial
	{
	public:
		UIRadial();
		virtual ~UIRadial() {}

		virtual void addButton(UIRadialButton* b);
		virtual void removeButton(UIRadialButton* b);

		virtual bool onButtonPress(UIRadialButton* b);

		virtual void onSelectionChange() {};
		virtual void setCurrent(int current);
		virtual int getCurrent() { return _current; }
		virtual UIRadialButton* getCurrentButton()
		{
			if (_current >= _buttons.size() || _current < 0)
			{
				return nullptr;
			}
			return _buttons[_current];
		}
		virtual void allowNoneSelected(bool ns) { _allowNoneSelected = ns; }

	protected:
		int _current;
		bool _allowNoneSelected;
		std::vector<UIRadialButton*> _buttons;
	};
}

#endif