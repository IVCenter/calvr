FIND_PATH(VRPN_INCLUDE_DIR vrpn_Configure.h
  PATHS
  $ENV{VRPN_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(VRPN_INCLUDE_DIR vrpn_Configure.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(VRPN_LIBRARY 
  NAMES vrpn
  PATHS $ENV{VRPN_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(VRPN_LIBRARY 
  NAMES vrpn
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

SET(VRPN_FOUND "NO")
IF(VRPN_LIBRARY AND VRPN_INCLUDE_DIR)
  SET(VRPN_FOUND "YES")
ENDIF(VRPN_LIBRARY AND VRPN_INCLUDE_DIR)

