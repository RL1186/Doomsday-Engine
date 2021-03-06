# Doomsday Engine -- libcore

cmake_minimum_required (VERSION 3.1)
project (DENG_LIBCORE)
include (../../cmake/Config.cmake)

# Dependencies.
include (ZLIB)
find_package (Git QUIET)

# Definitions.
add_definitions (
    -D__DENG2__=1
    -DLIBDENG2_MAJOR_VERSION=${DENG_VERSION_MAJOR}
    -DLIBDENG2_MINOR_VERSION=${DENG_VERSION_MINOR}
    -DLIBDENG2_PATCHLEVEL=${DENG_VERSION_PATCH}
    -DLIBDENG2_RELEASE_LABEL="${DENG_RELEASE_TYPE}"
    -DLIBDENG2_VERSION="${DENG_VERSION}"
)
if (DEFINED DENG_BUILD)
    add_definitions (-DLIBDENG2_BUILD_TEXT="${DENG_BUILD}")
endif ()
if (GIT_FOUND)
    execute_process (
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --long
        OUTPUT_VARIABLE DENG_GIT_DESCRIPTION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (DENG_GIT_DESCRIPTION)
        message (STATUS "Revision: ${DENG_GIT_DESCRIPTION}")
        add_definitions (-DLIBDENG2_GIT_DESCRIPTION="${DENG_GIT_DESCRIPTION}")
    endif ()
endif ()

# Source and header files.
file (GLOB_RECURSE HEADERS include/de/[a-zA-Z]*)
file (GLOB SOURCES src/*.cpp src/*.h src/core/*.h)

deng_merge_sources (concurrency    src/concurrency/*.cpp)
deng_merge_sources (core           src/core/*.cpp)
deng_merge_sources (data           src/data/*.cpp)
deng_merge_sources (filesys        src/filesys/*.cpp)
deng_merge_sources (filesys_remote src/filesys/remote/*.cpp)
deng_merge_sources (net            src/net/*.cpp)
deng_merge_sources (scriptsys      src/scriptsys/*.cpp)
deng_merge_sources (widgets        src/widgets/*.cpp)

if (WIN32)
    # Update the resource file.
    set (DENG_LIBCORE_VERSION_WINDOWS ${DENG_VERSION_WINDOWS})
    configure_file (res/win32/core.rc.in core.rc)
    list (APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/core.rc)
    include_directories (res/win32)
endif ()

deng_add_package (net.dengine.stdlib)

deng_add_library (libcore ${SOURCES} ${HEADERS})
target_include_directories (libcore PRIVATE ${ZLIB_INCLUDE_DIR})
deng_target_link_qt (libcore PUBLIC Core Network)
target_link_libraries (libcore PUBLIC ${ZLIB_LIBRARIES})
if (UNIX AND NOT APPLE)
    if (DENG_BASE_DIR)
        set (baseDir "${DENG_BASE_DIR}")
    else ()
        set (baseDir "${CMAKE_INSTALL_PREFIX}/${DENG_INSTALL_DATA_DIR}")
    endif ()
    if (DENG_LIBRARY_DIR)
        set (libDir "${DENG_LIBRARY_DIR}")
    else ()
        set (libDir "${CMAKE_INSTALL_PREFIX}/${DENG_INSTALL_PLUGIN_DIR}")
    endif ()
    target_compile_definitions (libcore PUBLIC
        -DDENG_BASE_DIR="${baseDir}"
        -DDENG_LIBRARY_DIR="${libDir}"
    )
endif ()

deng_deploy_library (libcore DengCore)

deng_cotire (libcore src/precompiled.h)
