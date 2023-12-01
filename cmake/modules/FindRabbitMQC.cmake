SET(Rabbitmqc_INCLUDE_SEARCH_PATHS
  $ENV{Rabbitmqc_HOME}
  $ENV{Rabbitmqc_HOME}/include
  /opt/rabbitmq-c/include
  /usr/local/include/rabbitmq-c
  /usr/include/rabbitmq-c
  /usr/local/include/rabbitmq-c
  /usr/include/rabbitmq-c
  /usr/include/x86_64-linux-gnu
  /usr/local/include
  /usr/include
  /usr/local/opt/rabbitmq-c/include
)

SET(Rabbitmqc_LIB_SEARCH_PATHS
        $ENV{Rabbitmqc}cd
        $ENV{Rabbitmqc}/lib
        $ENV{Rabbitmqc_HOME}
        $ENV{Rabbitmqc_HOME}/lib
        /opt/librabbitmq/lib
        /usr/local/lib64
        /usr/local/lib
        /lib/librabbitmq
        /lib64/
        /lib/
        /usr/lib/librabbitmq
        /usr/lib/x86_64-linux-gnu
        /usr/lib64
        /usr/lib
		/usr/local/opt/librabbitmq/lib
 )

FIND_PATH(Rabbitmqc_INCLUDE_DIR NAMES amqp.h PATHS ${Rabbitmqc_INCLUDE_SEARCH_PATHS} NO_DEFAULT_PATH)
FIND_LIBRARY(Rabbitmqc_LIB NAMES rabbitmq PATHS ${Rabbitmqc_LIB_SEARCH_PATHS}  NO_DEFAULT_PATH)

SET(Rabbitmqc_FOUND ON)
SET(Rabbitmqc_INCLUDE_FOUND ON)

# Check include files
IF(NOT Rabbitmqc_INCLUDE_DIR)
    SET(Rabbitmqc_INCLUDE_FOUND OFF)
    MESSAGE(STATUS "Could not find rabbitmq-c include")
ENDIF()

# Check libraries
IF(NOT Rabbitmqc_LIB)
    SET(Rabbitmqc_FOUND OFF)
    MESSAGE(STATUS "Could not find rabbitmq-c lib. Turning Rabbitmqc_FOUND off")
ENDIF()

IF (Rabbitmqc_FOUND)
  IF (NOT Rabbitmqc_FIND_QUIETLY)
    MESSAGE(STATUS "Found Rabbitmqc libraries: ${Rabbitmqc_LIB}")
    MESSAGE(STATUS "Found Rabbitmqc include: ${Rabbitmqc_INCLUDE_DIR}")
  ENDIF (NOT Rabbitmqc_FIND_QUIETLY)
ELSE (Rabbitmqc_FOUND)
  IF (Rabbitmqc_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Rabbitmqc")
  ENDIF (Rabbitmqc_FIND_REQUIRED)
ENDIF (Rabbitmqc_FOUND)

MARK_AS_ADVANCED(
    Rabbitmqc_INCLUDE_DIR
    Rabbitmqc_LIB
    Rabbitmqc
)