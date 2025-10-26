cmake_minimum_required(VERSION 3.15)
project(VinylTimecodeCHOP VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Platform detection
if(APPLE)
    set(PLATFORM_MACOS ON)
elseif(WIN32)
    set(PLATFORM_WINDOWS ON)
endif()

# xwax library sources (from Mixxx)
set(XWAX_SOURCES
    lib/xwax/timecoder.c
    lib/xwax/lut.c
    lib/xwax/pitch_kalman.c
)

# VinylTimecodeCHOP sources
set(CHOP_SOURCES
    src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp
)

# Create the plugin library
add_library(VinylTimecodeCHOP MODULE
    ${XWAX_SOURCES}
    ${CHOP_SOURCES}
)

# Include directories
target_include_directories(VinylTimecodeCHOP PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/touchdesigner-sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/xwax
)

# Compiler flags
if(PLATFORM_MACOS)
    target_compile_options(VinylTimecodeCHOP PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )

    # macOS specific settings for TouchDesigner plugin
    set_target_properties(VinylTimecodeCHOP PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "plugin"
        PREFIX ""
        SUFFIX ""
        OUTPUT_NAME "VinylTimecodeCHOP"
    )

    # Link frameworks (Accelerate for vDSP)
    target_link_libraries(VinylTimecodeCHOP PRIVATE
        "-framework Accelerate"
        "-framework CoreFoundation"
    )

    # Set architecture (Universal Binary)
    set_target_properties(VinylTimecodeCHOP PROPERTIES
        OSX_ARCHITECTURES "arm64;x86_64"
    )

    # Add math library
    target_link_libraries(VinylTimecodeCHOP PRIVATE m)

elseif(PLATFORM_WINDOWS)
    target_compile_options(VinylTimecodeCHOP PRIVATE
        /W3
        /wd4100  # unreferenced formal parameter
    )

    set_target_properties(VinylTimecodeCHOP PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
        OUTPUT_NAME "VinylTimecodeCHOP"
    )
endif()

# Output directory
set_target_properties(VinylTimecodeCHOP PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin"
)

# Installation
install(TARGETS VinylTimecodeCHOP
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
    BUNDLE DESTINATION .
)

# Status messages
message(STATUS "=================================")
message(STATUS "VinylTimecodeCHOP Configuration")
message(STATUS "=================================")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
if(PLATFORM_MACOS)
    message(STATUS "Architectures: arm64;x86_64")
endif()
message(STATUS "=================================")
