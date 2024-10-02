cmake_minimum_required(VERSION 3.22)

set(PANELCLONEPROTOBUF "${CMAKE_SOURCE_DIR}")

set(GENERATED_CODE_DIR ${PANELCLONEPROTOBUF})
set(PROTO_SRCS ${GENERATED_CODE_DIR}/event.pb.cc)
set(PROTO_HDRS ${GENERATED_CODE_DIR}/event.pb.h)
if (WIN32)
  set(PROTOC  ../build/bin/protoc)
  set(PROTO_DIR  ../build/bin)
else()
  set(PROTOC ${CMAKE_SOURCE_DIR}/protoc-28.2-osx-universal_binary/bin/protoc)
  set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (NOT WIN32)
  add_custom_command(
      OUTPUT ${PROTO_HDRS} ${PROTO_SRCS} 
     COMMAND ${PROTOC} --proto_path ${PROTO_DIR} event.proto -I ${PANELCLONEPROTOBUF} --cpp_out=${GENERATED_CODE_DIR}
      DEPENDS protoc ${PANELCLONEPROTOBUF}/event.proto
     )
else() 
#   add_custom_command(OUTPUT ${PROTO_HDRS} ${PROTO_SRCS} 
#     COMMAND ${PROTOC} --proto_path ${PROTO_DIR} event.proto -I ${PANELCLONEPROTOBUF} --cpp_out=${GENERATED_CODE_DIR}
#   )
endif()



