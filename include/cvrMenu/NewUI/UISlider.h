#ifndef CALVR_UI_SLIDER_H
#define CALVR_UI_SLIDER_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>
#include <cvrMenu/NewUI/UITexture.h>

namespace cvr
{
	class CVRMENU_EXPORT UISlider : public UIElement
	{
	public:

		UISlider(std::string emptytexture, std::string filledtexture, std::string handletexture)
			: UIElement()
		{
			empty = new UITexture(emptytexture);
			filled = new UITexture(filledtexture);
			handle = new UITexture(handletexture);
			handle->setAbsolutePos(handle->getAbsolutePos() + osg::Vec3(0, -0.1f, 0));
			addChild(empty);
			addChild(filled);
			addChild(handle);
			_button = 0;
			_percent = 0.0f;
		}

		UISlider(osg::Vec4 emptyColor = osg::Vec4(0,0,0,1), osg::Vec4 filledColor = osg::Vec4(1,1,1,1),
			osg::Vec4 handleColor = osg::Vec4(0.5, 0.5, 0.5, 1))
			: UIElement()
		{
			empty = new UITexture(emptyColor);
			filled = new UITexture(filledColor);
			handle = new UITexture(handleColor);
			handle->setAbsolutePos(handle->getAbsolutePos() + osg::Vec3(0, -0.1f, 0));
			addChild(empty);
			addChild(filled);
			addChild(handle);
			_button = 0;
			_percent = 0.0f;
		}


		virtual void updateElement(osg::Vec3 pos, osg::Vec3 size);

		virtual void createGeometry();
		virtual void updateGeometry();

		virtual bool processEvent(InteractionEvent * event);

		//Called when the button is pressed using _button, or the button is released
		virtual bool onPercentChange() { return true; }

		//Called when pointer enters and exits the intersection geode of this element
		virtual void processHover(bool enter);

		UITexture* empty;
		UITexture* filled;
		UITexture* handle;

	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		osg::ref_ptr<osg::Geode> _geode;

		float _percent;
		unsigned int _button;
	};
}

#endif