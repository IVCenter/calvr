#ifndef CALVR_INTERACTION_EVENT_H
#define CALVR_INTERACTION_EVENT_H

#include <osg/Matrix>

#include <iostream>

namespace cvr
{

enum Interaction
{
    NO_INTERACTION = 0,
    BUTTON_DOWN = 0x10000000,
    BUTTON_UP = 0x10000001,
    BUTTON_DRAG = 0x10000002,
    BUTTON_DOUBLE_CLICK = 0x10000003,
    KEY_UP = 0x04000000,
    KEY_DOWN = 0x04000001
};

enum InteractionEventType
{
    TRACKED_BUTTON_INTER_EVENT = 0,
    MOUSE_INTER_EVENT,
    KEYBOARD_INTER_EVENT,
    INTER_EVENT,
    NUM_INTER_EVENT_TYPES // must be last item
};

class TrackedButtonInteractionEvent;
class MouseInteractionEvent;
class KeyboardInteractionEvent;

class InteractionEvent
{
    public:
        InteractionEvent() : _interaction(NO_INTERACTION) {}

        Interaction getInteraction() { return _interaction; }
        void setInteraction(Interaction interaction) { _interaction = interaction; }

        virtual InteractionEventType getEventType() { return INTER_EVENT; }

        virtual TrackedButtonInteractionEvent * asTrackedButtonEvent() { return NULL; }
        virtual MouseInteractionEvent * asMouseEvent() { return NULL; }
        virtual KeyboardInteractionEvent * asKeyboardEvent() { return NULL; }

    protected:
        Interaction _interaction;
};

class TrackedButtonInteractionEvent : public InteractionEvent
{
    public:
        TrackedButtonInteractionEvent() : InteractionEvent(),_hand(0),_button(0) {}

        int getHand() { return _hand; }
        void setHand(int hand) { _hand = hand; }
        int getButton() { return _button; }
        void setButton(int button) { _button = button; }
        osg::Matrix & getTransform() { return _transform; }
        void setTransform(osg::Matrix transform) { _transform = transform; }

        virtual InteractionEventType getEventType() { return TRACKED_BUTTON_INTER_EVENT; }

        virtual TrackedButtonInteractionEvent * asTrackedButtonEvent() { return this; }

    protected:
        int _hand;
        int _button;
        osg::Matrix _transform;
};

class MouseInteractionEvent : public TrackedButtonInteractionEvent
{
    public:
        MouseInteractionEvent() : TrackedButtonInteractionEvent(),_x(0),_y(0),_masterScreenNum(0) {}

        int getX() { return _x; }
        void setX(int x) { _x = x; }
        int getY() { return _y; }
        void setY(int y) { _y = y; }
        int getMasterScreenNum() { return _masterScreenNum; }
        void setMasterScreenNum(int screen) { _masterScreenNum = screen; }

        virtual InteractionEventType getEventType() { return MOUSE_INTER_EVENT; }

        virtual MouseInteractionEvent * asMouseEvent() { return this; }

    protected:
        int _x;
        int _y;
        int _masterScreenNum;
};

class KeyboardInteractionEvent : public InteractionEvent
{
    public:
        KeyboardInteractionEvent() : InteractionEvent(),_key(0),_mod(0) {}

        int getKey() { return _key; }
        void setKey(int key) { _key = key; }
        int getMod() { return _mod; }
        void setMod(int mod) { _mod = mod; }

        virtual InteractionEventType getEventType() { return KEYBOARD_INTER_EVENT; }

        virtual KeyboardInteractionEvent * asKeyboardEvent() { return this; }

    protected:
        int _key;
        int _mod;
};

InteractionEvent * loadEventWithType(InteractionEvent * event, InteractionEventType type);
void storeEvent(InteractionEvent * event, void * des);
int getEventSize(InteractionEventType type);

}

#endif
