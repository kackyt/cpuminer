project(minerd)

cmake_minimum_required(VERSION 3.0.1)

# enforce C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

set(BUILD_TYPE "Release;Debug")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${BUILD_TYPE}")

# option to add static libgcc and libstdc++
option(CMAKE_LINK_STATIC "link as much as possible libraries static" OFF)


################################################################################
# Find PThreads
################################################################################

find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

################################################################################
# Windows Sockets
################################################################################

if(WIN32)
    set(LIBS ${LIBS} wsock32 ws2_32)
endif()


################################################################################
# Compile & Link
################################################################################

# activate sse2 and aes-ni
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -maes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -maes")
endif()

# activate static libgcc and libstdc++ linking
if(CMAKE_LINK_STATIC)
    set(BUILD_SHARED_LIBRARIES OFF)
    set(DL_LIB ${CMAKE_DL_LIBS})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(LIBS "-static-libgcc -static-libstdc++ ${LIBS}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

list(APPEND SRCFILES_APP "stratum.cpp" "main.cpp")

list(APPEND SRCFILES_LIB "yescrypt.cpp" "util.c" "sha256.c")

add_executable(minerd
    ${SRCFILES_APP}
)

set(EXECUTABLE_OUTPUT_PATH "bin")

################################################################################
# Install
################################################################################

# do not install the binary if the project and install are equal
if( NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${PROJECT_BINARY_DIR}" )
    install(TARGETS minerd
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif()

