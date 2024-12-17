cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

if(NOT DEFINED ENV{JUNOTOP})
  message(FATAL_ERROR "[JUNO] This need to be setup and built inside of a JUNO environment. Please source the environement before building the project")
endif()

if(NOT DEFINED ENV{WORKTOP})
  set(TOP $ENV{JUNOTOP})
else()
  set(TOP $ENV{WORKTOP})
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wcast-align -Wunused -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# ================== Setup ROOT ===============

execute_process(COMMAND root-config --prefix
  COMMAND tr -d \\n
  RESULT_VARIABLE ROOT_PRESENT
  OUTPUT_VARIABLE ROOT_PREFIX_PATH)

if(${ROOT_PRESENT} GREATER 0)
  message(FATAL_ERROR "[ROOT] Cannot find a root install. Please ensure that root is correctly installed or install it following the instruction at https://root.cern/install/")
endif()

# ================== Python binding ===========

execute_process(COMMAND python3 -m pybind11 --cmake
  COMMAND tr -d \\n
  RESULT_VARIABLE PYBIND_PRESENT
  OUTPUT_VARIABLE PYBIND_OUTPUT_PATH)
if(${PYBIND_PRESENT} GREATER 0)
  message(FATAL_ERROR "[Pybind11] Can't find pybind11")
else()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PYBIND_OUTPUT_PATH})

  find_package(pybind11 CONFIG REQUIRED)
  find_package(Python 3 REQUIRED COMPONENTS Interpreter Development)

  message(STATUS "[Pybind11] Python binding enabled and found")
endif()


# ================== Global conf ==============

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${ROOT_PREFIX_PATH})

find_package(ROOT REQUIRED Tree)
include_directories(${Python_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} $ENV{JUNOTOP}/junosw/InstallArea/include ${pybind11_INCLUDE_DIR})

# ================== CLHEP ====================


# ================== C++ Lib ==================
project(EDMReader VERSION 1.0.0 DESCRIPTION "Library optimized to read JUNO EDM files")

find_library(CLHEP_LOCATION "CLHEP" HINT ENV LD_LIBRARY_PATH)

add_library(CLHEP SHARED IMPORTED)
set_target_properties(CLHEP PROPERTIES
  IMPORTED_LOCATION "${CLHEP_LOCATION}")

option(BUILD_FOR_BUNDLE "Build the library for bundling -- recompile and bring the EDM" OFF)

if(BUILD_FOR_BUNDLE)
  include(GNUInstallDirs)

  list(APPEND CMAKE_MODULE_PATH "${TOP}/junosw")
  list(APPEND CMAKE_MODULE_PATH "${TOP}/junosw/cmake/legacy/Modules")

  include(cmake/legacy/JUNODependencies)

  # add macros
  include(PKG)
  include(EDM)

  add_subdirectory(${TOP}/junosw/XmlObjDesc XmlObjDesc)
  add_subdirectory(${TOP}/junosw/DataModel DataModel)
else()
  link_directories($ENV{JUNOTOP}/junosw/InstallArea/lib $ENV{JUNOTOP}/junosw/InstallArea/lib64)
endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(EDMReader SHARED
  src/EDMReader.cc
  )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(EDMReader PRIVATE BUILD_VERBOSITY=4)
endif()

target_include_directories(EDMReader PRIVATE
  ${TOP}/junosw/InstallArea/include)

set_target_properties(EDMReader PROPERTIES
  VERSION ${PROJECT_VERSION}
  PUBLIC_HEADER src/EDMReader.hpp
  CXX_STANDARD 17)

target_link_libraries(EDMReader
  ${ROOT_LIBRARIES}
  CLHEPDict
  SimEvent
  CalibEvent
  CLHEP
  )

set_target_properties(EDMReader PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

install(TARGETS EDMReader
  TYPE LIBRARY
  CONFIGURATIONS Release)

install(FILES src/EDMReader.hpp
  TYPE INCLUDE)
install(DIRECTORY dat
  TYPE DATA)

# ===================Python bindings ========

project(PyEDMReader LANGUAGES CXX VERSION 0.6.0)

pybind11_add_module(_core MODULE
  src/PyBinding.cpp
  )

target_link_libraries(_core PRIVATE EDMReader)
target_link_libraries(_core PRIVATE
  ${ROOT_LIBRARIES}
  )

set_target_properties(_core PROPERTIES
  CXX_STANDARD 17)

target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS _core EDMReader DESTINATION PyEDMReader)
if(BUILD_FOR_BUNDLE)
  install(TARGETS EDMReader DESTINATION PyEDMReader)
  install(TARGETS SimEvent DESTINATION PyEDMReader)
  install(TARGETS CalibEvent DESTINATION PyEDMReader)
endif()
