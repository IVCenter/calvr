#include <cvrKernel/InteractionEvent.h>

namespace cvr
{

const char * interactionToName(Interaction i)
{
    switch(i)
    {
        case NO_INTERACTION:
            return "No Interaction";
        case BUTTON_DOWN:
            return "Button Down";
        case BUTTON_UP:
            return "Button Up";
        case BUTTON_DRAG:
            return "Button Drag";
        case BUTTON_DOUBLE_CLICK:
            return "Double Click";
        case VALUATOR:
            return "Valuator";
        case KEY_UP:
            return "Key Up";
        case KEY_DOWN:
            return "Key Down";
        case MOVE:
            return "Move";
        default:
            return "Unknown";
    }
}

InteractionEvent * loadEventWithType(InteractionEvent * event,
        InteractionEventType type)
{
    switch(type)
    {
        case TRACKED_BUTTON_INTER_EVENT:
        {
            TrackedButtonInteractionEvent * tbie =
                    new TrackedButtonInteractionEvent();
            *tbie = *((TrackedButtonInteractionEvent*)event);
            return tbie;
        }
        case MOUSE_INTER_EVENT:
        {
            MouseInteractionEvent * mie = new MouseInteractionEvent();
            *mie = *((MouseInteractionEvent*)event);
            return mie;
        }
        case POINTER_INTER_EVENT:
        {
            PointerInteractionEvent * pie = new PointerInteractionEvent();
            *pie = *((PointerInteractionEvent*)event);
            return pie;
        }
        case VALUATOR_INTER_EVENT:
        {
            ValuatorInteractionEvent * vie = new ValuatorInteractionEvent();
            *vie = *((ValuatorInteractionEvent*)event);
            return vie;
        }
        case KEYBOARD_INTER_EVENT:
        {
            KeyboardInteractionEvent * kie = new KeyboardInteractionEvent();
            *kie = *((KeyboardInteractionEvent*)event);
            return kie;
        }
        case POSITION_INTER_EVENT:
        {
            PositionInteractionEvent * pie = new PositionInteractionEvent();
            *pie = *((PositionInteractionEvent*)event);
            return pie;
        }
        case HAND_INTER_EVENT:
        {
            HandInteractionEvent * hie = new HandInteractionEvent();
            *hie = *((HandInteractionEvent*)event);
            return hie;
        }
        case INTER_EVENT:
        {
            InteractionEvent * ie = new InteractionEvent();
            *ie = *event;
            return ie;
        }
        default:
            return NULL;
    }
}

void storeEvent(InteractionEvent * event, void * des)
{
    switch(event->getEventType())
    {
        case TRACKED_BUTTON_INTER_EVENT:
            *((TrackedButtonInteractionEvent*)des) =
                    *event->asTrackedButtonEvent();
            break;
        case MOUSE_INTER_EVENT:
            *((MouseInteractionEvent*)des) = *event->asMouseEvent();
            break;
        case POINTER_INTER_EVENT:
            *((PointerInteractionEvent*)des) = *event->asPointerEvent();
            break;
        case VALUATOR_INTER_EVENT:
            *((ValuatorInteractionEvent*)des) = *event->asValuatorEvent();
            break;
        case KEYBOARD_INTER_EVENT:
            *((KeyboardInteractionEvent*)des) = *event->asKeyboardEvent();
            break;
        case POSITION_INTER_EVENT:
            *((PositionInteractionEvent*)des) = *event->asPositionEvent();
            break;
        case HAND_INTER_EVENT:
            *((HandInteractionEvent*)des) = *event->asHandEvent();
            break;
        case INTER_EVENT:
            *((InteractionEvent*)des) = *event;
            break;
        default:
            std::cerr << "Error: unknown event type in storeEvent, value = "
                    << (int)event->getEventType() << std::endl;
            break;
    }
}

int getEventSize(InteractionEventType type)
{
    switch(type)
    {
        case TRACKED_BUTTON_INTER_EVENT:
            return sizeof(TrackedButtonInteractionEvent);
        case MOUSE_INTER_EVENT:
            return sizeof(MouseInteractionEvent);
        case POINTER_INTER_EVENT:
            return sizeof(PointerInteractionEvent);
        case VALUATOR_INTER_EVENT:
            return sizeof(ValuatorInteractionEvent);
        case KEYBOARD_INTER_EVENT:
            return sizeof(KeyboardInteractionEvent);
        case POSITION_INTER_EVENT:
            return sizeof(PositionInteractionEvent);
        case HAND_INTER_EVENT:
            return sizeof(HandInteractionEvent);
        case INTER_EVENT:
            return sizeof(InteractionEvent);
        default:
            std::cerr << "Error: unknown event type in getEventSize, value = "
                    << (int)type << std::endl;
            return 0;
    }
}

}

