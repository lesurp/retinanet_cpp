cmake_minimum_required(VERSION 3.11)
project(retinanet_cpp)
set(CMAKE_CXX_STANDARD 14)

################### compiler flags {
set(Release_FLAGS "-O3 -DNDEBUG"
CACHE STRING
"Flags to pass to the compiler for the 'Release' configuration")
set(RelWithDebInfo_FLAGS "-O2 -g"
CACHE STRING
"Flags to pass to the compiler for the 'RelWithDebInfo' configuration")
set(Debug_FLAGS "-O0 -g"
    CACHE STRING
    "Flags to pass to the compiler for the 'Debug' configuration")
list(APPEND CONFIG_FLAGS Release_FLAGS RelWithDebInfo_FLAGS Debug_FLAGS)
################### } compiler flags

################### dependencies {
# TensorRT
if(NOT DEFINED TensorRT_DIR)
    if(DEFINED ENV{TensorRT_DIR})
        set(TensorRT_DIR $ENV{TensorRT_DIR})
    else()
        message(FATAL_ERROR "TensorRT_DIR must be defined")
    endif()
endif()
add_library(TensorRT INTERFACE)
target_link_libraries(TensorRT INTERFACE
    ${TensorRT_DIR}/lib/libnvonnxparser.so
    ${TensorRT_DIR}/lib/libnvinfer.so)
target_include_directories(TensorRT INTERFACE ${TensorRT_DIR}/include)

# CUDA
find_package(CUDA REQUIRED)
enable_language(CUDA)
add_library(cuda INTERFACE)
target_include_directories(cuda INTERFACE ${CUDA_INCLUDE_DIRS})
target_link_libraries(cuda INTERFACE ${CUDA_LIBRARIES})
set(CMAKE_CUDA_FLAGS "--expt-extended-lambda")

# OpenCV
find_package(OpenCV REQUIRED opencv_core)

set(dependencies opencv_core TensorRT cuda)
################### } dependencies

################### source files {
set(public_hdr
    src/engine.h
)
set(private_src
    src/calibrator.h
    src/cuda/decode.cu
    src/cuda/decode.h
    src/cuda/nms.cu
    src/cuda/nms.h
    src/cuda/utils.h
    src/engine.cpp
)
################### } source files

################### create and install targets {
### Compile the library for each build type, and install all of them
### We then alias the "retinanet" target to "retinanet_${CMAKE_BUILD_TYPE}
### when looking for retinanet, loading the appropriate archive
set(main_lib_dest lib/retinanet)
foreach(config_flags IN LISTS CONFIG_FLAGS)
    set(CMAKE_CXX_FLAGS ${${config_flags}})
    string(REPLACE "_FLAGS" "" config ${config_flags})
    set(target retinanet_${config})

    add_library(${target} ${private_src} ${public_hdr})
    set_target_properties(${target} PROPERTIES 
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
    )
    target_link_libraries(${target} PUBLIC ${dependencies})

    set(lib_dest ${main_lib_dest}/${config})
    install(TARGETS ${target}
        EXPORT ${target}
        ARCHIVE DESTINATION ${lib_dest}
    )

    install(TARGETS TensorRT cuda EXPORT ${target})
    install(EXPORT ${target} DESTINATION ${lib_dest})
endforeach()

# we don't set "PUBLIC_HEADER" property on each target as each would install the
# headers
install(FILES retinanet-config.cmake DESTINATION ${main_lib_dest})
install(FILES ${public_hdr} DESTINATION include/retinanet)
################### } create and install targets
