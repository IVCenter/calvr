/*
 * openvrdevice.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 *		Modified by: Glynn Williams
 */


#ifdef _WIN32
	#include <Windows.h>
#endif

#include <osg/Geometry>
#include <osgViewer/Renderer>
#include <osgViewer/GraphicsWindow>
#include <osg/CullFace>
#include <osg/PrimitiveSet>
#include <iostream>

#include <cvrKernel/OpenVRDevice.h>

#ifndef GL_TEXTURE_MAX_LEVEL
	#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

void ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep(nMilliseconds);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}

using namespace cvr;

static const OSG_GLExtensions* getGLExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::FBOExtensions::instance(state.getContextID(), true);
#endif
}

static const OSG_Texture_Extensions* getTextureExtensions(const osg::State& state)
{
#if(OSG_VERSION_GREATER_OR_EQUAL(3, 4, 0))
	return state.get<osg::GLExtensions>();
#else
	return osg::Texture::getExtensions(state.getContextID(), true);
#endif
}

static osg::Matrix convertMatrix34(const vr::HmdMatrix34_t &mat34)
{
	osg::Matrix matrix(
		mat34.m[0][0], mat34.m[1][0], mat34.m[2][0], 0.0,
		mat34.m[0][1], mat34.m[1][1], mat34.m[2][1], 0.0,
		mat34.m[0][2], mat34.m[1][2], mat34.m[2][2], 0.0,
		mat34.m[0][3], mat34.m[1][3], mat34.m[2][3], 1.0f
		);
	return matrix;
}

static osg::Matrix convertMatrix44(const vr::HmdMatrix44_t &mat44)
{
	osg::Matrix matrix(
		mat44.m[0][0], mat44.m[1][0], mat44.m[2][0], mat44.m[3][0],
		mat44.m[0][1], mat44.m[1][1], mat44.m[2][1], mat44.m[3][1],
		mat44.m[0][2], mat44.m[1][2], mat44.m[2][2], mat44.m[3][2],
		mat44.m[0][3], mat44.m[1][3], mat44.m[2][3], mat44.m[3][3]
		);
	return matrix;
}

OpenVRDevice* OpenVRDevice::_instance = nullptr;

OpenVRDevice::OpenVRDevice(float nearClip, float farClip, const float worldUnitsPerMetre, const int samples) :
	m_vrSystem(nullptr),
	m_vrRenderModels(nullptr),
	m_worldUnitsPerMetre(worldUnitsPerMetre),
	m_position(osg::Vec3(0.0f, 0.0f, 0.0f)),
	m_orientation(osg::Quat(0.0f, 0.0f, 0.0f, 1.0f)),
	m_nearClip(nearClip), m_farClip(farClip),
	m_samples(samples)
{
	if (_instance == nullptr)
	{
		_instance = this;
	}
	m_universeMatrix = osg::Matrix::identity();
	trySetProcessAsHighPriority();

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_vrSystem = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		m_vrSystem = nullptr;
		osg::notify(osg::WARN) 
			<< "Error: Unable to initialize the OpenVR library.\n"
			<< "Reason: " << vr::VR_GetVRInitErrorAsEnglishDescription( eError ) << std::endl;
		return;
	}

	if ( !vr::VRCompositor() )
	{
		m_vrSystem = nullptr;
		vr::VR_Shutdown();
		osg::notify(osg::WARN) << "Error: Compositor initialization failed" << std::endl;
		return;
	}

	m_vrRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (m_vrRenderModels == nullptr)
	{
		m_vrSystem = nullptr;
		vr::VR_Shutdown();
		osg::notify(osg::WARN) 
			<< "Error: Unable to get render model interface!\n"
			<< "Reason: " << vr::VR_GetVRInitErrorAsEnglishDescription( eError ) << std::endl;
		return;
	}

	m_RenderModels = std::map<std::string, osg::Geometry*>();

	std::string driverName = GetDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	std::string deviceSerialNumber = GetDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	osg::notify(osg::NOTICE) << "HMD driver name: "<< driverName << std::endl;
	osg::notify(osg::NOTICE) << "HMD device serial number: " << deviceSerialNumber << std::endl;
}

std::string OpenVRDevice::GetDeviceProperty(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError)
{
	uint32_t bufferLen = m_vrSystem->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (bufferLen == 0)
	{
		return "";
	}

	char* buffer = new char[bufferLen];
	bufferLen = m_vrSystem->GetStringTrackedDeviceProperty(unDevice, prop, buffer, bufferLen, peError);
	std::string result = buffer;
	delete [] buffer;
	return result;
}

osg::Geometry* OpenVRDevice::GetOrLoadRenderModel(std::string name)
{
	osg::Geometry* renderModel = NULL;

	if (m_RenderModels.find(name) != m_RenderModels.end())
	{
		renderModel =  m_RenderModels[name];
	}


	vr::RenderModel_t* model;
	vr::EVRRenderModelError error;
	if (!renderModel)
	{
		std::cerr << "Loading render model with name: " << name << std::endl;

		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(name.c_str(), &model);
			if (error != vr::VRRenderModelError_Loading)
			{
				break;
			}
			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render model %s - %s\n", name.c_str(), vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL;
		}

		vr::RenderModel_TextureMap_t *texture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(model->diffuseTextureId, &texture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render texture id:%d for render model %s\n", model->diffuseTextureId, name.c_str());
			vr::VRRenderModels()->FreeRenderModel(model);
			return NULL; // move on to the next tracked device
		}

		renderModel = new osg::Geometry();
		//renderModel->setVertexArray()
		osg::Vec3Array* vertices = new osg::Vec3Array();
		osg::Vec3Array* normals = new osg::Vec3Array();
		osg::Vec2Array* texcoords = new osg::Vec2Array();
		//osg::UIntArray* indices = new osg::UIntArray();
		osg::DrawElementsUInt* ebo = new osg::DrawElementsUInt(GL_TRIANGLES);

		for (int i = 0; i < model->unVertexCount; ++i)
		{
			vr::HmdVector3_t pos = model->rVertexData[i].vPosition;
			vr::HmdVector3_t norm = model->rVertexData[i].vNormal;

			//Change of axes y -> -z, z -> y
			vertices->push_back(osg::Vec3(pos.v[0], -pos.v[2], pos.v[1]) * 1000.0f); // scale up to mm as standard unit
			normals->push_back(osg::Vec3(norm.v[0], -norm.v[2], norm.v[1]));

			texcoords->push_back(osg::Vec2(model->rVertexData[i].rfTextureCoord[0], model->rVertexData[i].rfTextureCoord[1]));
		}

		for (int i = 0; i < model->unTriangleCount * 3; ++i)
		{
			//indices->push_back(model->rIndexData[i]);
			ebo->addElement(model->rIndexData[i]);
		}


		std::cerr << "Loaded " << model->unVertexCount << " vertices and " << model->unTriangleCount << " triangles" << std::endl;




		renderModel->setVertexArray(vertices);
		renderModel->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		renderModel->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		renderModel->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
		renderModel->addPrimitiveSet(ebo);
		//osg::Vec4Array* colors = new osg::Vec4Array();
		//colors->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
		//renderModel->setColorArray(colors, osg::Array::BIND_OVERALL);
		renderModel->setTexCoordArray(0, texcoords, osg::Array::BIND_PER_VERTEX);
		
		osg::Image* image = new osg::Image();
		image->allocateImage(texture->unWidth, texture->unHeight, 1, GL_RGBA, GL_BYTE);
		
		const uint8_t* datasrc = texture->rubTextureMapData;
		uint8_t* datadest = (uint8_t*)image->data();

		//Copy the image from the vr texture to the osg image
		memcpy(datadest, datasrc, image->s() * image->t() * image->r() * sizeof(uint8_t) * 4);

		std::cerr << "Loaded " << texture->unWidth << "x" << texture->unHeight << " pixel diffuse texture" << std::endl;
		
		//image->data
		osg::Texture2D* tex = new osg::Texture2D(image);
		tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
		tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
		tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		renderModel->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

		vr::VRRenderModels()->FreeRenderModel(model);
		vr::VRRenderModels()->FreeTexture(texture);

		m_RenderModels[name] = renderModel;
	}

	return renderModel;
}

void OpenVRDevice::init()
{
	calculateEyeAdjustment();
	calculateProjectionMatrices();
}

bool OpenVRDevice::hmdPresent()
{
	return vr::VR_IsHmdPresent();
}

bool OpenVRDevice::hmdInitialized() const
{
	return m_vrSystem != nullptr && m_vrRenderModels != nullptr;
}

osg::Matrix OpenVRDevice::projectionMatrixCenter() const
{
	osg::Matrix projectionMatrixCenter;
	projectionMatrixCenter = m_leftEyeProjectionMatrix.operator*(0.5) + m_rightEyeProjectionMatrix.operator*(0.5);
	return projectionMatrixCenter;
}

osg::Matrix OpenVRDevice::projectionMatrixLeft() const
{
	return m_leftEyeProjectionMatrix;
}

osg::Matrix OpenVRDevice::projectionMatrixRight() const
{
	return m_rightEyeProjectionMatrix;
}

osg::Matrix OpenVRDevice::projectionOffsetMatrixLeft() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_leftEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OpenVRDevice::projectionOffsetMatrixRight() const
{
	osg::Matrix projectionOffsetMatrix;
	float offset = m_rightEyeProjectionMatrix(2, 0);
	projectionOffsetMatrix.makeTranslate(osg::Vec3(-offset, 0.0, 0.0));
	return projectionOffsetMatrix;
}

osg::Matrix OpenVRDevice::viewMatrixLeft() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(-m_leftEyeAdjust);
	return viewMatrix;
}

osg::Matrix OpenVRDevice::viewMatrixRight() const
{
	osg::Matrix viewMatrix;
	viewMatrix.makeTranslate(-m_rightEyeAdjust);
	return viewMatrix;
}

void OpenVRDevice::resetSensorOrientation() const
{
	m_vrSystem->ResetSeatedZeroPose();
}

void OpenVRDevice::updatePose()
{
	// Runs the assignIDs() method if...
	if (hmdDeviceID < 0 || // HMD id not yet initialized
		controllers[0].deviceID < 0 || // One of the controllers not yet initialized
		controllers[1].deviceID < 0 ||
		controllers[0].deviceID == controllers[1].deviceID || // Both controllerData structs store the same deviceId
		controllers[0].hand == controllers[1].hand) // TODO: Add function to reassign ids each minute or so
	{
		assignIDs();
		calculateProjectionMatrices();
	}


	/*
	vr::VRCompositor()->SetTrackingSpace(vr::TrackingUniverseStanding);
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) poses[i].bPoseIsValid = false;
	//waitgetposes NEEDS TO BE RUN otherwise cant submit to vrcompositor?
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	
	const vr::TrackedDevicePose_t& pose = poses[vr::k_unTrackedDeviceIndex_Hmd];
	if (pose.bPoseIsValid)
	{
		osg::Matrix matrix = convertMatrix34(pose.mDeviceToAbsoluteTracking);
		osg::Matrix poseTransform = osg::Matrix::inverse(matrix);
		m_position = poseTransform.getTrans() * m_worldUnitsPerMetre;
		m_orientation = poseTransform.getRotate();
	}
	*/


	vr::TrackedDevicePose_t TDP;
	vr::VRControllerState_t VRCS;

	
	//updateControllerEvents();
	//Controllers update pose
	for (int i = 0; i < numControllers; ++i)
	{
		ControllerData* pC = &(controllers[i]);
		if (pC->deviceID < 0 || !m_vrSystem->IsTrackedDeviceConnected(i) || pC->hand < 0)
			continue;
		m_vrSystem->GetControllerStateWithPose(vr::TrackingUniverseStanding, pC->deviceID, &VRCS, sizeof(VRCS), &TDP);

		pC->isValid = TDP.bPoseIsValid;
		pC->trigVal = VRCS.rAxis[1].x;
		pC->padX = VRCS.rAxis[0].x;
		pC->padY = VRCS.rAxis[0].y;

		if (pC->isValid)
		{
			osg::Matrix matrix = convertMatrix34(TDP.mDeviceToAbsoluteTracking);
			osg::Matrix poseTransform = matrix * m_universeMatrix;// osg::Matrix::inverse(matrix);
			//poseTransform.postMult(osg::Matrix::inverse(m_universeMatrix));
			pC->position = poseTransform.getTrans() * m_worldUnitsPerMetre;
			pC->rotation = poseTransform.getRotate();
			//std::cout << "Controller: " << pC->position.x() << ", " << pC->position.y() << ", " << pC->position.z() << std::endl;
		}

		//update rendermodel
		std::string sRenderModelName = GetDeviceProperty(pC->deviceID, vr::Prop_RenderModelName_String);
		if (sRenderModelName != pC->renderModelName)
		{
			pC->renderModel = GetOrLoadRenderModel(sRenderModelName);
			pC->renderModelName = sRenderModelName;
		}
	}

	//Trackers update pose
	for (int i = 0; i < numTrackers; ++i)
	{
		TrackerData* pT = &(trackers[i]);
		if (pT->deviceID < 0 || !m_vrSystem->IsTrackedDeviceConnected(i))
			continue;

		m_vrSystem->GetControllerStateWithPose(vr::TrackingUniverseStanding, pT->deviceID, &VRCS, sizeof(VRCS), &TDP);

		if (pT->isValid)
		{
			osg::Matrix matrix = convertMatrix34(TDP.mDeviceToAbsoluteTracking);
			osg::Matrix poseTransform = matrix;//osg::Matrix::inverse(matrix);
			pT->position = poseTransform.getTrans() * m_worldUnitsPerMetre;
			pT->rotation = poseTransform.getRotate();
		}
	}

	
	
	vr::TrackedDevicePose_t poses[1];
	poses[0].bPoseIsValid = false;
	//waitgetposes NEEDS TO BE RUN otherwise cant submit to vrcompositor?
	vr::VRCompositor()->WaitGetPoses(poses, 1, NULL, 0);

	const vr::TrackedDevicePose_t& pose = poses[vr::k_unTrackedDeviceIndex_Hmd];
	
	if (pose.bPoseIsValid)
	{
		osg::Matrix matrix = convertMatrix34(pose.mDeviceToAbsoluteTracking);
		osg::Matrix poseTransform = matrix * m_universeMatrix;
		//poseTransform.postMult(osg::Matrix::inverse(m_universeMatrix));
		//poseTransform = osg::Matrix::inverse(poseTransform);
		m_position = poseTransform.getTrans() * m_worldUnitsPerMetre;
		m_orientation = poseTransform.getRotate();
		//std::cout << "HMDHeadset: " << m_position.x() << ", " << m_position.y() << ", " << m_position.z() << std::endl;
		//std::cout << m_orientation.x() << ", " << m_orientation.y() << ", " << m_orientation.z() << ", " << m_orientation.w() << std::endl;
	}
	
	//HMD update pose
	/*
	m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseSeated, 0, &TDP, 1);
	if (TDP.bPoseIsValid)
	{
		osg::Matrix matrix = convertMatrix34(TDP.mDeviceToAbsoluteTracking);
		osg::Matrix poseTransform = osg::Matrix::inverse(matrix);
		m_position = poseTransform.getTrans() * m_worldUnitsPerMetre;
		m_orientation = poseTransform.getRotate();
	}
	*/
}

void OpenVRDevice::assignIDs()
{
	int numCInit = 0;
	int numTInit = 0;

	for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
	{
		if (!m_vrSystem->IsTrackedDeviceConnected(i))
			continue; //skip id if device not connected

		vr::ETrackedDeviceClass tdc = m_vrSystem->GetTrackedDeviceClass(i);

		switch (tdc)
		{
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				hmdDeviceID = i;
				break;
			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				if (numCInit < 2)
				{
					ControllerData* pC = &(controllers[numCInit]);
					
					vr::ETrackedControllerRole role = m_vrSystem->GetControllerRoleForTrackedDeviceIndex(i);
					switch (role)
					{
						case vr::TrackedControllerRole_LeftHand:
							pC->hand = 1;
							break;
						case vr::TrackedControllerRole_RightHand:
							pC->hand = 2;
							break;
						default:
							pC->hand = 0;
							break;
					}
					
					pC->deviceID = i;

					//vr::VRControllerState_t state = vr::VRControllerState_t();

					//m_vrSystem->GetControllerState(i, &state, sizeof(state));

					//for (int j = 0; j < vr::k_unControllerStateAxisCount; ++j) {
					//	int prop = m_vrSystem->GetInt32TrackedDeviceProperty(pC->deviceID, (vr::ETrackedDeviceProperty)(vr::Prop_Axis0Type_Int32 + j));
					//
					//	if (prop == vr::k_eControllerAxis_Trigger)
					//		pC->idtrigger = j;
					//	else if (prop == vr::k_eControllerAxis_TrackPad)
					//		pC->idpad = j;
					//}

					++numCInit;
				}
				break;
			case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
				TrackerData* pT = &(trackers[numTInit]);
				pT->deviceID = i;
				++numTInit;
		}
	}
}

void OpenVRDevice::updateControllerEvents()
{
	vr::VREvent_t event;
	while (m_vrSystem->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{
			case vr::VREvent_Quit:
			case vr::VREvent_ProcessQuit:
			case vr::VREvent_QuitAcknowledged:
				std::cerr << "OpenVR has quit!" << std::endl;
				break;
			case vr::VREvent_ButtonPress:
			case vr::VREvent_ButtonUnpress:
				for (int i = 0; i < 2; ++i)
				{
					ControllerData* pC = &(controllers[i]);
					if (event.trackedDeviceIndex == pC->deviceID)
					{
						switch (event.data.controller.button)
						{
							case Button::MENU:
								if (event.eventType == vr::VREvent_ButtonPress)
								{
									pC->menuPressed = true;
								}
								else
								{
									pC->menuPressed = false;
								}
								break;
							case Button::GRIP:
								if (event.eventType == vr::VREvent_ButtonPress)
								{
									pC->gripPressed = true;
								}
								else
								{
									pC->gripPressed = false;
								}
								break;
							case Button::PAD:
								if (event.eventType == vr::VREvent_ButtonPress)
								{
									pC->padPressed = true;
								}
								else
								{
									pC->padPressed = false;
								}
								break;
							case Button::TRIGGER:
								if (event.eventType == vr::VREvent_ButtonPress)
								{
									pC->triggerPressed = true;
								}
								else
								{
									pC->triggerPressed = false;
								}
								break;
						}
					}
				}
		}
	}
}


class OpenVRInitialDrawCallback : public osg::Camera::DrawCallback
{
public:
	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		osg::GraphicsOperation* graphicsOperation = renderInfo.getCurrentCamera()->getRenderer();
		osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(graphicsOperation);
		if (renderer != nullptr)
		{
			// Disable normal OSG FBO camera setup because it will undo the MSAA FBO configuration.
			renderer->setCameraRequiresSetUp(false);
		}
	}
};

void OpenVRDevice::shutdown(osg::GraphicsContext* gc)
{
	if (m_vrSystem != nullptr)
	{
		vr::VR_Shutdown();
		m_vrSystem = nullptr;
	}

}

/* Protected functions */
OpenVRDevice::~OpenVRDevice()
{
	// shutdown(gc);
}

void OpenVRDevice::calculateEyeAdjustment()
{
	vr::HmdMatrix34_t mat;
	
	mat = m_vrSystem->GetEyeToHeadTransform(vr::Eye_Left);
	m_leftEyeAdjust = convertMatrix34(mat).getTrans();
	mat = m_vrSystem->GetEyeToHeadTransform(vr::Eye_Right);
	m_rightEyeAdjust = convertMatrix34(mat).getTrans();
	
	// Display IPD
	float ipd = (m_leftEyeAdjust - m_rightEyeAdjust).length();
	osg::notify(osg::ALWAYS) << "Interpupillary Distance (IPD): " << ipd * 1000.0f << " mm" << std::endl;












	// Scale to world units
	m_leftEyeAdjust *= m_worldUnitsPerMetre;
	m_rightEyeAdjust *= m_worldUnitsPerMetre;
}

void OpenVRDevice::calculateProjectionMatrices()
{
	vr::HmdMatrix44_t mat;
	
	mat = m_vrSystem->GetProjectionMatrix(vr::Eye_Left, m_nearClip, m_farClip);
	m_leftEyeProjectionMatrix = convertMatrix44(mat);

	mat = m_vrSystem->GetProjectionMatrix(vr::Eye_Right, m_nearClip, m_farClip);
	m_rightEyeProjectionMatrix = convertMatrix44(mat);
}

void OpenVRDevice::trySetProcessAsHighPriority() const
{
	// Require at least 4 processors, otherwise the process could occupy the machine.
	if (OpenThreads::GetNumberOfProcessors() >= 4)
	{
#ifdef _WIN32
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
	}
}

