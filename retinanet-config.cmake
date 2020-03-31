set(RETINANET_BUILD_TYPE ""
    CACHE STRING
    "Can be one of 'Release;RelWithDebInfo;Debug', default to CMAKE_BUILD_TYPE if not set, defaulting to RelWithDebInfo as a last resort")

if (RETINANET_BUILD_TYPE STREQUAL "")
    message(VERBOSE "RETINANET_BUILD_TYPE is empty, looking at CMAKE_BUILD_TYPE")
    if(CMAKE_BUILD_TYPE STREQUAL "")
        message(VERBOSE "CMAKE_BUILD_TYPE is empty, defaulting to RelWithDebInfo")
        set(BUILD_TYPE RelWithDebInfo)
    else()
        message(VERBOSE "CMAKE_BUILD_TYPE is set to: ${CMAKE_BUILD_TYPE}")
        set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
    endif()
else()
    message(VERBOSE "RETINANET_BUILD_TYPE is set to: ${RETINANET_BUILD_TYPE}")
    set(BUILD_TYPE ${RETINANET_BUILD_TYPE})
endif()

message(STATUS "Using retinanet built as: ${BUILD_TYPE}")
set(retinanet_real retinanet_${BUILD_TYPE})

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/${BUILD_TYPE}/${retinanet_real}.cmake)
set_target_properties(${retinanet_real} PROPERTIES IMPORTED_GLOBAL TRUE)
add_library(retinanet ALIAS ${retinanet_real})
