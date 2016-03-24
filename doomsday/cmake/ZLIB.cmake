if (NOT MSVC)
    find_package (ZLIB REQUIRED)
else ()
    # There is a prebuilt zlib DLL in the external/zlib directory.
    add_definitions (-DZLIB_WINAPI)
    set (zlibDir "${DENG_EXTERNAL_SOURCE_DIR}/zlib")
    set (ZLIB_INCLUDE_DIR "${zlibDir}/include")
    if (MSVC14)
        set (_subDir msvc2015)
    elseif (MSVC12)
        set (_subDir msvc2013)
    else ()
        message (FATAL_ERROR "Prebuilt zlib not available for this version of MSVC")
    endif ()
    set (ZLIB_LIBRARIES "${zlibDir}/lib/${_subDir}/${DENG_ARCH}/zlibwapi.lib")
    set (ZLIB_FOUND YES)
    deng_install_library ("${zlibDir}/lib/${_subDir}/${DENG_ARCH}/zlibwapi.dll")
    set (_subDir)
endif ()
