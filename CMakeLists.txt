cmake_minimum_required(VERSION 3.18)

if(UNIX)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
    endif()
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "~/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    endif()
elseif(WIN32)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

# Project Name
project(HackArena2.5-StereoTanks-Cxx)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system beast asio)
find_package(nlohmann_json REQUIRED)

# Include Boost directories
include_directories(${Boost_INCLUDE_DIRS})

# Add executable
add_executable(HackArena2.5-StereoTanks-Cxx src/main.cpp
        src/web-socket-client.cpp
        src/web-socket-client.h
        src/packet.h
        src/pch.h
        src/bot/bot.cpp
        src/bot/bot.h
        src/processed-packets.h
        src/handler.cpp
        src/handler.h)

target_precompile_headers(HackArena2.5-StereoTanks-Cxx PRIVATE src/pch.h)

# Define preprocessor macros
if(WIN32)
    target_compile_definitions(HackArena2.5-StereoTanks-Cxx PRIVATE _WIN32_WINDOWS=0x0A00)
elseif(UNIX)
    # Unix-specific settings
endif()

# Link Boost libraries
target_link_libraries(HackArena2.5-StereoTanks-Cxx PRIVATE Boost::system nlohmann_json::nlohmann_json)

# Ensure static linking
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_STATIC_RUNTIME ON)