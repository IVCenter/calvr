FIND_PATH(LIBRK_INCLUDE_DIR rk.h rkgs.h
  PATHS
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    PATH_SUFFIXES include
)

FIND_LIBRARY(LIBRK_LIBRARY 
  NAMES rk
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

SET(LIBRK_FOUND "NO")
IF(LIBRK_LIBRARY AND LIBRK_INCLUDE_DIR)
  SET(LIBRK_FOUND "YES")
ENDIF(LIBRK_LIBRARY AND LIBRK_INCLUDE_DIR)

