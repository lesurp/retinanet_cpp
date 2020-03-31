cmake_minimum_required(VERSION 3.0)
project(retinanet_cpp)
set(CMAKE_CXX_STANDARD 14)

### Deps...
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

### Create actual retinanet target
set(retinanet_public_headers
    src/engine.h
    )
add_library(retinanet
    src/calibrator.h
    src/cuda/decode.cu
    src/cuda/decode.h
    src/cuda/nms.cu
    src/cuda/nms.h
    src/cuda/utils.h
    src/engine.cpp
    )
set_target_properties(retinanet PROPERTIES 
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    PUBLIC_HEADER ${retinanet_public_headers}
    )

target_link_libraries(retinanet PUBLIC TensorRT opencv_core cuda)

### Installation stuff
# *.a + export found target
set(lib_dest lib/retinanet)
set(inc_dest include/retinanet)
install(TARGETS retinanet
    EXPORT retinanet
    LIBRARY DESTINATION ${lib_dest}
    PUBLIC_HEADER DESTINATION ${inc_dest}
    )

install(FILES retinanet-config.cmake DESTINATION ${lib_dest})
install(TARGETS TensorRT EXPORT retinanet)
install(TARGETS cuda EXPORT retinanet)
