FIND_PATH(SPACENAV_INCLUDE_DIR spnav.h
  PATHS
  $ENV{MXML_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(SPACENAV_INCLUDE_DIR spnav.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(SPACENAV_LIBRARY 
  NAMES spnav
  PATHS $ENV{SPACENAV_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(SPACENAV_LIBRARY 
  NAMES spnav
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

SET(SPACENAV_FOUND "NO")
IF(SPACENAV_LIBRARY AND SPACENAV_INCLUDE_DIR)
  SET(SPACENAV_FOUND "YES")
ENDIF(SPACENAV_LIBRARY AND SPACENAV_INCLUDE_DIR)

