cmake_minimum_required( VERSION 3.2 )
link_directories(/usr/local/lib)
project( subscribe C CXX )
project( publish C CXX )
project( plugin C CXX )

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

if(WIN32)
    set(PLAT_SHORT "win64")
    set(PLAT_LONG "win-64")
    set(PLAT_ONLY "win")
    set(PLUGIN_BIN_OUTDIR "win_x64")
elseif(APPLE)
    set(PLAT_SHORT "mac64")
    set(PLAT_LONG "mac-64")
    set(PLAT_ONLY "mac")
    set(PLUGIN_BIN_OUTDIR "mac_x64")
else()
    set(PLAT_SHORT "lin64")
    set(PLAT_LONG "linux-64")
    set(PLAT_ONLY "linux")
    set(PLUGIN_BIN_OUTDIR "lin_x64")
endif()

set(LIBACFUTILS "${CMAKE_SOURCE_DIR}/../panelmanager/src/libacfutils")
set(XPSDK "${LIBACFUTILS}/SDK")
set(ZLIB "${LIBACFUTILS}/zlib/zlib-${PLAT_LONG}")

set(ALL_SRC
    plugin.cpp
    pluginWin.cpp
    DatarefPublisher.cpp
    DatarefPublisher.hpp
    Dref.hpp
    Dref.cpp
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
    )
LIST(SORT ALL_SRC)

add_library(plugin SHARED ${ALL_SRC})

include_directories(plugin PUBLIC
    "${XPSDK}/CHeaders/XPLM"
    "${XPSDK}/CHeaders/Widgets"
    "${XPSDK}/CHeaders/Wrappers"
    "${LIBACFUTILS}/src"
    )

add_definitions(-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64)
# Needed for C11's gmtime_s
add_definitions(-D__STDC_WANT_LIB_EXT1__=1)

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM302=1)
add_definitions(-DXPLM_DEPRECATED)
add_definitions(-DGLEW_BUILD=GLEW_STATIC -DGLEW_MX)
if(WIN32)
    add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
    add_definitions(-D_USRDLL -D_WIN32 -DWIN32 -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0600)
elseif(APPLE)
    add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
else()
    add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()

execute_process(COMMAND libacfutils/pkg-config-deps ${PLAT_LONG} --cflags
    OUTPUT_VARIABLE DEP_CFLAGS)
string(REGEX REPLACE "\n$" "" DEP_CFLAGS "${DEP_CFLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEP_CFLAGS}")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} -DDEBUG -g \
    -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -O0 -g")

# linking
# Careful: order matters in target_link_libraries. Dependencies must be
# ordered as first symbol callers and then as symbol providers (i.e.
# libpng16 depends on libz, so linking must be first: libpng, then libz).
#link_libraries(m)

if(WIN32)
    find_library(XPLM_LIBRARY XPLM_64 "${XPSDK}/Libraries/Win")
    find_library(XPWIDGETS_LIBRARY XPWidgets_64 "${XPSDK}/Libraries/Win")
elseif(APPLE)
    find_library(XPLM_LIBRARY XPLM "${XPSDK}/Libraries/Mac")
    find_library(XPWIDGETS_LIBRARY XPWidgets "${XPSDK}/Libraries/Mac")
else()
    set(XPLM_LIBRARY "")
    set(XPWIDGETS_LIBRARY "")
endif()

set(LIBACFUTILS_LIBRARY "${LIBACFUTILS}/qmake/${PLAT_SHORT}/libacfutils.a")

execute_process(COMMAND bash -c "${LIBACFUTILS}/pkg-config-deps ${PLAT_LONG} \
        --static-openal --libs" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LIB_ACF_LIBS)


target_link_libraries(plugin 
    PRIVATE
    ${LIBACFUTILS_LIBRARY}
    ${XPLM_LIBRARY}
    ${XPWIDGETS_LIBRARY}
    ${LIB_ACF_LIBS}
    )

if(NOT APPLE)
    target_link_libraries(plugin
      PRIVATE
      "-static-libgcc")
endif()

if(${HOST} MATCHES x86_64-w64-mingw32)
    
    message("We are cross compiling for Windows on Linux...")

    #target_link_libraries(plugin
    #  PRIVATE
    #  "-L${LIBACFUTILS}/GL_for_Windows/lib -lglu32 -lopengl32" )

endif()

get_target_property(OUT plugin LINK_LIBRARIES)
message(STATUS "${OUT}")

set_target_properties(plugin PROPERTIES PREFIX "")
set_target_properties(plugin PROPERTIES SUFFIX "")
set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${PLUGIN_BIN_OUTDIR}" )
set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY
    "${PLUGIN_BIN_OUTDIR}" )
set_target_properties(plugin PROPERTIES OUTPUT_NAME "PanelClone.xpl")


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

target_link_libraries( plugin
    PRIVATE
    cppzmq
    ${Protobuf_LIBRARIES}
    absl::log_internal_message
    absl::log_internal_check_op
)

if(UNIX AND NOT APPLE)
    set_target_properties(plugin PROPERTIES LINK_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -nodefaultlibs \
        -undefined_warning -fPIC -fvisibility=hidden -fno-gnu-unique")
endif()

install(
    TARGETS subscribe publish
    DESTINATION ${PROJECT_SOURCE_DIR}
)
