FIND_PATH(VIRVO_INCLUDE_DIR virvo/vvvirvo.h
  PATHS
  $ENV{VIRVO_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(VIRVO_INCLUDE_DIR virvo/vvvirvo.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(VIRVO_LIBRARY 
  NAMES virvo
  PATHS $ENV{VIRVO_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(VIRVO_LIBRARY 
  NAMES virvo
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

SET(VIRVO_FOUND "NO")
IF(VIRVO_LIBRARY AND VIRVO_INCLUDE_DIR)
  SET(VIRVO_FOUND "YES")
ENDIF(VIRVO_LIBRARY AND VIRVO_INCLUDE_DIR)

