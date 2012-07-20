FIND_PATH(OMICRON_INCLUDE_DIR connector/omicronConnectorClient.h
  PATHS
  $ENV{OMICRON_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(OMICRON_INCLUDE_DIR connector/omicronConnectorClient.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)


SET(OMICRON_FOUND "NO")
IF(OMICRON_INCLUDE_DIR)
  SET(OMICRON_FOUND "YES")
ENDIF(OMICRON_INCLUDE_DIR)

