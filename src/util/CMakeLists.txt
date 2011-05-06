SET(LIB_NAME util)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/CVRSocket.h
    ${HEADER_PATH}/Intersection.h
    ${HEADER_PATH}/MultiListenSocket.h
    ${HEADER_PATH}/DepthPartitionNode.h
    ${HEADER_PATH}/DistanceAccumulator.h
    ${HEADER_PATH}/OsgPrint.h
)

SET(EXTRA_SOURCE)

ADD_LIBRARY(${LIB_NAME}
    ${CALVR_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ${EXTRA_SOURCE}
    CVRSocket.cpp
    Intersection.cpp
    MultiListenSocket.cpp
    DepthPartitionNode.cpp
    DistanceAccumulator.cpp
    OsgPrint.cpp
)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)
