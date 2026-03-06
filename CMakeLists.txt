cmake_minimum_required(VERSION 3.16)
project(CameraCapture VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

# --------------------------------------------------------------------------
# Canon EDSDK — bundled inside the repo under EDSDK/
# --------------------------------------------------------------------------
set(EDSDK_DIR "${CMAKE_SOURCE_DIR}/EDSDK")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(EDSDK_LIB    "${EDSDK_DIR}/x64/EDSDK.lib")
    set(EDSDK_DLL    "${EDSDK_DIR}/x64/EDSDK.dll")
    set(EDSIMAGE_DLL "${EDSDK_DIR}/x64/EdsImage.dll")
else()
    set(EDSDK_LIB    "${EDSDK_DIR}/x86/EDSDK.lib")
    set(EDSDK_DLL    "${EDSDK_DIR}/x86/EDSDK.dll")
    set(EDSIMAGE_DLL "${EDSDK_DIR}/x86/EdsImage.dll")
endif()

# --------------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------------
add_executable(CameraCapture
    main.cpp
    canoncontroller.cpp
    canoncontroller.h
)

target_include_directories(CameraCapture PRIVATE "${EDSDK_DIR}/Header")
target_link_libraries(CameraCapture PRIVATE "${EDSDK_LIB}")

if(MSVC)
    target_compile_options(CameraCapture PRIVATE /Zc:__cplusplus)
    set_target_properties(CameraCapture PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

# Copy DLLs next to the .exe after each build
foreach(_dll "${EDSDK_DLL}" "${EDSIMAGE_DLL}")
    add_custom_command(TARGET CameraCapture POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_dll}"
            $<TARGET_FILE_DIR:CameraCapture>
    )
endforeach()
