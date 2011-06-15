/**
 * @file MenuImage.h
 */
#ifndef CALVR_MENU_IMAGE_H
#define CALVR_MENU_IMAGE_H

#include <menu/MenuItem.h>

#include <osg/Texture2D>

#include <string>

namespace cvr
{

/**
 * @brief Allows you to add a texture image to a menu
 */
class MenuImage : public MenuItem
{
    public:
        /**
         * @brief constructor
         * @param imageFile path to an image file to try to load using osgDB::readImageFile()
         * @param width width of the final image geometry
         * @param height height of the final image geometry
         *
         * If width or height are 0, they will be set to the width and height of the 
         * image in pixels
         */
        MenuImage(std::string imageFile, float width = 0, float height = 0);

        /**
         * @brief constructor
         * @param texture image to put in a menu
         * @param width width of final image geometry
         * @param height height of final image geometry
         */
        MenuImage(osg::Texture2D * texture, float width, float height);

        virtual ~MenuImage();

        /**
         * @brief Set the image for the menu item by file
         * @param imageFile path to an image file to try to load using osgDB::readImageFile()
         * @param width width of the final image geometry
         * @param height height of the final image geometry
         *
         * If width or height are 0, they will be set to the width and height of the 
         * image in pixels
         */
        void setImage(std::string imageFile, float width = 0, float height = 0);

        /**
         * @brief Set the image for the menu item by texture
         * @param texture image to put in a menu
         * @param width width of final image geometry
         * @param height height of final image geometry
         */
        void setImage(osg::Texture2D * texture, float width, float height);

        /**
         * @brief Get a pointer to the texture form of this image
         * @return NULL is returned if the image is not valid
         */
        osg::Texture2D * getImage();

        /**
         * @brief Set the width for the final image geometry
         */
        void setWidth(float width);

        /**
         * @brief Get the image geometry width
         */
        float getWidth();

        /**
         * @brief Set the height for the final image geometry
         */
        void setHeight(float height);

        /**
         * @brief Get the image geometry height
         */
        float getHeight();

        /**
         * @brief Returns IMAGE type for this item
         */
        virtual MenuItemType getType();
    protected:
        /**
         * @brief Attempt to load an image texture from a file
         */
        void loadImageFromFile(std::string & file);

        float _width; ///< width for image geometry
        float _height; ///< height for image geometry

        osg::ref_ptr<osg::Texture2D> _image; ///< image texture

};

}

#endif
