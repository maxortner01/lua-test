set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH OFF)

set(DIR ${CMAKE_CURRENT_LIST_DIR}/..)

find_package(Lua REQUIRED)

# Packages for collision detection
find_package(Eigen3 REQUIRED)
find_package(ccd REQUIRED)

# Crap for fcl dependencies
find_path(CCD_INCLUDE_DIR ccd/ccd.h
    HINTS "${PC_CCD_INCLUDE_DIRS}"
          "${PC_CCD_INCLUDE_DIRS}/.."
          ENV PKG_CONFIG_SYSTEM_INCLUDE_PATH
          ENV C_INCLUDE_PATH
          ENV CPLUS_INCLUDE_PATH)

# Using find_library() even if pkg-config is available ensures that the full
# path to the ccd library is available in CCD_LIBRARIES
find_library(CCD_LIBRARY ccd
HINTS "${PC_CCD_LIBRARY_DIRS}")

# libccd links to LibM on UNIX.
if(CYGWIN OR NOT WIN32)
find_library(M_LIBRARY m)
endif()

if(CCD_INCLUDE_DIR AND CCD_LIBRARY)
    set(CCD_INCLUDE_DIRS "${CCD_INCLUDE_DIR}")
    set(CCD_LIBRARIES "${CCD_LIBRARY}" "${M_LIBRARY}")
    set(ccd_FOUND ON)

    mark_as_advanced(CCD_INCLUDE_DIR CCD_LIBRARY)
endif()

set(SIMPLE2D_INCLUDE_DIRS
    ${LUA_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${CCD_INCLUDE_DIRS}
    ${DIR}/include
    ${DIR}/extern/spdlog/include
    ${DIR}/extern/flecs/include
    ${DIR}/extern/fcl/include
    ${DIR}/build/gcc/extern/fcl/include)

set(SIMPLE2D_LIBRARIES simple2d-lua simple2d-engine simple2d-graphics flecs)