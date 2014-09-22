FIND_PATH(OVR_INCLUDE_DIR OVR.h
  PATHS
  $ENV{OVR_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include Include
)

FIND_PATH(OVR_INCLUDE_DIR OVR.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OVR_LIBRARY 
  NAMES ovr ovr64
  PATHS $ENV{OVR_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(OVR_LIBRARY 
  NAMES ovr ovr64
  PATHS
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(OVR_FOUND "NO")
IF(OVR_LIBRARY AND OVR_INCLUDE_DIR)
  SET(OVR_FOUND "YES")
ENDIF(OVR_LIBRARY AND OVR_INCLUDE_DIR)

