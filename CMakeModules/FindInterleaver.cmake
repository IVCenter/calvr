FIND_PATH(INTERLEAVER_INCLUDE_DIR coInterleaver.h
  PATHS
  $ENV{INTERLEAVER_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(INTERLEAVER_INCLUDE_DIR coInterleaver.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(INTERLEAVER_LIBRARY 
  NAMES coInterleaver
  PATHS $ENV{INTERLEAVER_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(INTERLEAVER_LIBRARY 
  NAMES coInterleaver
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

SET(INTERLEAVER_FOUND "NO")
IF(INTERLEAVER_LIBRARY AND INTERLEAVER_INCLUDE_DIR)
  SET(INTERLEAVER_FOUND "YES")
ENDIF(INTERLEAVER_LIBRARY AND INTERLEAVER_INCLUDE_DIR)

