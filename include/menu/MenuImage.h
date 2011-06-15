#ifndef CALVR_MENU_IMAGE_H
#define CALVR_MENU_IMAGE_H

#include <menu/MenuItem.h>

#include <osg/Texture2D>

#include <string>

namespace cvr
{

class MenuImage : public MenuItem
{
    public:
        MenuImage(std::string imageFile, float width = 0, float height = 0);
        MenuImage(osg::Texture2D * texture, float width, float height);

        virtual ~MenuImage();

        void setImage(std::string imageFile, float width = 0, float height = 0);
        void setImage(osg::Texture2D * texture, float width, float height);

        osg::Texture2D * getImage();

        void setWidth(float width);
        float getWidth();

        void setHeight(float height);
        float getHeight();

        virtual MenuItemType getType();
    protected:
        void loadImageFromFile(std::string & file);

        float _width;
        float _height;

        osg::ref_ptr<osg::Texture2D> _image;

};

}

#endif
