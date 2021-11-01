#
# Locate and configure the Google Protocol Buffers library
#
# Adds the following targets:
#
#  protobuf::libprotobuf - Protobuf library
#  protobuf::libprotobuf-lite - Protobuf lite library
#  protobuf::libprotoc - Protobuf Protoc Library
#  protobuf::protoc - protoc executable
#

#
# Generates C++ sources from the .proto files
#
# protobuf_generate_cpp (<SRCS> <HDRS> <DEST> [<ARGN>...])
#
#  SRCS - variable to define with autogenerated source files
#  HDRS - variable to define with autogenerated header files
#  DEST - directory where the source files will be created
#  ARGN - .proto files
#
function(PROTOBUF_GENERATE_CPP SRCS HDRS DEST)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif()

  if(PROTOBUF_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  else()
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(DEFINED PROTOBUF_IMPORT_DIRS)
    foreach(DIR ${PROTOBUF_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    list(APPEND ${SRCS} "${DEST}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${DEST}/${FIL_WE}.pb.h")

    add_custom_command(
      OUTPUT "${DEST}/${FIL_WE}.pb.cc"
             "${DEST}/${FIL_WE}.pb.h"
      COMMAND protobuf::protoc
      ARGS --cpp_out ${DEST} ${_protobuf_include_path} ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

# By default have PROTOBUF_GENERATE_CPP macro pass -I to protoc
# for each directory where a proto file is referenced.
if(NOT DEFINED PROTOBUF_GENERATE_CPP_APPEND_PATH)
  set(PROTOBUF_GENERATE_CPP_APPEND_PATH TRUE)
endif()

# Find the include directory
find_path(PROTOBUF_INCLUDE_DIR google/protobuf/service.h)
mark_as_advanced(PROTOBUF_INCLUDE_DIR)

# The Protobuf library
find_library(PROTOBUF_LIBRARY NAMES protobuf)
mark_as_advanced(PROTOBUF_LIBRARY)
add_library(protobuf::libprotobuf UNKNOWN IMPORTED)
set_target_properties(protobuf::libprotobuf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES pthread
    IMPORTED_LOCATION ${PROTOBUF_LIBRARY}
)

# The Protobuf lite library
find_library(PROTOBUF_LITE_LIBRARY NAMES protobuf-lite)
mark_as_advanced(PROTOBUF_LITE_LIBRARY)
add_library(protobuf::libprotobuf-lite UNKNOWN IMPORTED)
set_target_properties(protobuf::libprotobuf-lite PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES pthread
    IMPORTED_LOCATION ${PROTOBUF_LITE_LIBRARY}
)

# The Protobuf Protoc Library
find_library(PROTOBUF_PROTOC_LIBRARY NAMES protoc)
mark_as_advanced(PROTOBUF_PROTOC_LIBRARY)
add_library(protobuf::libprotoc UNKNOWN IMPORTED)
set_target_properties(protobuf::libprotoc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES protobuf::libprotobuf
    IMPORTED_LOCATION ${PROTOBUF_PROTOC_LIBRARY}
)

# Find the protoc executable.
if("$ENV{CMAKE_SYSROOT}" STREQUAL "")
    # Use system protoc if not cross compiling
    find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc)
else()
    # Use CMAKE_PROGRAM_PATH/protoc if cross compiling
    find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc NO_CMAKE_FIND_ROOT_PATH)
endif()

mark_as_advanced(PROTOBUF_PROTOC_EXECUTABLE)
add_executable(protobuf::protoc IMPORTED)
set_target_properties(protobuf::protoc PROPERTIES
    IMPORTED_LOCATION ${PROTOBUF_PROTOC_EXECUTABLE}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Protobuf DEFAULT_MSG
    PROTOBUF_LIBRARY PROTOBUF_INCLUDE_DIR PROTOBUF_PROTOC_EXECUTABLE)
