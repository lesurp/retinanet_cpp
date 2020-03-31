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
set(retinanet_headers
    src/cuda/decode.h
    src/cuda/nms.h
    src/cuda/utils.h
    src/engine.h
    src/calibrator.h
    )
add_library(retinanet
    src/cuda/decode.cu
    src/cuda/nms.cu
    src/engine.cpp
    ${retinanet_headers}
    )
set_target_properties(retinanet PROPERTIES 
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    )

target_link_libraries(retinanet PUBLIC TensorRT opencv_core cuda)

### Installation stuff
# *.a + export found target
set(lib_dest lib/retinanet)
install(TARGETS retinanet EXPORT retinanet DESTINATION ${lib_dest})
install(TARGETS TensorRT EXPORT retinanet)
install(TARGETS cuda EXPORT retinanet)
install(EXPORT retinanet DESTINATION ${lib_dest})

# for find_package
install(FILES retinanet-config.cmake DESTINATION ${lib_dest})

# headers
install(FILES ${retinanet_headers} DESTINATION include/retinanet)
