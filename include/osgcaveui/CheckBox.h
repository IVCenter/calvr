#ifndef _CUI_CHECK_BOX_H_
#define _CUI_CHECK_BOX_H_

#include "Widget.h"
#include "Button.h"

namespace cui
{
class Interaction;

/** This is the implementation of a push button, which triggers an action.
*/
class CUIEXPORT CheckBox : public Button
{
  public:
    CheckBox(Interaction*);
    virtual ~CheckBox(){};
    virtual void setChecked(bool, bool=false);
    virtual bool isChecked();
    virtual void setImages(std::string, std::string);

  protected:
    bool _isChecked;   ///< true = checked
    osg::Image* _imageChecked;
    osg::Image* _imageUnchecked;
    
    virtual void updateIcon();    
    virtual void buttonEvent(InputDevice*, int);
};

}

#endif
