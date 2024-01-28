# Local libraries
include_directories(${LandmarkDetector_SOURCE_DIR}/include)

add_executable(MQAUExtractor MQAUExtractor.cpp base64.cpp utils.cpp)

find_package(Rabbitmqc REQUIRED)
INCLUDE_DIRECTORIES(${Rabbitmqc_INCLUDE_DIR})

target_link_libraries(MQAUExtractor LandmarkDetector)
target_link_libraries(MQAUExtractor FaceAnalyser)
target_link_libraries(MQAUExtractor GazeAnalyser)
target_link_libraries(MQAUExtractor Utilities)
target_link_libraries(MQAUExtractor ${Rabbitmqc_LIB})

install (TARGETS MQAUExtractor DESTINATION bin)