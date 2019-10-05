
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

namespace cvr
{

	class CVRMENU_EXPORT UIElement
	{
	public:
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


		//Getter/setter for _handle property (determines if the item can be used to move the menu around
		virtual void setIsHandle(bool h) { _handle = h; }
		virtual bool getIsHandle() { return _handle; }

		virtual void addChild(UIElement* e);
		virtual void removeChild(UIElement* e);

		virtual osg::Group* getGroup() { return _group; }

	protected:
		osg::ref_ptr<osg::Group> _group;
		osg::ref_ptr<osg::Geode> _intersect;

		osg::Vec3 _percentPos;
		osg::Vec3 _absolutePos;
		osg::Vec3 _percentSize;
		osg::Vec3 _absoluteSize;

		osg::Vec3 _actualPos;
		osg::Vec3 _actualSize;

		std::vector<std::shared_ptr<UIElement> > _children;
		std::vector<std::shared_ptr<UIElement> > _parents;

		bool _dirty;
		bool _handle;

		osg::Vec3 _lastHitPoint;
	};
}

#endif