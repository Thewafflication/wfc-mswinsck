# wfc-mswinsck: Winsock (mswinsck.ocx, library MSWinsockLib).
#
# The member table is written by hand from the documented members;
# src/winsock_io.cpp is the socket layer. The table is built static by
# default; WFC_MSWINSCK_SHARED builds it as wfcmswinsck.dll so a program can
# load it at run time instead. The version is the one in VERSION and is
# stamped into the DLL's version resource.
#
# Parent project variables used when present: WFC_OCX_SHARED (the default of
# WFC_MSWINSCK_SHARED) and the functions wfc_enable_strict_warnings and
# wsp_enable_hardening.

# Standalone build: the library outside WFC. WFC itself adds this directory
# with add_subdirectory and supplies the project, CTest and the WSP hardening.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.28)
  project(wfc-mswinsck LANGUAGES CXX)
  option(WFC_BUILD_TESTS "Build the tests" ON)
  include(CTest)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  include(WfcWarnings)
  # WSP-SEC-0015: the pinned WSP baseline supplies the hardening flags (not for
  # the WebAssembly target, whose toolchain has none of them).
  if(
    EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wsp/tools/cmake/WspBuild.cmake"
    AND NOT EMSCRIPTEN
  )
    include("${CMAKE_CURRENT_SOURCE_DIR}/wsp/tools/cmake/WspBuild.cmake")
  endif()
endif()

if(NOT DEFINED WFC_OCX_SHARED)
  set(WFC_OCX_SHARED OFF)
endif()
option(
  WFC_MSWINSCK_SHARED
  "Build the WFC mswinsck library as wfcmswinsck.dll"
  ${WFC_OCX_SHARED}
)

file(
  STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
  WFC_MSWINSCK_VERSION
  LIMIT_COUNT 1
)
set(
  WFC_MSWINSCK_VERSION
  "${WFC_MSWINSCK_VERSION}"
  CACHE INTERNAL
  "wfcmswinsck version"
)

set(WFC_MSWINSCK_SOURCES src/mswinsck_tables.cpp)

if(WFC_MSWINSCK_SHARED AND WIN32)
  enable_language(RC)
  string(REPLACE "." ";" version_parts "${WFC_MSWINSCK_VERSION}")
  list(GET version_parts 0 WFC_LIB_VERSION_MAJOR)
  list(GET version_parts 1 WFC_LIB_VERSION_MINOR)
  list(GET version_parts 2 WFC_LIB_VERSION_PATCH)
  set(WFC_LIB_VERSION "${WFC_MSWINSCK_VERSION}")
  file(
    STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/TARGET"
    WFC_LIB_TARGET
    LIMIT_COUNT 1
  )
  configure_file(
    src/registration_info.hpp.in
    "${CMAKE_CURRENT_BINARY_DIR}/registration_info.hpp"
    @ONLY
  )
  set(WFC_LIB_NAME wfcmswinsck)
  set(WFC_LIB_DESCRIPTION "WFC Winsock")
  configure_file(
    version.rc.in
    "${CMAKE_CURRENT_BINARY_DIR}/wfcmswinsck.rc"
    @ONLY
  )
  add_library(
    wfc_mswinsck
    SHARED
    ${WFC_MSWINSCK_SOURCES}
    src/registration.cpp
    src/wfcmswinsck.def
    "${CMAKE_CURRENT_BINARY_DIR}/wfcmswinsck.rc"
  )
  target_include_directories(wfc_mswinsck PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(wfc_mswinsck PRIVATE advapi32)
  target_compile_definitions(
    wfc_mswinsck
    PUBLIC WFC_MSWINSCK_SHARED
    PRIVATE WFC_MSWINSCK_BUILD_DLL
  )
  set_target_properties(
    wfc_mswinsck
    PROPERTIES
      OUTPUT_NAME wfcmswinsck
      VERSION "${WFC_MSWINSCK_VERSION}"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
else()
  add_library(wfc_mswinsck STATIC ${WFC_MSWINSCK_SOURCES})
  set_target_properties(wfc_mswinsck PROPERTIES OUTPUT_NAME wfcmswinsck_static)
endif()
target_include_directories(
  wfc_mswinsck
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_features(wfc_mswinsck PRIVATE cxx_std_20)

# The socket layer is not part of the DLL: it stays in the program.
add_library(wfc_mswinsck_io STATIC src/winsock_io.cpp)
target_include_directories(
  wfc_mswinsck_io
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_features(wfc_mswinsck_io PRIVATE cxx_std_20)
if(WIN32)
  target_link_libraries(wfc_mswinsck_io PUBLIC ws2_32)
endif()
if(COMMAND wfc_enable_strict_warnings)
  wfc_enable_strict_warnings(wfc_mswinsck_io)
endif()
if(COMMAND wsp_enable_hardening)
  wsp_enable_hardening(wfc_mswinsck_io)
endif()
target_link_libraries(wfc_mswinsck INTERFACE wfc_mswinsck_io)
if(COMMAND wfc_enable_strict_warnings)
  wfc_enable_strict_warnings(wfc_mswinsck)
endif()
if(COMMAND wsp_enable_hardening)
  wsp_enable_hardening(wfc_mswinsck)
endif()

# --- Tests --------------------------------------------------------------------
# The table (any build) and, in the shared build, the DLL itself: a C program
# loads wfcmswinsck.dll, checks its COM entry points and version resource, and
# registers and unregisters it in the registry of the current user.
if(BUILD_TESTING AND WFC_BUILD_TESTS)
  add_executable(mswinsck_tables_test tests/mswinsck_tables_test.cpp)
  target_link_libraries(mswinsck_tables_test PRIVATE wfc_mswinsck)
  if(WFC_MSWINSCK_SHARED)
    # Next to the DLL, so the program finds it.
    set_target_properties(
      mswinsck_tables_test
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
  endif()
  target_compile_features(mswinsck_tables_test PRIVATE cxx_std_20)
  if(COMMAND wfc_enable_strict_warnings)
    wfc_enable_strict_warnings(mswinsck_tables_test)
  endif()
  if(COMMAND wsp_enable_hardening)
    wsp_enable_hardening(mswinsck_tables_test)
  endif()
  add_test(NAME TC-MP0010-mswinsck-tables COMMAND mswinsck_tables_test)
  set_tests_properties(
    TC-MP0010-mswinsck-tables
    PROPERTIES LABELS "unit;mp-0010;mswinsck"
  )

  if(WIN32)
    add_executable(winsock_io_tests tests/winsock_io_tests.cpp)
    target_link_libraries(winsock_io_tests PRIVATE wfc_mswinsck_io)
    target_compile_features(winsock_io_tests PRIVATE cxx_std_20)
    if(COMMAND wfc_enable_strict_warnings)
      wfc_enable_strict_warnings(winsock_io_tests)
    endif()
    if(COMMAND wsp_enable_hardening)
      wsp_enable_hardening(winsock_io_tests)
    endif()
    add_test(NAME TC-MP0010-mswinsck-io COMMAND winsock_io_tests)
    set_tests_properties(
      TC-MP0010-mswinsck-io
      PROPERTIES LABELS "integration;mp-0010;mswinsck" TIMEOUT 60
    )
  endif()

  if(WIN32 AND WFC_MSWINSCK_SHARED)
    enable_language(C)
    add_executable(mswinsck_dll_test tests/mswinsck_dll_test.c)
    target_link_libraries(mswinsck_dll_test PRIVATE version advapi32)
    add_dependencies(mswinsck_dll_test wfc_mswinsck)
    if(MSVC)
      target_compile_options(mswinsck_dll_test PRIVATE /W4 /WX)
    else()
      target_compile_options(mswinsck_dll_test PRIVATE -Wall -Wextra -Werror)
    endif()
    if(COMMAND wsp_enable_hardening)
      wsp_enable_hardening(mswinsck_dll_test)
    endif()
    add_test(
      NAME TC-MP0010-mswinsck-dll
      COMMAND
        mswinsck_dll_test "$<TARGET_FILE:wfc_mswinsck>" "${WFC_MSWINSCK_VERSION}"
        "${WFC_LIB_TARGET}"
    )
    set_tests_properties(
      TC-MP0010-mswinsck-dll
      PROPERTIES
        LABELS "integration;mp-0010;mswinsck"
        RESOURCE_LOCK registry
        SKIP_RETURN_CODE 77
    )
  endif()
endif()
