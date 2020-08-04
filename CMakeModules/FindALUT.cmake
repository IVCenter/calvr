FIND_PATH(ALUT_INCLUDE_DIR AL/alut.h
  PATHS
  $ENV{ALUTDIR}
  $ENV{ALUT_PATH}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(ALUT_INCLUDE_DIR AL/alut.h
  PATHS
  /home/covise/covise/extern_libs/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(ALUT_LIBRARY 
  NAMES alut libalut
  PATHS
    /home/covise/covise/extern_libs
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(ALUT_FOUND "NO")
IF(ALUT_LIBRARY AND ALUT_INCLUDE_DIR)
  SET(ALUT_FOUND "YES")
ENDIF(ALUT_LIBRARY AND ALUT_INCLUDE_DIR)

