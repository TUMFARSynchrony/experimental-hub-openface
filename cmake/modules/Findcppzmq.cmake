SET(cppzmq_INCLUDE_SEARCH_PATHS
  $ENV{cppzmq_HOME}
  $ENV{cppzmq_HOME}/include
  /opt/cppzmq/include
  /usr/local/include/zmq
  /usr/include/zmq
  /usr/local/include/zmq
  /usr/include/zmq
  /usr/include/x86_64-linux-gnu
  /usr/local/include
  /usr/include
  /usr/local/opt/zmq/include
)

SET(cppzmq_LIB_SEARCH_PATHS
        $ENV{cppzmq}cd
        $ENV{cppzmq}/lib
        $ENV{cppzmq_HOME}
        $ENV{cppzmq_HOME}/lib
        /opt/libzmq/lib
        /usr/local/libzmq
        /usr/local/lib
        /lib/libzmq
        /lib64/
        /lib/
        /usr/lib/libzmq
        /usr/lib/x86_64-linux-gnu
        /usr/lib64
        /usr/lib
		/usr/local/opt/libzmq/lib
 )

FIND_PATH(cppzmq_INCLUDE_DIR NAMES zmq.h PATHS ${cppzmq_INCLUDE_SEARCH_PATHS} NO_DEFAULT_PATH)
FIND_LIBRARY(cppzmq_LIB NAMES libzmq.a PATHS ${cppzmq_LIB_SEARCH_PATHS}  NO_DEFAULT_PATH)

SET(cppzmq_FOUND ON)
SET(cppzmq_INCLUDE_FOUND ON)

# Check include files
IF(NOT cppzmq_INCLUDE_DIR)
    SET(cppzmq_INCLUDE_FOUND OFF)
    MESSAGE(STATUS "Could not find cppzmq include")
ENDIF()

# Check libraries
IF(NOT cppzmq_LIB)
    SET(cppzmq_FOUND OFF)
    MESSAGE(STATUS "Could not find cppzmq lib. Turning cppzmq_FOUND off")
ENDIF()

IF (cppzmq_FOUND)
  IF (NOT cppzmq_FIND_QUIETLY)
    MESSAGE(STATUS "Found cppzmq libraries: ${cppzmq_LIB}")
    MESSAGE(STATUS "Found cppzmq include: ${cppzmq_INCLUDE_DIR}")
  ENDIF (NOT cppzmq_FIND_QUIETLY)
ELSE (cppzmq_FOUND)
  IF (cppzmq_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find cppzmq")
  ENDIF (cppzmq_FIND_REQUIRED)
ENDIF (cppzmq_FOUND)

MARK_AS_ADVANCED(
    cppzmq_INCLUDE_DIR
    cppzmq_LIB
    cppzmq
)