FIND_PATH(OPENAL_INCLUDE_DIR AL/al.h
  PATHS
  $ENV{OPENAL_HOME}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(OPENAL_INCLUDE_DIR AL/al.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OPENAL_LIBRARY 
  NAMES openal libopenal
  PATHS
    $ENV{OPENAL_HOME}
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(OPENAL_FOUND "NO")
IF(OPENAL_LIBRARY AND OPENAL_INCLUDE_DIR)
  SET(OPENAL_FOUND "YES")
ENDIF(OPENAL_LIBRARY AND OPENAL_INCLUDE_DIR)

