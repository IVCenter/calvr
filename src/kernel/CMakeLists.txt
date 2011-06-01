IF(WIN32)
	IF(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRKERNEL_LIBRARY)
	ELSE(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRKERNEL_LIBRARY_STATIC)
	ENDIF(DYNAMIC_LINKING)
ENDIF(WIN32)

SET(LIB_NAME kernel)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/ScreenConfig.h
    ${HEADER_PATH}/ScreenBase.h
    ${HEADER_PATH}/ScreenMono.h
    ${HEADER_PATH}/ScreenStereo.h
    ${HEADER_PATH}/ScreenLenticular.h
    ${HEADER_PATH}/ScreenMultiViewer.h
    ${HEADER_PATH}/MultiViewerCullVisitor.h
    ${HEADER_PATH}/ScreenMVMaster.h
    ${HEADER_PATH}/ScreenMVSimulator.h
    ${HEADER_PATH}/ScreenMultiViewer2.h
    ${HEADER_PATH}/MultiViewScreen.h
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
    ${HEADER_PATH}/PreCullVisitor.h
    ${HEADER_PATH}/PostCullVisitor.h
	${HEADER_PATH}/Export.h
)

SET(EXTRA_SOURCE)

IF(WIN32)
    FIND_PACKAGE(GLEW)
	IF(GLEW_FOUND)
	    SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${GLEW_INCLUDE_DIR})
		SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${GLEW_LIBRARY})
		SET(EXTRA_SOURCE ${EXTRA_SOURCE} ScreenMultiViewer.cpp)
	ENDIF(GLEW_FOUND)
ELSE(WIN32)
    SET(EXTRA_SOURCE ${EXTRA_SOURCE} ScreenMultiViewer.cpp)
ENDIF(WIN32)

IF(INTERLEAVER_FOUND)
    ADD_DEFINITIONS(-DWITH_INTERLEAVER)
    SET(EXTRA_SOURCE ${EXTRA_SOURCE} ScreenLenticular.cpp)
ENDIF(INTERLEAVER_FOUND)

SET(LIB_SRC_FILES
    ${EXTRA_SOURCE}
    ScreenConfig.cpp
    ScreenBase.cpp
    ScreenMono.cpp
    ScreenStereo.cpp
    MultiViewerCullVisitor.cpp
    ScreenMVMaster.cpp
    ScreenMVSimulator.cpp
    ScreenMultiViewer2.cpp
    MultiViewScreen.cpp
    ComController.cpp
    CVRViewer.cpp
    SceneManager.cpp
    FileHandler.cpp
    PluginManager.cpp
    PluginHelper.cpp
    InteractionManager.cpp
    Navigation.cpp
    CVRCullVisitor.cpp
    PreCullVisitor.cpp
    PostCullVisitor.cpp
)

SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES}
    ${MXML_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
)

SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES}
    ${MXML_LIBRARY}
    ${OSG_LIBRARIES}
)

SET(LIB_INTERNAL_LIBRARIES
    config
    util
    input
    menu
)

IF(INTERLEAVER_FOUND)
    SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${INTERLEAVER_INCLUDE_DIR})
    SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${INTERLEAVER_LIBRARY})
ENDIF(INTERLEAVER_FOUND)

SETUP_CORE_LIBRARY(${LIB_NAME})
