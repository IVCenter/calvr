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
			createGeometry();
			_absoluteRounding = new osg::Uniform("absoluteRounding", 0.0f);
			_percentRounding = new osg::Uniform("percentRounding", 0.0f);
			_borderColorUniform = new osg::Uniform("borderColor", color);
			_borderSizeUniform = new osg::Uniform("borderSize", 0.0f);
			_borderOnlyUniform = new osg::Uniform("borderOnly", false);
			_borderColorUniform = new osg::Uniform("borderColor", osg::Vec4(0.0f,0.0f,0.0f,1.0f));
			(_geode->getDrawable(0))->getOrCreateStateSet()->addUniform(_absoluteRounding);
			(_geode->getDrawable(0))->getOrCreateStateSet()->addUniform(_percentRounding);
			(_geode->getDrawable(0))->getOrCreateStateSet()->addUniform(_borderSizeUniform);
			(_geode->getDrawable(0))->getOrCreateStateSet()->addUniform(_borderOnlyUniform);
			(_geode->getDrawable(0))->getOrCreateStateSet()->addUniform(_borderColorUniform);
		}

		virtual void createGeometry();
		virtual void updateGeometry();

		virtual void setColor(osg::Vec4 color);

		virtual void setTransparent(bool transparent);

		virtual void setRounding(float absRounding, float percentRounding);
		virtual void setBorderSize(float size);
		
		void borderOnly(bool borderOnly);

		void setBorderColor(osg::Vec4 borderColor);

		void setRot(osg::Quat rot) { _rotQuat = rot; }

		osg::ref_ptr<osg::MatrixTransform> getTransform() { return _transform; }

		
		osg::ref_ptr<osg::Geode> _geode;
	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		
		osg::Quat rotQuat;
		osg::Vec4 _color;
		osg::Vec4 _borderColor;
		osg::Uniform* _absoluteRounding;
		osg::Uniform* _percentRounding;
		osg::Uniform* _borderColorUniform;
		osg::Uniform* _borderSizeUniform;
		osg::Uniform* _borderOnlyUniform;
		 
	};
}

#endif