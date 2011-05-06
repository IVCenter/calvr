FIND_PATH(GLEW_INCLUDE_DIR GL/glew.h
  PATHS
  $ENV{GLEW_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(GLEW_INCLUDE_DIR GL/glew.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(GLEW_LIBRARY 
  NAMES GLEW
  PATHS $ENV{GLEW_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(GLEW_LIBRARY 
  NAMES GLEW
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

SET(GLEW_FOUND "NO")
IF(GLEW_LIBRARY AND GLEW_INCLUDE_DIR)
  SET(GLEW_FOUND "YES")
ENDIF(GLEW_LIBRARY AND GLEW_INCLUDE_DIR)
