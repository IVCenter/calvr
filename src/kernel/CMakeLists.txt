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
)

SET(EXTRA_SOURCE)

IF(INTERLEAVER_FOUND)
    ADD_DEFINITIONS(-DWITH_INTERLEAVER)
    SET(EXTRA_SOURCE ${EXTRA_SOURCE} ScreenLenticular.cpp)
ENDIF(INTERLEAVER_FOUND)

ADD_LIBRARY(${LIB_NAME}
    ${CALVR_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ${EXTRA_SOURCE}
    ScreenConfig.cpp
    ScreenBase.cpp
    ScreenMono.cpp
    ScreenStereo.cpp
    ScreenMultiViewer.cpp
    MultiViewerCullVisitor.cpp
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

IF(INTERLEAVER_FOUND)
    INCLUDE_DIRECTORIES(${INTERLEAVER_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${INTERLEAVER_LIBRARY})
ENDIF(INTERLEAVER_FOUND)

IF(WIN32)
    FIND_PACKAGE(Winsock2)
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${WINSOCK2_LIBRARY})
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${MXML_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${MXML_LIBRARY})
INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)
