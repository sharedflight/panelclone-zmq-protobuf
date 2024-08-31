cmake_minimum_required( VERSION 3.2 )
link_directories(/usr/local/lib)
project( subscribe C CXX )
project( publish C CXX )

# See: https://stackoverflow.com/questions/73047153/protobuf-ld-undefined-symbol-internalmetadata
add_definitions(-DNDEBUG)

#find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq)

find_package( absl REQUIRED )

# Compile the protobuf file and make sure the compiled
# files are accessible. The cpp and h files are both put into
# the binary dir.
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS event.proto)

add_executable( subscribe subscribe.cpp ${PROTO_SRCS} ${PROTO_HDRS})
add_executable( publish publish.cpp ${PROTO_SRCS} ${PROTO_HDRS})

target_compile_features(subscribe PRIVATE cxx_std_17)
target_compile_features(publish PRIVATE cxx_std_17)

target_link_libraries( subscribe
    cppzmq
    ${Protobuf_LIBRARIES}
    absl::log_internal_message
    absl::log_internal_check_op
)

target_link_libraries( publish 
    cppzmq
    ${Protobuf_LIBRARIES}
    absl::log_internal_message
    absl::log_internal_check_op
)

install(
    TARGETS subscribe publish
    DESTINATION ${PROJECT_SOURCE_DIR}
)
