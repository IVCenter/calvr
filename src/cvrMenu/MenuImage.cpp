#include <cvrMenu/MenuImage.h>

#include <osgDB/ReadFile>

using namespace cvr;

MenuImage::MenuImage(std::string imageFile, float width, float height) :
        MenuItem()
{
    _width = width;
    _height = height;
    loadImageFromFile(imageFile);
}

MenuImage::MenuImage(osg::Texture2D * texture, float width, float height) :
        MenuItem()
{
    _width = width;
    _height = height;
    _image = texture;
    _image->setResizeNonPowerOfTwoHint(false);
}

MenuImage::~MenuImage()
{
}

void MenuImage::setImage(std::string imageFile, float width, float height)
{
    _width = width;
    _height = height;
    loadImageFromFile(imageFile);
}

void MenuImage::setImage(osg::Texture2D * texture, float width, float height)
{
    _width = width;
    _height = height;
    _image = texture;
    _image->setResizeNonPowerOfTwoHint(false);
}

osg::Texture2D * MenuImage::getImage()
{
    if(_image.valid())
    {
        return _image.get();
    }

    return NULL;
}

void MenuImage::setWidth(float width)
{
    _width = width;
    setDirty(true);
}

float MenuImage::getWidth()
{
    return _width;
}

void MenuImage::setHeight(float height)
{
    _height = height;
    setDirty(true);
}

float MenuImage::getHeight()
{
    return _height;
}

MenuItemType MenuImage::getType()
{
    return IMAGE;
}

void MenuImage::loadImageFromFile(std::string & file)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(file);
    if(image.get())
    {
        _image = new osg::Texture2D;
        _image->setImage(image);
        _image->setResizeNonPowerOfTwoHint(false);

        if(_width == 0)
        {
            _width = image->s();
        }
        if(_height == 0)
        {
            _height = image->t();
        }
        return;
    }
    _image = NULL;
}
