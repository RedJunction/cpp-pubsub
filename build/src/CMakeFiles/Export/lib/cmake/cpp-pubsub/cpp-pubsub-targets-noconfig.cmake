#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CPPPubSub::CPPPubSub" for configuration ""
set_property(TARGET CPPPubSub::CPPPubSub APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(CPPPubSub::CPPPubSub PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcpp-pubsub.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS CPPPubSub::CPPPubSub )
list(APPEND _IMPORT_CHECK_FILES_FOR_CPPPubSub::CPPPubSub "${_IMPORT_PREFIX}/lib/libcpp-pubsub.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
