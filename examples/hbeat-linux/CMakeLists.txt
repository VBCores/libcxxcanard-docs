cmake_minimum_required(VERSION 3.18)
project(hbeat_example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

if(DEFINED ENV{<CYPHAL_DIR>})
    set(CYPHAL_DIR $ENV{CYPHAL_DIR})
else()
    get_filename_component(CYPHAL_DIR
                           "../../libcyphal"
                           ABSOLUTE)
endif()
message("${CMAKE_CURRENT_LIST_DIR} | Using <${CYPHAL_DIR}> as directory of libcyphal")

if(DEFINED ENV{<COMMON_LIBS_DIR>})
    set(COMMON_LIBS_DIR $ENV{COMMON_LIBS_DIR})
else()
    get_filename_component(COMMON_LIBS_DIR
            "../../libcyphal/libs"
            ABSOLUTE)
endif()
message("${CMAKE_CURRENT_LIST_DIR} | Using <${COMMON_LIBS_DIR}> as directory of common Cyphal libs - uavcan, reg, etc.")

include_directories(${CYPHAL_DIR} ${COMMON_LIBS_DIR})

add_subdirectory(${CYPHAL_DIR} ${PROJECT_BINARY_DIR}/build/libcyphal)

add_executable(hbeat_example.run heartbeat_sub.cpp)

target_link_libraries(hbeat_example.run libcyphal)
