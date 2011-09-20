IF(WIN32)
	IF(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRUTIL_LIBRARY)
	ELSE(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRUTIL_LIBRARY_STATIC)
	ENDIF(DYNAMIC_LINKING)
ENDIF(WIN32)

SET(LIB_NAME util)
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
    ${HEADER_PATH}/Export.h
)

IF(WIN32)
SET(LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS} ${HEADER_PATH}/TimeOfDay.h)
ENDIF(WIN32)

SET(EXTRA_SOURCE)

IF(WIN32)
SET(EXTRA_SOURCE ${EXTRA_SOURCE} TimeOfDay.cpp)
ENDIF(WIN32)

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
)

SET(LIB_EXTERNAL_INCLUDES
    ${OSG_INCLUDE_DIR}
)

SET(LIB_EXTERNAL_LIBRARIES
    ${OSG_LIBRARIES}
)

#SET(LIB_INTERNAL_LIBRARIES
#)

SETUP_CORE_LIBRARY(${LIB_NAME})
