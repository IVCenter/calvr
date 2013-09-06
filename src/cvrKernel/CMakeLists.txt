IF(WIN32)
	IF(CALVR_WIN_DYNAMIC)
		ADD_DEFINITIONS(-DCVRKERNEL_LIBRARY)
	ELSE(CALVR_WIN_DYNAMIC)
		ADD_DEFINITIONS(-DCVRKERNEL_LIBRARY_STATIC)
	ENDIF(CALVR_WIN_DYNAMIC)
ENDIF(WIN32)

SET(LIB_NAME cvrKernel)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(SCREEN_HEADER_PATH ${HEADER_PATH}/Screens)
SET(STATE_HEADER_PATH ${HEADER_PATH}/States)
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/CalVR.h
    ${HEADER_PATH}/ScreenConfig.h
    ${SCREEN_HEADER_PATH}/ScreenBase.h
    ${SCREEN_HEADER_PATH}/ScreenMono.h
    ${SCREEN_HEADER_PATH}/ScreenStereo.h
    ${SCREEN_HEADER_PATH}/ScreenInterlacedTopBottom.h
    ${SCREEN_HEADER_PATH}/ScreenLenticular.h
    ${SCREEN_HEADER_PATH}/ScreenMVShader.h
    ${SCREEN_HEADER_PATH}/ScreenMVCullVisitor.h
    ${SCREEN_HEADER_PATH}/ScreenMVMaster.h
    ${SCREEN_HEADER_PATH}/ScreenMVSimulator.h
    ${SCREEN_HEADER_PATH}/ScreenMVZones.h
    ${SCREEN_HEADER_PATH}/ScreenFixedViewer.h
    ${SCREEN_HEADER_PATH}/ScreenHMD.h
    ${SCREEN_HEADER_PATH}/ScreenMVStencil.h
    ${HEADER_PATH}/StateManager.h
    ${STATE_HEADER_PATH}/CvrState.h
    ${STATE_HEADER_PATH}/MetadataState.h
    ${STATE_HEADER_PATH}/SpatialState.h
    ${HEADER_PATH}/ComController.h
    ${HEADER_PATH}/CVRViewer.h
    ${HEADER_PATH}/SceneManager.h
    ${HEADER_PATH}/FileHandler.h
    ${HEADER_PATH}/CVRPlugin.h
    ${HEADER_PATH}/PluginManager.h
    ${HEADER_PATH}/PluginHelper.h
    ${HEADER_PATH}/InteractionManager.h
    ${HEADER_PATH}/Navigation.h
    ${HEADER_PATH}/CVRCullVisitor.h
    ${HEADER_PATH}/ThreadedLoader.h
    ${HEADER_PATH}/SceneObject.h
    ${HEADER_PATH}/TiledWallSceneObject.h
    ${HEADER_PATH}/InteractionEvent.h
    ${HEADER_PATH}/CVRStatsHandler.h
    ${HEADER_PATH}/Export.h
    ${HEADER_PATH}/DatabaseHandler.h
)

SET(EXTRA_SOURCE)

IF(WIN32)
    FIND_PACKAGE(GLEW)
	IF(GLEW_FOUND)
	    SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${GLEW_INCLUDE_DIR})
		SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${GLEW_LIBRARY})
		SET(EXTRA_SOURCE ${EXTRA_SOURCE} Screens/ScreenMVShader.cpp)
		ADD_DEFINITIONS(-DWITH_SCREEN_MULTI_VIEWER)
	ENDIF(GLEW_FOUND)
ELSE(WIN32)
    SET(EXTRA_SOURCE ${EXTRA_SOURCE} Screens/ScreenMVShader.cpp)
	ADD_DEFINITIONS(-DWITH_SCREEN_MULTI_VIEWER)
ENDIF(WIN32)

IF(INTERLEAVER_FOUND)
    FIND_PACKAGE(GLEW)
    IF(GLEW_FOUND)
	ADD_DEFINITIONS(-DWITH_INTERLEAVER)
	SET(EXTRA_SOURCE ${EXTRA_SOURCE} ScreenLenticular.cpp)
    ENDIF(GLEW_FOUND)
ENDIF(INTERLEAVER_FOUND)

SET(LIB_SRC_FILES
    ${EXTRA_SOURCE}
    CalVR.cpp
    ScreenConfig.cpp
    Screens/ScreenBase.cpp
    Screens/ScreenMono.cpp
    Screens/ScreenStereo.cpp
    Screens/ScreenInterlacedTopBottom.cpp
    Screens/ScreenMVCullVisitor.cpp
    Screens/ScreenMVMaster.cpp
    Screens/ScreenMVSimulator.cpp
    Screens/ScreenMVZones.cpp
    Screens/ScreenFixedViewer.cpp
    Screens/ScreenHMD.cpp
    Screens/ScreenMVStencil.cpp
    StateManager.cpp
    States/CvrState.cpp
    States/MetadataState.cpp
    States/SpatialState.cpp
    ComController.cpp
    CVRViewer.cpp
    SceneManager.cpp
    FileHandler.cpp
    PluginManager.cpp
    PluginHelper.cpp
    InteractionManager.cpp
    Navigation.cpp
    CVRCullVisitor.cpp
    ThreadedLoader.cpp
    SceneObject.cpp
    TiledWallSceneObject.cpp
    InteractionEvent.cpp
    CVRStatsHandler.cpp
    DatabaseHandler.cpp
)

SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES}
    ${MXML_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
    ${CDBAPP_INCLUDE_DIR}
)

SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES}
    ${MXML_LIBRARY}
    ${OSG_LIBRARIES}
    ${CDBAPP_LIBRARY}
)

SET(LIB_INTERNAL_LIBRARIES
    config
    util
    input
    menu
)

IF(INTERLEAVER_FOUND)
    IF(GLEW_FOUND)
	SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${INTERLEAVER_INCLUDE_DIR})
	SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${GLEW_INCLUDE_DIR})
	SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${INTERLEAVER_LIBRARY})
	SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${GLEW_LIBRARY})
    ENDIF(GLEW_FOUND)
ENDIF(INTERLEAVER_FOUND)

SETUP_CORE_LIBRARY(${LIB_NAME})
