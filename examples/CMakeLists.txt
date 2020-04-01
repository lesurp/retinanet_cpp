set(CMAKE_CXX_STANDARD 17)

find_package(fmt CONFIG REQUIRED)
find_package(OpenCV
    CONFIG
    REQUIRED
        opencv_core
        opencv_highgui
        opencv_videoio
        opencv_imgproc
)

# This is not required once the target is installed
include_directories(${PROJECT_SOURCE_DIR}/src)

# About the `_Release` suffix:
# This is not required once the package is installed
# The import script aliases `retinanet` to the correct target
add_executable(single_image single_image.cpp)
target_link_libraries(single_image
    PRIVATE
        opencv_highgui
        retinanet_Release
)

add_executable(live live.cpp)
target_link_libraries(live
    PRIVATE
        retinanet_Release
        opencv_videoio
        opencv_highgui
        opencv_imgproc
        fmt::fmt
)
