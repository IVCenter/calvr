#ifndef _CUI_MARKER_H_
#define _CUI_MARKER_H_

// OSG:
#include <osg/ShapeDrawable>

// CUI:
#include "Widget.h"
#include "Events.h"
#include "InputDevice.h"

namespace covise {
   class coTrackerButtonInteraction;
}

namespace cui
{
  class Interaction;
  class Measure;
  class MarkerListener;

  /** This is an implementation of a cone-shaped marker. It provides a pop-up
    menu to change its color, size, and to remove it.
    @author Devon Penney
    @author Jurgen Schulze
  */
  class CUIEXPORT Marker : public Widget, public Events
  {
    public:
      enum GeometryType
      {
        CONE, SPHERE, BOX
      };
      Marker(GeometryType, Interaction* = NULL);
      Marker(GeometryType, Interaction*, float, osg::Vec4&);
      virtual ~Marker();
      
      // Inherited:
      virtual void cursorEnter(InputDevice*);
      virtual void cursorUpdate(InputDevice*);
      virtual void cursorLeave(InputDevice*);
      virtual void buttonEvent(InputDevice*, int);
      virtual void joystickEvent(InputDevice*);
      virtual void wheelEvent(InputDevice*, int);

      // Local:
      virtual void init(GeometryType, Interaction*, float, osg::Vec4&);
      virtual void setSize(float);
      virtual float getSize();
      virtual void setColor(osg::Vec4);
      virtual void setHue(float);
      virtual float getHue();
      virtual void setOpacity(float);
      virtual float getOpacity();
      virtual osg::Vec4 getColor();
      virtual void setPosition(osg::Vec3);
      virtual osg::Vec3 getPosition();
      virtual osg::Vec3 getDirection();
      virtual void addMarkerListener(MarkerListener*);
      virtual void invertColor();

    protected:
      covise::coTrackerButtonInteraction* _interactionA;   ///< interaction for first button
      covise::coTrackerButtonInteraction* _interactionB;   ///< interaction for second button
      static const float RATIO_LENGTH2RADIUS;
      osg::Matrix _lastWand2w;
      osg::ShapeDrawable* _shapeDrawable;
      osg::Cone* _coneShape;
      osg::Sphere* _sphereShape;
      osg::Box* _boxShape;
      Interaction* _interaction;
      std::list<MarkerListener*> _listeners;
      osg::ref_ptr<osg::MatrixTransform> _markerXF;
      cui::InputDevice::CursorType _prevType;    ///< pointer style when pointer entered box
      GeometryType _gt;
      float _size;
      osg::Vec4 _color;

      osg::Geode* createCone(osg::Vec4&);
      osg::Geode* createSphere(osg::Vec4&);
      osg::Geode* createBox(osg::Vec4&);
  };

  class MarkerListener
  {
   public:
    virtual ~MarkerListener() {}
    virtual void markerEvent(Marker*, int, int) = 0;
  };
}

#endif
