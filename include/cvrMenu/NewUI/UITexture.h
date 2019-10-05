#ifndef CALVR_UI_TEXTURE_H
#define CALVR_UI_TEXTURE_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIQuadElement.h>
#include <cvrMenu/NewUI/UIUtil.h>

#include <osg/Texture2D>

namespace cvr
{
	class CVRMENU_EXPORT UITexture : public UIQuadElement
	{
	public:

		UITexture(osg::Texture2D* texture)
			: UIQuadElement()
		{
			_texture = texture;
		}

		UITexture(std::string texturePath)
			: UIQuadElement()
		{
			_texture = UIUtil::loadImage(texturePath);
		}

		UITexture(osg::Vec4 color)
			: UIQuadElement(color)
		{

		}

		virtual void updateGeometry();

		virtual void setTexture(osg::Texture2D* texture);
		virtual void setTexture(std::string texturePath);


	protected:
		osg::ref_ptr<osg::MatrixTransform> _transform;
		osg::ref_ptr<osg::Geode> _geode;

		osg::ref_ptr<osg::Texture2D> _texture;
	};
}

#endif