#ifndef _CUI_RADIO_GROUP_H_
#define _CUI_RADIO_GROUP_H_

#include "RadioButton.h"

namespace cui
{
  class RadioGroupListener;

  class CUIEXPORT RadioGroup : public CardListener
  {
  public:
    RadioGroup();
    virtual ~RadioGroup();
    
    void add(RadioButton*);
    void remove(RadioButton*);
    void setSelected(RadioButton*);
    RadioButton* getSelected();
    bool isSelected(RadioButton*);
    void addRadioGroupListener(RadioGroupListener*);
    bool cardButtonEvent(Card*, int, int);
    bool cardCursorUpdate(Card*, InputDevice*);
    void setVisible(bool);

  private:
    std::vector<RadioButton*> _buttons;
    std::list<RadioGroupListener*> _listeners;
    RadioButton* _active;
  };

  class CUIEXPORT RadioGroupListener
  {
  public:
    virtual ~RadioGroupListener() {}
    virtual bool radioGroupStatusChanged(RadioGroup*) = 0;
  };
}

#endif
