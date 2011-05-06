FIND_PATH(MXML_INCLUDE_DIR mxml.h
  PATHS
  $ENV{MXML_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(MXML_INCLUDE_DIR mxml.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(MXML_LIBRARY 
  NAMES mxml mxml1 mxml1.4
  PATHS $ENV{MXML_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(MXML_LIBRARY 
  NAMES mxml mxml1 mxml1.4
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

SET(MXML_FOUND "NO")
IF(MXML_LIBRARY AND MXML_INCLUDE_DIR)
  SET(MXML_FOUND "YES")
ENDIF(MXML_LIBRARY AND MXML_INCLUDE_DIR)

