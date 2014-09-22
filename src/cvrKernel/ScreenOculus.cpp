#include <cvrKernel/ScreenOculus.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrInput/TrackingManager.h>
#include <cvrInput/TrackerBase.h>
#include <cvrInput/TrackerOculus.h>

#include <osgViewer/Renderer>

#include <iostream>

#ifdef WIN32
#pragma comment(lib, "Winmm.lib")
#define M_PI 3.141592653589793238462643
#endif

using namespace cvr;

std::string passThroughVertSrc =
"#version 150 compatibility \n"
"\n"
"void main(void) \n"
"{ \n"
"    gl_Position = gl_ModelViewMatrix * gl_Vertex; \n"
"    gl_TexCoord[0].xy = (gl_Position.xy + 1.0f) / 2.0f; \n"
"} \n";

std::string passThroughFragSrc =
"#version 150 compatibility                          \n"
"                                                    \n"
"uniform sampler2D texture; \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
"    gl_FragColor = texture2D(texture,gl_TexCoord[0].xy); \n"
"    //gl_FragColor = vec4(1.0,0.0,0.0,1.0); \n"
"}                                                   \n";

std::string meshVertSrc =
	"#version 150 compatibility\n"
    "#extension GL_ARB_gpu_shader5 : enable              \n"
	"#extension GL_ARB_explicit_attrib_location : enable \n"

    "uniform vec2 EyeToSourceUVScale;\n"
    "uniform vec2 EyeToSourceUVOffset;\n"

	"layout(location = 4) in vec2 TexCoord0;\n"
    "layout(location = 5) in vec2 TexCoord1;\n"
    "layout(location = 6) in vec2 TexCoord2;\n"
	"layout(location = 7) in float vf; \n"

    "out vec2 oTexCoord0;\n"
    "out vec2 oTexCoord1;\n"
    "out vec2 oTexCoord2;\n"
	"out float ovf;\n"

    "void main()\n"
    "{\n"
	"   gl_Position.xy = (gl_ModelViewMatrix * gl_Vertex).xy;\n"
	"   //gl_Position.xy = gl_Vertex.xy; \n"
    "   gl_Position.z = 0.5;\n"
    "   gl_Position.w = 1.0;\n"
    // Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).
    // Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)
    "   oTexCoord0 = TexCoord0 * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   oTexCoord0.y = 1.0-oTexCoord0.y;\n"
    "   oTexCoord1 = TexCoord1 * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   oTexCoord1.y = 1.0-oTexCoord1.y;\n"
    "   oTexCoord2 = TexCoord2 * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   oTexCoord2.y = 1.0-oTexCoord2.y;\n"
	"   ovf = vf;\n"
    "}\n";

std::string meshTimewarpVertSrc =
	"#version 150 compatibility\n"
	"#extension GL_ARB_gpu_shader5 : enable              \n"
	"#extension GL_ARB_explicit_attrib_location : enable \n"
    
    "uniform vec2 EyeToSourceUVScale;\n"
    "uniform vec2 EyeToSourceUVOffset;\n"
    "uniform mat4 EyeRotationStart;\n"
    "uniform mat4 EyeRotationEnd;\n"

	"layout(location = 4) in vec2 TexCoord0;\n"
    "layout(location = 5) in vec2 TexCoord1;\n"
    "layout(location = 6) in vec2 TexCoord2;\n"
	"layout(location = 7) in float vf; \n"
	"layout(location = 8) in float twf; \n"

    "out float ovf;\n"
    "out vec2 oTexCoord0;\n"
    "out vec2 oTexCoord1;\n"
    "out vec2 oTexCoord2;\n"

    "void main()\n"
    "{\n"
	"   gl_Position.xy = (gl_ModelViewMatrix * gl_Vertex).xy;\n"
    "   gl_Position.z = 0.0;\n"
    "   gl_Position.w = 1.0;\n"

    // Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).
    // These are now "real world" vectors in direction (x,y,1) relative to the eye of the HMD.
    "   vec3 TanEyeAngleR = vec3 ( TexCoord0.x, TexCoord0.y, 1.0 );\n"
    "   vec3 TanEyeAngleG = vec3 ( TexCoord1.x, TexCoord1.y, 1.0 );\n"
    "   vec3 TanEyeAngleB = vec3 ( TexCoord2.x, TexCoord2.y, 1.0 );\n"

    // Accurate time warp lerp vs. faster
    "   mat3 EyeRotation;\n"
    "   EyeRotation[0] = mix ( EyeRotationStart[0], EyeRotationEnd[0], twf ).xyz;\n"
    "   EyeRotation[1] = mix ( EyeRotationStart[1], EyeRotationEnd[1], twf ).xyz;\n"
    "   EyeRotation[2] = mix ( EyeRotationStart[2], EyeRotationEnd[2], twf ).xyz;\n"
    "   vec3 TransformedR   = EyeRotation * TanEyeAngleR;\n"
    "   vec3 TransformedG   = EyeRotation * TanEyeAngleG;\n"
    "   vec3 TransformedB   = EyeRotation * TanEyeAngleB;\n"

    // Project them back onto the Z=1 plane of the rendered images.
    "   float RecipZR = 1.0 / TransformedR.z;\n"
    "   float RecipZG = 1.0 / TransformedG.z;\n"
    "   float RecipZB = 1.0 / TransformedB.z;\n"
    "   vec2 FlattenedR = vec2 ( TransformedR.x * RecipZR, TransformedR.y * RecipZR );\n"
    "   vec2 FlattenedG = vec2 ( TransformedG.x * RecipZG, TransformedG.y * RecipZG );\n"
    "   vec2 FlattenedB = vec2 ( TransformedB.x * RecipZB, TransformedB.y * RecipZB );\n"

    // These are now still in TanEyeAngle space.
    // Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)
    "   vec2 SrcCoordR = FlattenedR * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   vec2 SrcCoordG = FlattenedG * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   vec2 SrcCoordB = FlattenedB * EyeToSourceUVScale + EyeToSourceUVOffset;\n"
    "   oTexCoord0 = SrcCoordR;\n"
    "   oTexCoord0.y = 1.0-oTexCoord0.y;\n"
    "   oTexCoord1 = SrcCoordG;\n"
    "   oTexCoord1.y = 1.0-oTexCoord1.y;\n"
    "   oTexCoord2 = SrcCoordB;\n"
    "   oTexCoord2.y = 1.0-oTexCoord2.y;\n"
    "   ovf = vf;\n"              // Used for vignette fade.
    "}\n";

std::string meshFragSrc =
	"#version 150 compatibility\n"
    
    "uniform sampler2D Texture;\n"
    
    "in vec2 oTexCoord0;\n"
    "in vec2 oTexCoord1;\n"
    "in vec2 oTexCoord2;\n"
	"in float ovf;\n"
    
    "void main()\n"
    "{\n"
    "   gl_FragColor.r =  ovf * texture2D(Texture, oTexCoord0).r;\n"
    "   gl_FragColor.g =  ovf * texture2D(Texture, oTexCoord1).g;\n"
    "   gl_FragColor.b =  ovf * texture2D(Texture, oTexCoord2).b;\n"
	"   //gl_FragColor = texture2D(Texture, oTexCoord0);\n"
    "   gl_FragColor.a = 1.0;\n"
	"   //gl_FragColor = vec4(1.0,0.0,0.0,1.0); \n"
    "}\n";

ScreenOculus::ScreenOculus() :
        ScreenBase()
{
	_hmd = NULL;
}

ScreenOculus::~ScreenOculus()
{
}

void ScreenOculus::init(int mode)
{
	_hmd = TrackerOculus::getHMD();

	if(!_hmd)
	{
		ovr_Initialize();
		_hmd = ovrHmd_Create(0);
	}

	if(_hmd)
	{
		/*std::cerr << "Oculus:" << std::endl;
		std::cerr << "Type: " << _hmd->Type << std::endl;
		std::cerr << "Product Name: " << _hmd->ProductName << std::endl;
		std::cerr << "Manufacturer: " << _hmd->Manufacturer << std::endl;
		std::cerr << "Vendor id: " << _hmd->VendorId << " Product id: " << _hmd->ProductId << std::endl;
		std::cerr << "Serial Number: " << _hmd->SerialNumber << std::endl;
		std::cerr << "Firmware: " << _hmd->FirmwareMajor << "." << _hmd->FirmwareMinor << std::endl;
		std::cerr << "Camera fov H: " << _hmd->CameraFrustumHFovInRadians << " rad, V: " << _hmd->CameraFrustumVFovInRadians << " rad" << std::endl;
		std::cerr << "Camera near: " << _hmd->CameraFrustumNearZInMeters << " m, far: " << _hmd->CameraFrustumFarZInMeters << " m" << std::endl;
		std::cerr << "Eye 0 fov uptan: " << _hmd->DefaultEyeFov[0].UpTan << " downtan: " << _hmd->DefaultEyeFov[0].DownTan << " righttan: " << _hmd->DefaultEyeFov[0].RightTan << " lefttan: " << _hmd->DefaultEyeFov[0].LeftTan << std::endl;
		std::cerr << "Eye 1 fov uptan: " << _hmd->DefaultEyeFov[1].UpTan << " downtan: " << _hmd->DefaultEyeFov[1].DownTan << " righttan: " << _hmd->DefaultEyeFov[1].RightTan << " lefttan: " << _hmd->DefaultEyeFov[1].LeftTan << std::endl;
		std::cerr << "Resolution: w: " << _hmd->Resolution.w << " h: " << _hmd->Resolution.h << std::endl;
		std::cerr << "Window pos: x: " << _hmd->WindowsPos.x << " y: " << _hmd->WindowsPos.y << std::endl;
		std::cerr << "Display name: " << _hmd->DisplayDeviceName << " id: " << _hmd->DisplayId << std::endl;*/

		/*ovrEyeRenderDesc eyeRenderDesc = ovrHmd_GetRenderDesc(_hmd, ovrEye_Left, _hmd->DefaultEyeFov[ovrEye_Left]);
		std::cerr << "FOV " << eyeRenderDesc.Fov.UpTan << " " << eyeRenderDesc.Fov.DownTan << " " << eyeRenderDesc.Fov.RightTan << " " << eyeRenderDesc.Fov.LeftTan << std::endl;
		std::cerr << "DVP " << eyeRenderDesc.DistortedViewport.Pos.x << " " << eyeRenderDesc.DistortedViewport.Pos.y << " " << eyeRenderDesc.DistortedViewport.Size.h << " " << eyeRenderDesc.DistortedViewport.Size.w << std::endl;
		std::cerr << "PPT " << eyeRenderDesc.PixelsPerTanAngleAtCenter.x << " " << eyeRenderDesc.PixelsPerTanAngleAtCenter.y << std::endl;
		std::cerr << "ViewAdj " << eyeRenderDesc.ViewAdjust.x << " " << eyeRenderDesc.ViewAdjust.y << " " << eyeRenderDesc.ViewAdjust.z << std::endl;*/

		ovrSizei recommenedleftSize = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Left,_hmd->DefaultEyeFov[0], 1.0f);
		//std::cerr << "Left texture size: w: " << recommenedleftSize.w << " h: " << recommenedleftSize.h << std::endl;

		ovrRecti vp;
		vp.Pos.x = 0;
		vp.Pos.y = 0;
		vp.Size.w = recommenedleftSize.w;
		vp.Size.h = recommenedleftSize.h;
		ovrVector2f scaleOffsetLeft[2];
		ovrHmd_GetRenderScaleAndOffset(_hmd->DefaultEyeFov[ovrEye_Left],recommenedleftSize,vp,scaleOffsetLeft);
		//std::cerr << "Dist " << out[0].x << " " << out[0].y << std::endl;
		//std::cerr << "Dist " << out[1].x << " " << out[1].y << std::endl;

		_leftTexture = new osg::Texture2D();
		_leftTexture->setTextureSize(recommenedleftSize.w,recommenedleftSize.h);
		_leftTexture->setInternalFormat(GL_RGBA);
		_leftTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		_leftTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
		_leftTexture->setResizeNonPowerOfTwoHint(false);
		_leftTexture->setUseHardwareMipMapGeneration(false);

		_leftDepthTexture = new osg::Texture2D();
		_leftDepthTexture->setTextureSize(recommenedleftSize.w,recommenedleftSize.h);
		_leftDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
		_leftDepthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		_leftDepthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		_leftDepthTexture->setResizeNonPowerOfTwoHint(false);
		_leftDepthTexture->setUseHardwareMipMapGeneration(false);

		ovrSizei recommenedrightSize = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Right,_hmd->DefaultEyeFov[1], 1.0f);
		//std::cerr << "Right texture size: w: " << recommenedrightSize.w << " h: " << recommenedrightSize.h << std::endl;

		ovrVector2f scaleOffsetRight[2];
		ovrHmd_GetRenderScaleAndOffset(_hmd->DefaultEyeFov[ovrEye_Right],recommenedrightSize,vp,scaleOffsetRight);
		//std::cerr << "ds " << scaleOffsetRight[0].x << " " << scaleOffsetRight[0].y << std::endl;
		//std::cerr << "do " << scaleOffsetRight[1].x << " " << scaleOffsetRight[1].y << std::endl;

		_rightTexture = new osg::Texture2D();
		_rightTexture->setTextureSize(recommenedrightSize.w,recommenedrightSize.h);
		_rightTexture->setInternalFormat(GL_RGBA);
		_rightTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		_rightTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
		_rightTexture->setResizeNonPowerOfTwoHint(false);
		_rightTexture->setUseHardwareMipMapGeneration(false);

		_rightDepthTexture = new osg::Texture2D();
		_rightDepthTexture->setTextureSize(recommenedrightSize.w,recommenedrightSize.h);
		_rightDepthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
		_rightDepthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		_rightDepthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		_rightDepthTexture->setResizeNonPowerOfTwoHint(false);
		_rightDepthTexture->setUseHardwareMipMapGeneration(false);

		_cameraLeft = new osg::Camera();
		_cameraRight = new osg::Camera();

		osg::DisplaySettings * ds = new osg::DisplaySettings();
		_cameraLeft->setDisplaySettings(ds);

		CVRViewer::instance()->addSlave(_cameraLeft.get(),osg::Matrixd(),
            osg::Matrixd());
		defaultCameraInit(_cameraLeft.get());
		_cameraLeft->setViewport(new osg::Viewport(0,0,recommenedleftSize.w,recommenedleftSize.h));

		osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*>(_cameraLeft->getRenderer());
		if(!renderer)
		{
			std::cerr << "Error getting renderer pointer." << std::endl;
		}
		else
		{
			osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
			ds->setStereo(false);
		}
		_cameraLeft->setAllowEventFocus(false);
		_cameraLeft->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//_cameraLeft->setRenderOrder(osg::Camera::PRE_RENDER);
		_cameraLeft->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		_cameraLeft->attach(osg::Camera::COLOR_BUFFER0,_leftTexture);
		_cameraLeft->attach(osg::Camera::DEPTH_BUFFER,_leftDepthTexture);
		_cameraLeft->setReferenceFrame(osg::Transform::RELATIVE_RF);

		_cameraDistLeft = new osg::Camera();
		_cameraDistLeft->setAllowEventFocus(false);
		_cameraDistLeft->setClearMask(0);
		//_cameraDistLeft->setClearColor(osg::Vec4(1.0,0,0,0));
		_cameraDistLeft->setRenderOrder(osg::Camera::POST_RENDER);
		_cameraDistLeft->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		_cameraDistLeft->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER);

		_cameraDistLeft->setGraphicsContext(_myInfo->myChannel->myWindow->gc);
		_cameraDistLeft->setViewport(
            new osg::Viewport(_myInfo->myChannel->left,
                    _myInfo->myChannel->bottom,_myInfo->myChannel->width/2.0,
                    _myInfo->myChannel->height));
		GLenum buffer =
				_myInfo->myChannel->myWindow->gc->getTraits()->doubleBuffer ?
                    GL_BACK : GL_FRONT;

		_cameraDistLeft->setDrawBuffer(buffer);
		_cameraDistLeft->setReadBuffer(buffer);
		_cameraDistLeft->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

		_cameraDistLeft->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		_cameraDistLeft->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_cameraDistLeft->setCullingActive(false);
		osg::Matrix ms, mt;
		mt.makeTranslate(osg::Vec3(0.5,0,0));
		ms.makeScale(osg::Vec3(2.0,1.0,1.0));
		_cameraDistLeft->setViewMatrix(mt*ms);

		CVRViewer::instance()->addSlave(_cameraDistLeft.get(),osg::Matrixd(),
            osg::Matrixd());

		_distVert = new osg::Shader(osg::Shader::VERTEX,meshTimewarpVertSrc);
		_distFrag = new osg::Shader(osg::Shader::FRAGMENT,meshFragSrc);
		_distProg = new osg::Program();
		_distProg->setName("Oculus Dist");
		_distProg->addShader(_distVert);
		_distProg->addShader(_distFrag);
		_cameraDistLeft->getOrCreateStateSet()->setAttribute(_distProg);
		_cameraDistLeft->getOrCreateStateSet()->addUniform(new osg::Uniform("EyeToSourceUVScale",osg::Vec2(scaleOffsetLeft[0].x,scaleOffsetLeft[0].y)));
		_cameraDistLeft->getOrCreateStateSet()->addUniform(new osg::Uniform("EyeToSourceUVOffset",osg::Vec2(scaleOffsetLeft[1].x,scaleOffsetLeft[1].y)));
		_cameraDistLeft->getOrCreateStateSet()->setTextureAttributeAndModes(0,_leftTexture,osg::StateAttribute::ON);
		/*_meshGeom = new osg::Geometry();
		osg::ref_ptr<osg::Vec2Array> points = new osg::Vec2Array(4);
		points->at(0) = osg::Vec2(1.0,1.0);
		points->at(1) = osg::Vec2(-1.0,1.0);
		points->at(2) = osg::Vec2(-1.0,-1.0);
		points->at(3) = osg::Vec2(1.0,-1.0);
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array(1);
		color->at(0) = osg::Vec4(1.0,1.0,1.0,1.0);
		_meshGeom->setVertexArray(points);
		_meshGeom->setColorArray(color);
		_meshGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		_meshGeom->setUseDisplayList(false);
		_meshGeom->setUseVertexBufferObjects(true);
		_meshGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

		_meshGeode = new osg::Geode();
		_meshGeode->setCullingActive(false);
		_meshGeode->addDrawable(_meshGeom);
		_cameraDistLeft->addChild(_meshGeode);*/

		_meshLeftGeom = new osg::Geometry();
		osg::ref_ptr<osg::Vec2Array> meshpoints = new osg::Vec2Array();
		osg::ref_ptr<osg::Vec2Array> texR = new osg::Vec2Array();
		osg::ref_ptr<osg::Vec2Array> texG = new osg::Vec2Array();
		osg::ref_ptr<osg::Vec2Array> texB = new osg::Vec2Array();
		osg::ref_ptr<osg::FloatArray> vf = new osg::FloatArray();
		osg::ref_ptr<osg::FloatArray> twf = new osg::FloatArray();

		ovrDistortionMesh meshData;
		ovrHmd_CreateDistortionMesh(_hmd, ovrEye_Left, _hmd->DefaultEyeFov[ovrEye_Left],_hmd->DistortionCaps, &meshData);
		osg::ref_ptr<osg::DrawElementsUShort> indices = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES,meshData.IndexCount);
		for(int i = 0; i < meshData.IndexCount; ++i)
		{
			indices->at(i) = meshData.pIndexData[i];
		}

		for(int i = 0; i < meshData.VertexCount; ++i)
		{
			/*std::cerr << "x: " << meshData.pVertexData[i].ScreenPosNDC.x << " y: " << meshData.pVertexData[i].ScreenPosNDC.y << std::endl;
			std::cerr << "rx: " << meshData.pVertexData[i].TanEyeAnglesR.x << " ry: " << meshData.pVertexData[i].TanEyeAnglesR.y << std::endl;
			std::cerr << "gx: " << meshData.pVertexData[i].TanEyeAnglesG.x << " gy: " << meshData.pVertexData[i].TanEyeAnglesG.y << std::endl;
			std::cerr << "bx: " << meshData.pVertexData[i].TanEyeAnglesB.x << " by: " << meshData.pVertexData[i].TanEyeAnglesB.y << std::endl;
			std::cerr << "timewarp: " << meshData.pVertexData[i].TimeWarpFactor << " vf: " << meshData.pVertexData[i].VignetteFactor << std::endl;*/
			meshpoints->push_back(osg::Vec2(meshData.pVertexData[i].ScreenPosNDC.x,meshData.pVertexData[i].ScreenPosNDC.y));
			texR->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesR.x,meshData.pVertexData[i].TanEyeAnglesR.y));
			texG->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesG.x,meshData.pVertexData[i].TanEyeAnglesG.y));
			texB->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesB.x,meshData.pVertexData[i].TanEyeAnglesB.y));
			vf->push_back(meshData.pVertexData[i].VignetteFactor);
			twf->push_back(meshData.pVertexData[i].TimeWarpFactor);
		}
		ovrHmd_DestroyDistortionMesh( &meshData );

		_meshLeftGeom->setVertexArray(meshpoints);
		_meshLeftGeom->setVertexAttribArray(4,texR,osg::Array::BIND_PER_VERTEX);
		_meshLeftGeom->setVertexAttribArray(5,texG,osg::Array::BIND_PER_VERTEX);
		_meshLeftGeom->setVertexAttribArray(6,texB,osg::Array::BIND_PER_VERTEX);
		_meshLeftGeom->setVertexAttribArray(7,vf,osg::Array::BIND_PER_VERTEX);
		_meshLeftGeom->setVertexAttribArray(8,twf,osg::Array::BIND_PER_VERTEX);
		_meshLeftGeom->setUseDisplayList(false);
		_meshLeftGeom->setUseVertexBufferObjects(true);
		_meshLeftGeom->addPrimitiveSet(indices);
		
		_meshLeftGeode = new osg::Geode();
		_meshLeftGeode->setCullingActive(false);
		_meshLeftGeode->addDrawable(_meshLeftGeom);
		//std::cerr << "Mesh size: " << meshpoints->size() << std::endl;

		ds = new osg::DisplaySettings();
		_cameraRight->setDisplaySettings(ds);

		CVRViewer::instance()->addSlave(_cameraRight.get(),osg::Matrixd(),
            osg::Matrixd());
		defaultCameraInit(_cameraRight.get());
		_cameraRight->setViewport(new osg::Viewport(0,0,recommenedrightSize.w,recommenedrightSize.h));

		renderer =
            dynamic_cast<osgViewer::Renderer*>(_cameraRight->getRenderer());
		if(!renderer)
		{
			std::cerr << "Error getting renderer pointer." << std::endl;
		}
		else
		{
			osg::DisplaySettings * ds =
                renderer->getSceneView(0)->getDisplaySettings();
			ds->setStereo(false);
		}
		_cameraRight->setAllowEventFocus(false);
		_cameraRight->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//_cameraRight->setRenderOrder(osg::Camera::PRE_RENDER);
		_cameraRight->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		_cameraRight->attach(osg::Camera::COLOR_BUFFER0,_rightTexture);
		_cameraRight->attach(osg::Camera::DEPTH_BUFFER,_rightDepthTexture);
		_cameraRight->setReferenceFrame(osg::Transform::RELATIVE_RF);

		_cameraDistRight = new osg::Camera();
		_cameraDistRight->setAllowEventFocus(false);
		_cameraDistRight->setClearMask(0);
		//_cameraDistRight->setClearColor(osg::Vec4(1.0,0,0,0));
		_cameraDistRight->setRenderOrder(osg::Camera::POST_RENDER);
		_cameraDistRight->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		_cameraDistRight->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER);

		_cameraDistRight->setGraphicsContext(_myInfo->myChannel->myWindow->gc);
		_cameraDistRight->setViewport(
            new osg::Viewport(_myInfo->myChannel->left+(_myInfo->myChannel->width/2.0),
                    _myInfo->myChannel->bottom,_myInfo->myChannel->width/2.0,
                    _myInfo->myChannel->height));

		_cameraDistRight->setDrawBuffer(buffer);
		_cameraDistRight->setReadBuffer(buffer);
		_cameraDistRight->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

		_cameraDistRight->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		_cameraDistRight->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_cameraDistRight->setCullingActive(false);
		mt.makeTranslate(osg::Vec3(-0.5,0,0));
		ms.makeScale(osg::Vec3(2.0,1.0,1.0));
		_cameraDistRight->setViewMatrix(mt*ms);

		CVRViewer::instance()->addSlave(_cameraDistRight.get(),osg::Matrixd(),
            osg::Matrixd());

		_cameraDistRight->getOrCreateStateSet()->setAttribute(_distProg);
		_cameraDistRight->getOrCreateStateSet()->addUniform(new osg::Uniform("EyeToSourceUVScale",osg::Vec2(scaleOffsetRight[0].x,scaleOffsetRight[0].y)));
		_cameraDistRight->getOrCreateStateSet()->addUniform(new osg::Uniform("EyeToSourceUVOffset",osg::Vec2(scaleOffsetRight[1].x,scaleOffsetRight[1].y)));
		_cameraDistRight->getOrCreateStateSet()->setTextureAttributeAndModes(0,_rightTexture,osg::StateAttribute::ON);
		//_cameraDistRight->addChild(_meshGeode);

		_meshRightGeom = new osg::Geometry();
		meshpoints = new osg::Vec2Array();
		texR = new osg::Vec2Array();
		texG = new osg::Vec2Array();
		texB = new osg::Vec2Array();
		vf = new osg::FloatArray();
		twf = new osg::FloatArray();

		ovrHmd_CreateDistortionMesh(_hmd, ovrEye_Right, _hmd->DefaultEyeFov[ovrEye_Right],_hmd->DistortionCaps, &meshData);
		indices = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES,meshData.IndexCount);
		for(int i = 0; i < meshData.IndexCount; ++i)
		{
			indices->at(i) = meshData.pIndexData[i];
		}

		for(int i = 0; i < meshData.VertexCount; ++i)
		{
			meshpoints->push_back(osg::Vec2(meshData.pVertexData[i].ScreenPosNDC.x,meshData.pVertexData[i].ScreenPosNDC.y));
			texR->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesR.x,meshData.pVertexData[i].TanEyeAnglesR.y));
			texG->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesG.x,meshData.pVertexData[i].TanEyeAnglesG.y));
			texB->push_back(osg::Vec2(meshData.pVertexData[i].TanEyeAnglesB.x,meshData.pVertexData[i].TanEyeAnglesB.y));
			vf->push_back(meshData.pVertexData[i].VignetteFactor);
			twf->push_back(meshData.pVertexData[i].TimeWarpFactor);
		}
		ovrHmd_DestroyDistortionMesh( &meshData );

		_meshRightGeom->setVertexArray(meshpoints);
		_meshRightGeom->setVertexAttribArray(4,texR,osg::Array::BIND_PER_VERTEX);
		_meshRightGeom->setVertexAttribArray(5,texG,osg::Array::BIND_PER_VERTEX);
		_meshRightGeom->setVertexAttribArray(6,texB,osg::Array::BIND_PER_VERTEX);
		_meshRightGeom->setVertexAttribArray(7,vf,osg::Array::BIND_PER_VERTEX);
		_meshRightGeom->setVertexAttribArray(8,twf,osg::Array::BIND_PER_VERTEX);
		_meshRightGeom->setUseDisplayList(false);
		_meshRightGeom->setUseVertexBufferObjects(true);
		_meshRightGeom->addPrimitiveSet(indices);
		
		_meshRightGeode = new osg::Geode();
		_meshRightGeode->setCullingActive(false);
		_meshRightGeode->addDrawable(_meshRightGeom);

		_cameraCallback = new OculusCameraCallback();
		_cameraCallback->hmd = _hmd;
		_cameraRight->setFinalDrawCallback(_cameraCallback);

		osg::Uniform * twUni = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"EyeRotationStart");
		_cameraCallback->_leftTWStart = twUni;
		_cameraDistLeft->getOrCreateStateSet()->addUniform(twUni);
		twUni = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"EyeRotationEnd");
		_cameraCallback->_leftTWEnd = twUni;
		_cameraDistLeft->getOrCreateStateSet()->addUniform(twUni);

		twUni = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"EyeRotationStart");
		_cameraCallback->_rightTWStart = twUni;
		_cameraDistRight->getOrCreateStateSet()->addUniform(twUni);
		twUni = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"EyeRotationEnd");
		_cameraCallback->_rightTWEnd = twUni;
		_cameraDistRight->getOrCreateStateSet()->addUniform(twUni);

		_fec = new OculusFrameEndCallback();
		_fec->hmd = _hmd;
		_fec->screenID = 0;

		CVRViewer::instance()->addPerContextFrameStartCallback(_fec);
	}
	else
	{
		std::cerr << "No Oculus found." << std::endl;
		ovr_Shutdown();
	}
}

void ScreenOculus::computeViewProj()
{
	if(!_hmd)
	{
		return;
	}

	_fec->screenID = _myInfo->myChannel->myWindow->gc->getState()->getContextID();

	//std::cerr << "View compute" << std::endl;

	// get current oculus camera adjustment
	ovrTrackingState ts = ovrHmd_GetTrackingState(_hmd, ovr_GetTimeInSeconds());

	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
		_cameraPos.x() = ts.LeveledCameraPose.Position.x;
		_cameraPos.y() = ts.LeveledCameraPose.Position.y;
		_cameraPos.z() = ts.LeveledCameraPose.Position.z;
	}

	_cameraCallback->frameTiming = ovrHmd_BeginFrameTiming(_hmd, 0);

	_cameraDistLeft->removeChildren(0,_cameraDistLeft->getNumChildren());
	_cameraDistLeft->addChild(_meshLeftGeode);
	_cameraDistRight->removeChildren(0,_cameraDistRight->getNumChildren());
	_cameraDistRight->addChild(_meshRightGeode);

	ovrPosef headPose[2];
	headPose[ovrEye_Left] = ovrHmd_GetEyePose(_hmd, ovrEye_Left);
	headPose[ovrEye_Right] = ovrHmd_GetEyePose(_hmd, ovrEye_Right);

	_cameraCallback->headPose[ovrEye_Left] = headPose[ovrEye_Left];
	_cameraCallback->headPose[ovrEye_Right] = headPose[ovrEye_Right];

	//std::cerr << "Left: x: " << headPose[ovrEye_Left].Position.x << " y: " << headPose[ovrEye_Left].Position.y << " z: " << headPose[ovrEye_Left].Position.z << std::endl;
	//std::cerr << "Left Rot x: " << headPose[ovrEye_Left].Orientation.x << " y: " << headPose[ovrEye_Left].Orientation.y << " z: " << headPose[ovrEye_Left].Orientation.z << " w: " << headPose[ovrEye_Left].Orientation.w << std::endl;
	//std::cerr << "Right: x: " << headPose[ovrEye_Right].Position.x << " y: " << headPose[ovrEye_Right].Position.y << " z: " << headPose[ovrEye_Right].Position.z << std::endl;
	//std::cerr << "Right Rot x: " << headPose[ovrEye_Right].Orientation.x << " y: " << headPose[ovrEye_Right].Orientation.y << " z: " << headPose[ovrEye_Right].Orientation.z << " w: " << headPose[ovrEye_Right].Orientation.w << std::endl;
	
	// create tracked body for estimated eye
	TrackerBase::TrackedBody tb;
	tb.x = (headPose[ovrEye_Left].Position.x - _cameraPos.x()) * 1000.0;
	tb.y = (headPose[ovrEye_Left].Position.y - _cameraPos.y()) * 1000.0;
	tb.z = (headPose[ovrEye_Left].Position.z - _cameraPos.z()) * 1000.0;
	tb.qx = headPose[ovrEye_Left].Orientation.x;
	tb.qy = headPose[ovrEye_Left].Orientation.y;
	tb.qz = headPose[ovrEye_Left].Orientation.z;
	tb.qw = headPose[ovrEye_Left].Orientation.w;

	// transform using tracking adjustment
	osg::Matrix headXForm = TrackingManager::instance()->getHeadTransformFromTrackedBody(_myInfo->myChannel->head,&tb);

    osg::Vec3d eyeLeft;
	//std::cerr << "eye: x: " << eyeLeft.x() << " y: " << eyeLeft.y() << " z: " << eyeLeft.z() << std::endl;

    //eyeLeft = defaultLeftEye(_myInfo->myChannel->head);
	eyeLeft = osg::Vec3d(-64.0 * _eyeSepMult / 2.0,0.0,0.0) * headXForm;

    //osg::Quat invViewerRot = TrackingManager::instance()->getHeadMat(
    //        _myInfo->myChannel->head).getRotate();
	osg::Quat invViewerRot = headXForm.getRotate();
    invViewerRot = invViewerRot.inverse();

    //make frustum
    float top, bottom, left, right;

    top = _near * _hmd->DefaultEyeFov[ovrEye_Left].UpTan;
    bottom = -_near * _hmd->DefaultEyeFov[ovrEye_Left].DownTan;
    right = _near * _hmd->DefaultEyeFov[ovrEye_Left].RightTan;
    left = -_near * _hmd->DefaultEyeFov[ovrEye_Left].LeftTan;

    _projLeft.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyeLeft);

    //make view
    _viewLeft = cameraTrans * osg::Matrix::rotate(invViewerRot)
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));

	tb.x = (headPose[ovrEye_Right].Position.x - _cameraPos.x()) * 1000.0;
	tb.y = (headPose[ovrEye_Right].Position.y - _cameraPos.y()) * 1000.0;
	tb.z = (headPose[ovrEye_Right].Position.z - _cameraPos.z()) * 1000.0;
	tb.qx = headPose[ovrEye_Right].Orientation.x;
	tb.qy = headPose[ovrEye_Right].Orientation.y;
	tb.qz = headPose[ovrEye_Right].Orientation.z;
	tb.qw = headPose[ovrEye_Right].Orientation.w;

	// transform using tracking adjustment
	headXForm = TrackingManager::instance()->getHeadTransformFromTrackedBody(_myInfo->myChannel->head,&tb);

	osg::Vec3d eyeRight;

    //eyeRight = defaultRightEye(_myInfo->myChannel->head);
	eyeRight = osg::Vec3d(64.0 * _eyeSepMult / 2.0,0.0,0.0) * headXForm;

	invViewerRot = headXForm.getRotate();
    invViewerRot = invViewerRot.inverse();

    //make frustum
    top = _near * _hmd->DefaultEyeFov[ovrEye_Right].UpTan;
    bottom = -_near * _hmd->DefaultEyeFov[ovrEye_Right].DownTan;
    right = _near * _hmd->DefaultEyeFov[ovrEye_Right].RightTan;
    left = -_near * _hmd->DefaultEyeFov[ovrEye_Right].LeftTan;

    _projRight.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    cameraTrans.makeTranslate(-eyeRight);

    //make view
    _viewRight = cameraTrans * osg::Matrix::rotate(invViewerRot)
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));
}

void ScreenOculus::updateCamera()
{
    _cameraLeft->setViewMatrix(_viewLeft);
	_cameraLeft->setProjectionMatrix(_projLeft);

	_cameraRight->setViewMatrix(_viewRight);
	_cameraRight->setProjectionMatrix(_projRight);
}

void ScreenOculus::setClearColor(osg::Vec4 color)
{
    _cameraLeft->setClearColor(color);
	_cameraRight->setClearColor(color);
}

ScreenInfo * ScreenOculus::findScreenInfo(osg::Camera * c)
{
    if(c == _cameraLeft.get() || c == _cameraRight.get())
    {
        return _myInfo;
    }
    return NULL;
}

void ScreenOculus::adjustViewportCoords(int & x, int & y)
{
    if(x > (_myInfo->myChannel->width / 2.0))
    {
        x = (int)(((float)x) - (_myInfo->myChannel->width / 2.0));
    }
    x *= 2;

    return;
}

void OculusCameraCallback::operator() (osg::RenderInfo &renderInfo) const
{
	// finish the render to texture
	glFinish();

	// Wait till time-warp point to reduce latency.
	ovr_WaitTillTime(frameTiming.TimewarpPointSeconds);

	ovrMatrix4f timeWarpMatrices[2];
	ovrHmd_GetEyeTimewarpMatrices(hmd,ovrEye_Left, headPose[ovrEye_Left],timeWarpMatrices);

	osg::Matrixf m;
	m.set((float*)&timeWarpMatrices[0]);
	_leftTWStart->set(m);
	m.set((float*)&timeWarpMatrices[1]);
	_leftTWEnd->set(m);
	
	ovrHmd_GetEyeTimewarpMatrices(hmd,ovrEye_Right, headPose[ovrEye_Right],timeWarpMatrices);

	m.set((float*)&timeWarpMatrices[0]);
	_rightTWStart->set(m);
	m.set((float*)&timeWarpMatrices[1]);
	_rightTWEnd->set(m);
}

void OculusFrameEndCallback::perContextCallback(int contextid, PCCType type) const
{
	if(contextid == screenID)
	{
		//std::cerr << "Frame finish" << std::endl;
		// wait for swap to finish, dont know if this should be done
		glFinish();

		ovrHmd_EndFrameTiming(hmd);
	}
}
