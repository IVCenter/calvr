/**
 * @file InteractionEvent.h
 */
#ifndef CALVR_INTERACTION_EVENT_H
#define CALVR_INTERACTION_EVENT_H

#include <osg/Matrix>
#include <osg/Vec3>

#include <iostream>

namespace cvr
{

/**
 * @brief Possible calvr interactions
 */
enum Interaction
{
    NO_INTERACTION = 0,
    BUTTON_DOWN = 0x10000000,
    BUTTON_UP = 0x10000001,
    BUTTON_DRAG = 0x10000002,
    BUTTON_DOUBLE_CLICK = 0x10000003,
    VALUATOR = 0x20000000,
    KEY_UP = 0x04000000,
    KEY_DOWN = 0x04000001,
    MOVE = 0x08000000
};

/**
 * @brief Interaction event class type
 *
 * There is a unique value for each interaction class
 */
enum InteractionEventType
{
    TRACKED_BUTTON_INTER_EVENT = 0,
    MOUSE_INTER_EVENT,
    VALUATOR_INTER_EVENT,
    KEYBOARD_INTER_EVENT,
    POSITION_INTER_EVENT,
    HAND_INTER_EVENT,
    INTER_EVENT,
    NUM_INTER_EVENT_TYPES
// must be last item
};

class TrackedButtonInteractionEvent;
class MouseInteractionEvent;
class ValuatorInteractionEvent;
class KeyboardInteractionEvent;
class PositionInteractionEvent;
class HandInteractionEvent;

//TODO: add timestamps

/**
 * @brief CalVR base class representing an interaction
 */
class InteractionEvent
{
    public:
        InteractionEvent() :
                _interaction(NO_INTERACTION)
        {
        }

        /**
         * @brief Get the interaction value for this event
         */
        Interaction getInteraction()
        {
            return _interaction;
        }

        /**
         * @brief Set the interaction value for this event
         */
        void setInteraction(Interaction interaction)
        {
            _interaction = interaction;
        }

        /**
         * @brief Get the interaction class type
         */
        virtual InteractionEventType getEventType()
        {
            return INTER_EVENT;
        }

        /**
         * @brief Get a pointer to this class as a TrackedButtonInteractionEvent pointer
         * @return NULL is returned if this class can not be cast to a TrackedButtonInteractionEvent
         */
        virtual TrackedButtonInteractionEvent * asTrackedButtonEvent()
        {
            return NULL;
        }

        /**
         * @brief Get a pointer to this class as a MouseInteractionEvent pointer
         * @return NULL is returned if this class can not be cast to a MouseInteractionEvent
         */
        virtual MouseInteractionEvent * asMouseEvent()
        {
            return NULL;
        }

        virtual ValuatorInteractionEvent * asValuatorEvent()
        {
            return NULL;
        }

        /**
         * @brief Get a pointer to this class as a KeyboardInteractionEvent pointer
         * @return NULL is returned if this class can not be cast to a KeyboardInteractionEvent
         */
        virtual KeyboardInteractionEvent * asKeyboardEvent()
        {
            return NULL;
        }

        virtual PositionInteractionEvent * asPositionEvent()
        {
            return NULL;
        }

        virtual HandInteractionEvent * asHandEvent()
        {
            return NULL;
        }

    protected:
        Interaction _interaction; ///< event interaction value
};

class HandInteractionEvent : public InteractionEvent
{
    public:
        HandInteractionEvent() : InteractionEvent(), _hand(0)
        {
        }

        /**
         * @brief Get hand number for this event
         */
        int getHand()
        {
            return _hand;
        }

        /**
         * @brief Set hand number for this event
         */
        void setHand(int hand)
        {
            _hand = hand;
        }

        virtual InteractionEventType getEventType()
        {
            return HAND_INTER_EVENT;
        }

        virtual HandInteractionEvent * asHandEvent()
        {
            return this;
        }

    protected:
        int _hand;
};

/**
 * @brief Interaction event representing a button event for a tracked hand device
 */
class TrackedButtonInteractionEvent : public HandInteractionEvent
{
    public:
        TrackedButtonInteractionEvent() :
                HandInteractionEvent(), _button(0)
        {
        }

        /**
         * @brief Get button number for this event
         */
        int getButton()
        {
            return _button;
        }

        /**
         * @brief Set button number for this event
         */
        void setButton(int button)
        {
            _button = button;
        }

        /**
         * @brief Get pointer orientation matrix
         */
        osg::Matrix & getTransform()
        {
            return _transform;
        }

        /**
         * @brief Set pointer orientation matrix
         */
        void setTransform(osg::Matrix transform)
        {
            _transform = transform;
        }

        virtual InteractionEventType getEventType()
        {
            return TRACKED_BUTTON_INTER_EVENT;
        }

        virtual TrackedButtonInteractionEvent * asTrackedButtonEvent()
        {
            return this;
        }

    protected:
        int _button; ///< event button
        osg::Matrix _transform; ///< event orientation
};

/**
 * @brief Interaction event for mouse interaction
 */
class MouseInteractionEvent : public TrackedButtonInteractionEvent
{
    public:
        MouseInteractionEvent() :
                TrackedButtonInteractionEvent(), _x(0), _y(0), _masterScreenNum(
                        0)
        {
        }

        /**
         * @brief Get the viewport X value for this event
         *
         * Note: Viewport origin is bottom left
         */
        int getX()
        {
            return _x;
        }

        /**
         * @brief Set the viewport X value for this event
         *
         * Note: Viewport origin is bottom left
         */
        void setX(int x)
        {
            _x = x;
        }

        /**
         * @brief Get the viewport Y value for this event
         *
         * Note: Viewport origin is bottom left
         */
        int getY()
        {
            return _y;
        }

        /**
         * @brief Set the viewport Y value for this event
         *
         * Note: Viewport origin is bottom left
         */
        void setY(int y)
        {
            _y = y;
        }

        /**
         * @brief Get the screen number on the headnode for this event
         *
         * This number can be used with the ScreenConfig getMasterScreenInfo function
         */
        int getMasterScreenNum()
        {
            return _masterScreenNum;
        }

        /**
         * @brief Set the screen number on the headnode for this event
         *
         * This number can be used with the ScreenConfig getMasterScreenInfo function
         */
        void setMasterScreenNum(int screen)
        {
            _masterScreenNum = screen;
        }

        virtual InteractionEventType getEventType()
        {
            return MOUSE_INTER_EVENT;
        }

        virtual MouseInteractionEvent * asMouseEvent()
        {
            return this;
        }

    protected:
        int _x; ///< viewport x coord
        int _y; ///< viewport y coord
        int _masterScreenNum; ///< screen on the master when this event was generated
};

/**
 * @brief Interaction event for a valuator
 */
class ValuatorInteractionEvent : public HandInteractionEvent
{
    public:
        ValuatorInteractionEvent() : HandInteractionEvent(),
                _value(0.0), _valuator(0)
        {
        }

        int getValuator()
        {
            return _valuator;
        }

        void setValuator(int valuator)
        {
            _valuator = valuator;
        }

        float getValue()
        {
            return _value;
        }

        void setValue(float value)
        {
            _value = value;
        }

        virtual InteractionEventType getEventType()
        {
            return VALUATOR_INTER_EVENT;
        }

        virtual ValuatorInteractionEvent * asValuatorEvent()
        {
            return this;
        }

    protected:
        int _valuator;
        float _value;
};

/**
 * @brief Interaction event for a key press
 */
class KeyboardInteractionEvent : public InteractionEvent
{
    public:
        KeyboardInteractionEvent() :
                InteractionEvent(), _key(0), _mod(0)
        {
        }

        /**
         * @brief Get the key for this event
         *
         * This value is likely an ascii char, or a value from osgGA::GUIEventAdapter::KeySymbol
         */
        int getKey()
        {
            return _key;
        }

        /**
         * @brief Set the key for this event
         *
         * This value is likely an ascii char, or a value from osgGA::GUIEventAdapter::KeySymbol
         */
        void setKey(int key)
        {
            _key = key;
        }

        /**
         * @brief Get any modifiers on this key event
         *
         * This value is a mask created from osgGA::GUIEventAdapter::ModKeyMask
         */
        int getMod()
        {
            return _mod;
        }

        /**
         * @brief Set any modifiers on this key event
         *
         * This value is a mask created from osgGA::GUIEventAdapter::ModKeyMask
         */
        void setMod(int mod)
        {
            _mod = mod;
        }

        virtual InteractionEventType getEventType()
        {
            return KEYBOARD_INTER_EVENT;
        }

        virtual KeyboardInteractionEvent * asKeyboardEvent()
        {
            return this;
        }

    protected:
        int _key; ///< key value
        int _mod; ///< key modifier
};

class PositionInteractionEvent : public HandInteractionEvent
{
    public:
        PositionInteractionEvent() :
                HandInteractionEvent()
        {
        }

        osg::Vec3 & getPosition()
        {
            return _position;
        }

        void setPosition(osg::Vec3 pos)
        {
            _position = pos;
        }

        virtual InteractionEventType getEventType()
        {
            return POSITION_INTER_EVENT;
        }

        virtual PositionInteractionEvent * asPositionEvent()
        {
            return this;
        }

    protected:
        osg::Vec3 _position;
};

/**
 * @brief Creates a new Interaction event object of the given type and loads it with the
 * data of the given event
 * @param event pointer to the event to copy
 * @param type class identifier for the source event
 *
 * This function is needed since the class of the input event is unknown, but can be
 * identified from the type
 */
InteractionEvent * loadEventWithType(InteractionEvent * event,
        InteractionEventType type);
void storeEvent(InteractionEvent * event, void * des);
int getEventSize(InteractionEventType type);

}

#endif
