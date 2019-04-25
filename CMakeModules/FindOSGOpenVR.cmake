IF (DEFINED ENV{OSGOPENVR_HOME})
    SET(OSGOPENVR_HOME "$ENV{OSGOPENVR_HOME}")
ENDIF()
SET(OSGOPENVR_HOME
    "${OSGOPENVR_HOME}"
    CACHE
    PATH
    "Root directory to search for OsgOpenVRViewer")


FIND_PATH(OSGOPENVR_INCLUDE_DIR openvrdevice.h
  PATHS
  ${OSGOPENVR_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include Include
)

FIND_PATH(OSGOPENVR_INCLUDE_DIR openvrdevice.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OSGOPENVR_LIBRARY 
  NAMES OsgOpenVR
  PATHS ${OSGOPENVR_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib
)
FIND_LIBRARY(OSGOPENVR_LIBRARY 
  NAMES OsgOpenVR
  PATHS
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib
)

SET(OSGOPENVR_FOUND "NO")
IF(OSGOPENVR_LIBRARY AND OSGOPENVR_INCLUDE_DIR)
  SET(OSGOPENVR_FOUND "YES")
ENDIF(OSGOPENVR_LIBRARY AND OSGOPENVR_INCLUDE_DIR)

