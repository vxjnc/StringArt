cmake_minimum_required(VERSION 3.28)

project(StringArt CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCE_FILES
    "src/*.cpp"
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    include
)

target_compile_options(${PROJECT_NAME} PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic
    -march=native 
    -mtune=native
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring project ${PROJECT_NAME} for OPTIMIZATION mode (Release)")
    
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O3
    )
    
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    
    target_link_options(${PROJECT_NAME} PRIVATE
        -s
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring project ${PROJECT_NAME} for PROFILING/DEBUG mode (Debug)")

    target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-omit-frame-pointer
    )

else()
    message(STATUS "Configuring project ${PROJECT_NAME} for default mode: ${CMAKE_BUILD_TYPE}")
endif()

set(EXECUTABLE_OUTPUT_PATH ${SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
