
#ifndef CALVR_UI_ELEMENT_H
#define CALVR_UI_ELEMENT_H

#include <cvrMenu/Export.h>
#include <cvrKernel/InteractionEvent.h>
#include <cvrUtil/Intersection.h>

#include <osg/Vec3>
#include <osg/Node>
#include <osg/Geode>
#include <osg/MatrixTransform>

#include <vector>
#include <memory>

namespace cvr
{

	class CVRMENU_EXPORT UIElement
	{
	public:

		enum Alignment {
			NONE,
			LEFT_TOP,
			LEFT_CENTER,
			LEFT_BOTTOM,
			CENTER_TOP,
			CENTER_CENTER,
			CENTER_BOTTOM,
			RIGHT_TOP,
			RIGHT_CENTER,
			RIGHT_BOTTOM
		};

		UIElement();
		virtual ~UIElement();

		virtual void updateElement(osg::Vec3 pos, osg::Vec3 size);

		virtual void createGeometry();
		virtual void updateGeometry();

		virtual bool processEvent(InteractionEvent * event) { return false; }

		//Called when pointer enters and exits the intersection geode of this element
		virtual void processHover(bool enter) {};

		//Depth-first search to find intersected element, returns null if none found
		virtual UIElement* processIsect(IsectInfo & isect, int hand);

		/**
		 * @brief Called each frame the item is selected
		 * @param pointerStart start point of pointer intersection test
		 * @param pointerEnd end point of pointer intersection test
		 */
		virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd) {};

		virtual void setDirty(bool dirty) { _dirty = dirty; }
		virtual bool getDirty() { return _dirty; }

		/**
		 * @brief Set position relative to parent using coordinates where (-0.5,-0.5,-0.5) and (0.5,0.5,0.5)
		 * are the bounding box corners and (0,0,0) means the center of this element will be at the same
		 * position as the center of parent element (AKA the anchor for the position is the center)
		 */
		virtual void setPercentPos(osg::Vec3 pos);
		virtual void setAbsolutePos(osg::Vec3 pos);

		virtual osg::Vec3 getPercentPos() { return _percentPos; }
		virtual osg::Vec3 getAbsolutePos() { return _absolutePos; }

		virtual void setPercentSize(osg::Vec3 size);
		virtual void setAbsoluteSize(osg::Vec3 size);

		virtual osg::Vec3 getPercentSize() { return _percentSize; }
		virtual osg::Vec3 getAbsoluteSize() { return _absoluteSize; }

		virtual void setPos(osg::Vec3 percentPos, osg::Vec3 absolutePos)
		{
			setPercentPos(percentPos);
			setAbsolutePos(absolutePos);
		}

		virtual void setSize(osg::Vec3 percentSize, osg::Vec3 absoluteSize)
		{
			setPercentSize(percentSize);
			setAbsoluteSize(absoluteSize);
		}

		virtual void setAspect(osg::Vec3 aspect, bool useAspect = true);
		virtual osg::Vec3 getAspect() { return _aspect; }

		virtual void setAlign(Alignment align);
		virtual void dummy(int dummy);

		virtual Alignment getAlign() { return _alignment; }


		//Getter/setter for _handle property (determines if the item can be used to move the menu around)
		virtual void setIsHandle(bool h) { _handle = h; }
		virtual bool getIsHandle() { return _handle; }

		virtual int getLastHand() { return _lastHand; }

		virtual void addChild(UIElement* e);
		virtual void removeChild(UIElement* e);
		virtual UIElement* getChild(int index);

		virtual osg::Group* getGroup() { return _group; }

		virtual void setActive(bool isActive) { _active = isActive; }
		virtual bool getActive() { return _active; }
		UIElement* _parent;

	protected:

		virtual void calculateBounds(osg::Vec3 pos, osg::Vec3 size);

		osg::ref_ptr<osg::Group> _group;
		osg::ref_ptr<osg::Geode> _intersect;
	
		osg::Vec3 _percentPos;
		osg::Vec3 _absolutePos;
		osg::Vec3 _percentSize;
		osg::Vec3 _absoluteSize;

		osg::Vec3 _aspect;
		bool _useAspect;
		Alignment _alignment;

		osg::Vec3 _actualPos;
		osg::Vec3 _actualSize;



		std::vector<std::shared_ptr<UIElement> > _children;
		

		bool _dirty;
		bool _handle;
		bool _active;

		osg::Vec3 _lastHitPoint;
		int _lastHand;
	};
}

#endif
