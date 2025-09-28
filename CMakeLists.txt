cmake_minimum_required(VERSION 3.14)

# Check for compiler before project() command
if(NOT DEFINED CMAKE_CXX_COMPILER)
    # Try to find common compilers
    find_program(CMAKE_CXX_COMPILER NAMES g++ clang++ c++)
    if(NOT CMAKE_CXX_COMPILER)
        message(FATAL_ERROR "No C++ compiler found. Please install g++ or clang++.")
    endif()
endif()

project(iriscpp VERSION 1.0.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vcpkg toolchain file if not specified
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(WIN32)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    else()
        set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

# Debug option
option(ENABLE_DEBUG_OUTPUT "Enable debug output for API calls" OFF)

# Platform specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
else()
    add_definitions(-D_POSIX_C_SOURCE=200112L)
endif()

if(ENABLE_DEBUG_OUTPUT)
    add_definitions(-DDEBUG_OUTPUT)
endif()

# Add dependencies
find_package(CURL REQUIRED)

# Find nlohmann_json
if(NOT DEFINED nlohmann_json_DIR)
    # Try to find via vcpkg first
    find_package(nlohmann_json CONFIG)
    
    if(NOT nlohmann_json_FOUND)
        # Try to find system-installed version
        find_package(PkgConfig)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(NLOHMANN_JSON QUIET nlohmann_json)
        endif()
        
        # Try common system paths
        if(NOT NLOHMANN_JSON_FOUND)
            find_path(JSON_INCLUDE_DIR
                NAMES nlohmann/json.hpp
                PATHS
                    ${CMAKE_INSTALL_PREFIX}/include
                    /usr/include
                    /usr/local/include
                    $ENV{HOME}/vcpkg/installed/x64-linux/include
                    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
            )
            
            if(JSON_INCLUDE_DIR)
                set(NLOHMANN_JSON_FOUND TRUE)
                set(NLOHMANN_JSON_INCLUDE_DIRS ${JSON_INCLUDE_DIR})
            else()
                # If still not found, download and install it
                include(FetchContent)
                FetchContent_Declare(json
                    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
                    URL_HASH SHA256=8c4b26bf4b422252e13f332bc5e388ec0ab5451f999a7999993acf29d4ef2561
                )
                FetchContent_MakeAvailable(json)
            endif()
        endif()
    endif()
endif()

# Add library
add_library(${PROJECT_NAME}
    src/iris_api.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
)

# Link and include nlohmann_json
if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
else()
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${nlohmann_json_INCLUDE_DIRS}
        $ENV{HOME}/vcpkg/installed/x64-linux/include
    )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    CURL::libcurl
    $<TARGET_NAME_IF_EXISTS:nlohmann_json::nlohmann_json>
    $<TARGET_NAME_IF_EXISTS:nlohmann_json>
)

# Examples
add_subdirectory(examples)

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)