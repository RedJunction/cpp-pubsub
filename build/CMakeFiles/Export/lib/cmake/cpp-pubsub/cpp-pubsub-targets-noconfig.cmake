#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cpp-pubsub::cpp-pubsub" for configuration ""
set_property(TARGET cpp-pubsub::cpp-pubsub APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(cpp-pubsub::cpp-pubsub PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcpp-pubsub.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cpp-pubsub::cpp-pubsub )
list(APPEND _IMPORT_CHECK_FILES_FOR_cpp-pubsub::cpp-pubsub "${_IMPORT_PREFIX}/lib/libcpp-pubsub.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
