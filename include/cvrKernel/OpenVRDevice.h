/*
 * openvrdevice.h
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#ifndef _OSG_OPENVRDEVICE_H_
#define _OSG_OPENVRDEVICE_H_

// Include the OpenVR SDK
#include <openvr.h>

#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Version>
#include <osg/FrameBufferObject>

#include <cvrKernel/Export.h>


#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	typedef osg::GLExtensions OSG_GLExtensions;
	typedef osg::GLExtensions OSG_Texture_Extensions;
#else
	typedef osg::FBOExtensions OSG_GLExtensions;
	typedef osg::Texture::Extensions OSG_Texture_Extensions;
#endif

namespace cvr
{

	class CVRKERNEL_EXPORT OpenVRDevice : public osg::Referenced
	{

	public:

		static OpenVRDevice* instance()
		{
			return _instance;
		}

		typedef enum _Button
		{
			MENU = 1,
			GRIP = 2,
			PAD = 32,
			TRIGGER = 33
		} Button;

		typedef enum Eye_
		{
			LEFT = 0,
			RIGHT = 1,
			COUNT = 2
		} Eye;

		typedef struct _ControllerData
		{
			// Fields to be initialzed by iterateAssignIds() and setHands()
			int deviceID = -1;  // Device ID according to the SteamVR system
			int hand = -1;      // 0=invalid 1=left 2=right
			int idtrigger = -1; // Trigger axis id
			int idpad = -1;     // Touchpad axis id

			// Analog button data to be set in ContollerCoods()
			float padX;
			float padY;
			float trigVal;

			bool menuPressed = false;
			bool gripPressed = false;
			bool padPressed = false;
			bool triggerPressed = false;

			osg::Vec3 position;
			osg::Quat rotation;

			bool isValid;
		} ControllerData;

		typedef struct _TrackerData
		{
			int deviceID = -1; // Device ID according to the SteamVR system
			osg::Vec3 position;
			osg::Quat rotation;
			bool isValid;
		} TrackerData;

		ControllerData controllers[2];
		TrackerData trackers[32];
		int hmdDeviceID = -1;
		int numControllers = 2;
		int numTrackers = 0;


		OpenVRDevice(float nearClip, float farClip, const float worldUnitsPerMetre = 1.0f, const int samples = 0);
		void init();
		void calculateEyeAdjustment();
		void calculateProjectionMatrices();
		void shutdown(osg::GraphicsContext* gc);

		static bool hmdPresent();
		bool hmdInitialized() const;

		osg::Matrix projectionMatrixCenter() const;
		osg::Matrix projectionMatrixLeft() const;
		osg::Matrix projectionMatrixRight() const;

		osg::Matrix projectionOffsetMatrixLeft() const;
		osg::Matrix projectionOffsetMatrixRight() const;

		osg::Matrix viewMatrixLeft() const;
		osg::Matrix viewMatrixRight() const;

		float nearClip() const { return m_nearClip; }
		float farClip() const { return m_farClip; }
		void setNearClip(const float nearclip) { m_nearClip = nearclip; }
		void setFarClip(const float farclip) { m_farClip = farclip; }

		void resetSensorOrientation() const;
		void updatePose();

		void assignIDs();
		void updateControllerEvents();

		osg::Vec3 position() const { return m_position; }
		osg::Quat orientation() const { return m_orientation; }

		osg::Matrix getUniverseMatrix() const { return m_universeMatrix; }
		void setUniverseMatrix(osg::Matrix matrix) { m_universeMatrix = matrix; }

		vr::IVRSystem* vrSystem() const { return m_vrSystem; }

		osg::Geometry* GetOrLoadRenderModel(std::string name);

	protected:
		~OpenVRDevice(); // Since we inherit from osg::Referenced we must make destructor protected

		void trySetProcessAsHighPriority() const;

		osg::Matrixf m_leftEyeProjectionMatrix;
		osg::Matrixf m_rightEyeProjectionMatrix;
		osg::Matrixf m_universeMatrix;
		osg::Vec3f m_leftEyeAdjust;
		osg::Vec3f m_rightEyeAdjust;

		osg::Vec3 m_position;
		osg::Quat m_orientation;

		float m_nearClip;
		float m_farClip;
		int m_samples;

		vr::IVRSystem* m_vrSystem;
		vr::IVRRenderModels* m_vrRenderModels;
		const float m_worldUnitsPerMetre;
	private:
		std::string GetDeviceProperty(vr::TrackedDeviceProperty prop);
		OpenVRDevice(const OpenVRDevice&); // Do not allow copy
		OpenVRDevice& operator=(const OpenVRDevice&); // Do not allow assignment operator.

		std::map<std::string, osg::Geometry*> m_RenderModels;

		static OpenVRDevice* _instance;
	};

}

#endif /* _OSG_OPENVRDEVICE_H_ */
