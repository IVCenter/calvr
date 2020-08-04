FIND_PATH(OSSIM_INCLUDE_DIR ossim.h
  PATHS
  $ENV{OSSIM_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(OSSIM_INCLUDE_DIR ossim.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OSSIM_LIBRARY 
  NAMES ossim
  PATHS $ENV{OSSIM_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(OSSIM_LIBRARY 
  NAMES ossim
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

SET(OSSIM_FOUND "NO")
IF(OSSIM_LIBRARY AND OSSIM_INCLUDE_DIR)
  SET(OSSIM_FOUND "YES")
ENDIF(OSSIM_LIBRARY AND OSSIM_INCLUDE_DIR)

