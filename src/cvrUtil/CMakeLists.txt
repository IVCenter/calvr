IF(WIN32)
	IF(CALVR_WIN_DYNAMIC)
		ADD_DEFINITIONS(-DCVRUTIL_LIBRARY)
	ELSE(CALVR_WIN_DYNAMIC)
		ADD_DEFINITIONS(-DCVRUTIL_LIBRARY_STATIC)
	ENDIF(CALVR_WIN_DYNAMIC)
ENDIF(WIN32)

SET(LIB_NAME cvrUtil)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/CVRSocket.h
    ${HEADER_PATH}/Intersection.h
    ${HEADER_PATH}/MultiListenSocket.h
    ${HEADER_PATH}/DepthPartitionNode.h
    ${HEADER_PATH}/DistanceAccumulator.h
    ${HEADER_PATH}/OsgPrint.h
    ${HEADER_PATH}/ComputeBoundingBoxVisitor.h
    ${HEADER_PATH}/CVRMulticastSocket.h
    ${HEADER_PATH}/OsgMath.h
    ${HEADER_PATH}/LocalToWorldVisitor.h
    ${HEADER_PATH}/TextureVisitors.h
    ${HEADER_PATH}/PointsNode.h
    ${HEADER_PATH}/Export.h
)

IF(WIN32)
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS} ${HEADER_PATH}/TimeOfDay.h)
ENDIF(WIN32)

IF(ANDROID)
    SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS}
    ${HEADER_PATH}/LightingEstimator.h
    ${HEADER_PATH}/AndroidHelper.h
    ${HEADER_PATH}/AndroidPreloadPlugins.h
    ${HEADER_PATH}/glesDrawable.h
    ${HEADER_PATH}/ARCoreManager.h
    ${HEADER_PATH}/arcore_c_api.h
    ${HEADER_PATH}/OsgGlesMath.h
    )
ENDIF(ANDROID)

SET(EXTRA_SOURCE)

IF(WIN32)
SET(EXTRA_SOURCE ${EXTRA_SOURCE} TimeOfDay.cpp)
ENDIF(WIN32)

IF(ANDROID)
    SET(EXTRA_SOURCE ${EXTRA_SOURCE}
    LightingEstimator.cpp
    AndroidHelper.cpp
    AndroidPreloadPlugins.cpp
    glesDrawable.cpp
    ARCoreManager.cpp
    OsgGlesMath.cpp
    )
ENDIF(ANDROID)

SET(LIB_SRC_FILES
    ${EXTRA_SOURCE}
    CVRSocket.cpp
    Intersection.cpp
    MultiListenSocket.cpp
    DepthPartitionNode.cpp
    DistanceAccumulator.cpp
    OsgPrint.cpp
    ComputeBoundingBoxVisitor.cpp
    CVRMulticastSocket.cpp
    OsgMath.cpp
    LocalToWorldVisitor.cpp
    TextureVisitors.cpp
    PointsNode.cpp
)

SET(LIB_EXTERNAL_INCLUDES
    ${OSG_INCLUDE_DIR}
)

SET(LIB_EXTERNAL_LIBRARIES
    ${OSG_LIBRARIES}
)

#SET(LIB_INTERNAL_LIBRARIES
#)

IF(ANDROID AND ${USE_OPENCV})
      SET(LIB_EXTERNAL_INCLUDES ${LIB_EXTERNAL_INCLUDES} ${OpenCV_INCLUDE_DIR})
      SET(LIB_EXTERNAL_LIBRARIES ${LIB_EXTERNAL_LIBRARIES} ${OpenCV_DIR})
ENDIF()

SETUP_CORE_LIBRARY(${LIB_NAME})

#IF(ANDROID AND ${USE_OPENCV})
#      set(OpenCV_DIR "${EXT_PROJ_DIR}/opencv-3.4.3/build/${ANDROID_ABI}/sdk/native/jni")
#      find_package(OpenCV REQUIRED)
#      message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
#      target_link_libraries(cvrUtil ${OpenCV_LIBS})
#ENDIF()