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
			: UIQuadElement(osg::Vec4(1,1,1,1))
		{
			_texture = texture;
		}

		UITexture(std::string texturePath)
			: UIQuadElement(osg::Vec4(1, 1, 1, 1))
		{
			_texture = UIUtil::loadImage(texturePath);
		}

		UITexture(osg::Vec4 color = osg::Vec4(1, 1, 1, 1))
			: UIQuadElement(color)
		{

		}

		virtual void updateGeometry() override;

		virtual void setTexture(osg::Texture2D* texture);
		virtual void setTexture(std::string texturePath);


	protected:
		osg::ref_ptr<osg::Texture2D> _texture;
	};
}

#endif