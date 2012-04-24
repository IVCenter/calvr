#include <cvrUtil/TextureVisitors.h>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Texture>

#include <iostream>

using namespace cvr;

TextureResizeNonPowerOfTwoHintVisitor::TextureResizeNonPowerOfTwoHintVisitor(bool hint) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    _hint = hint;
}

TextureResizeNonPowerOfTwoHintVisitor::~TextureResizeNonPowerOfTwoHintVisitor()
{
}

void TextureResizeNonPowerOfTwoHintVisitor::apply(osg::Node & node)
{
    osg::StateSet * stateset = node.getOrCreateStateSet();

    if(stateset)
    {
	setHint(stateset);
    }
    traverse(node);
}

void TextureResizeNonPowerOfTwoHintVisitor::apply(osg::Geode & node)
{
    osg::StateSet * stateset = node.getOrCreateStateSet();

    if(stateset)
    {
	setHint(stateset);
    }
    
    for(int i = 0; i < node.getNumDrawables(); i++)
    {
	stateset = node.getDrawable(i)->getStateSet();
	if(stateset)
	{
	    setHint(stateset);
	}
    }
}

void TextureResizeNonPowerOfTwoHintVisitor::setHint(osg::StateSet * stateset)
{
    //TODO: get max texture units from somewhere
    for(int i = 0; i < 32; i++)
    {
	osg::StateAttribute * stateatt = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
	if(stateatt)
	{
	    osg::Texture * texture = stateatt->asTexture();
	    if(texture)
	    {
		texture->setResizeNonPowerOfTwoHint(_hint);
	    }
	}
    }
}
