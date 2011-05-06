#ifndef _CUI_MESSAGE_H_
#define _CUI_MESSAGE_H_

// Virvo:
//#include <virvo/vvstopwatch.h>
#include "VirvoDeps.h"

// OpenSceneGraph:
#include <osg/Vec3>
#include <osg/Geode>
#include <osgText/Text>

#include "Widget.h"

namespace cui
{

/** This class displays a text message on screen, for a limited time.
*/
class CUIEXPORT Message
{
  public:
    enum AlignmentType 
    {
      LEFT, CENTER, RIGHT
    };
    Message(AlignmentType);
    virtual ~Message();
    virtual void update();
    virtual void setText(std::string&, float = -1.0f);
    virtual void setText(const char*, float = -1.0f);
    virtual void setPosition(osg::Vec3&);
    virtual osg::Node* getNode();
    virtual void setSize(float);
  virtual void setColor(osg::Vec4&);

  protected:
    osgText::Text* _messageText;
    osg::Geode* _geode;
    float _timeLeft;   // time left for message display; -1 = always displayed
    vvStopwatch* _watch;
};

}

#endif
