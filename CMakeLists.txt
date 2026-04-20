CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0)
PROJECT(unqlite VERSION 1.1.9 LANGUAGES C)
INCLUDE(GNUInstallDirs)

# ============================================================================
# Build type
# ============================================================================
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRel)" FORCE)
ENDIF()
MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Project options
# ============================================================================
OPTION(ENABLE_THREADS    "Enable thread safety support" OFF)
OPTION(ENABLE_SANITIZERS "Enable AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
OPTION(ENABLE_COVERAGE   "Enable code coverage reporting" OFF)
OPTION(ENABLE_DOCS       "Enable documentation generation (Doxygen)" OFF)

# ============================================================================
# Compiler warnings - strict by default
# ============================================================================
IF(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  ADD_COMPILE_OPTIONS(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wpointer-arith -Wcast-qual
    -Wformat=2 -Wno-unused-parameter
    -fno-omit-frame-pointer
  )
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_COMPILE_OPTIONS(-g3 -O0)
  ENDIF()
ELSEIF(MSVC)
  ADD_COMPILE_OPTIONS(
    /W4 /wd4100 /wd4200
    /utf-8
  )
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_COMPILE_OPTIONS(/Zi /Od)
  ENDIF()
ENDIF()

# ============================================================================
# Thread safety
# ============================================================================
IF(ENABLE_THREADS)
  ADD_DEFINITIONS("-DUNQLITE_ENABLE_THREADS")
  IF(NOT WIN32)
    FIND_PACKAGE(Threads REQUIRED)
  ENDIF()
ENDIF()

# ============================================================================
# Sanitizers (Debug builds only)
# ============================================================================
IF(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  IF(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    ADD_COMPILE_OPTIONS(-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all)
    ADD_LINK_OPTIONS(-fsanitize=address -fsanitize=undefined)
    MESSAGE(STATUS "Sanitizers enabled: ASan + UBSan")
  ELSE()
    MESSAGE(WARNING "Sanitizers only supported with GCC/Clang in Debug mode")
  ENDIF()
ENDIF()

# ============================================================================
# Code coverage (Debug builds only, GCC/Clang)
# ============================================================================
IF(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  IF(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    ADD_COMPILE_OPTIONS(--coverage -fprofile-arcs -ftest-coverage)
    ADD_LINK_OPTIONS(--coverage)
    MESSAGE(STATUS "Code coverage enabled")
  ELSE()
    MESSAGE(WARNING "Code coverage only supported with GCC/Clang")
  ENDIF()
ENDIF()

# ============================================================================
# Install prefix
# ============================================================================
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
ENDIF()

# ============================================================================
# Library
# ============================================================================
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src)

SET(HEADERS_UNQLITE
  unqlite.h
)
source_group("Header Files" FILES "${HEADERS_UNQLITE}")

SET(SOURCES_UNQLITE
  unqlite.c
)
source_group("Source Files" FILES "${SOURCES_UNQLITE}")

SET(UNQLITE_STATIC_LIB unqlite)
ADD_LIBRARY(${UNQLITE_STATIC_LIB} STATIC ${HEADERS_UNQLITE} ${SOURCES_UNQLITE})

IF(ENABLE_THREADS AND NOT WIN32)
  TARGET_LINK_LIBRARIES(${UNQLITE_STATIC_LIB} PUBLIC Threads::Threads)
ENDIF()

SET_PROPERTY(TARGET ${UNQLITE_STATIC_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# Install
# ============================================================================
INSTALL(TARGETS ${UNQLITE_STATIC_LIB} COMPONENT devel ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
INSTALL(FILES ${HEADERS_UNQLITE} COMPONENT devel DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# ============================================================================
# Testing
# ============================================================================
INCLUDE(CTest)
IF(BUILD_TESTING)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY("example")
ENDIF()

# ============================================================================
# Summary
# ============================================================================
MESSAGE(STATUS "===========================================")
MESSAGE(STATUS "  UnQLite ${PROJECT_VERSION} Build Configuration")
MESSAGE(STATUS "===========================================")
MESSAGE(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "  Compiler:      ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
MESSAGE(STATUS "  Threads:       ${ENABLE_THREADS}")
MESSAGE(STATUS "  Sanitizers:    ${ENABLE_SANITIZERS}")
MESSAGE(STATUS "  Coverage:      ${ENABLE_COVERAGE}")
MESSAGE(STATUS "  Testing:       ${BUILD_TESTING}")
MESSAGE(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "===========================================")
