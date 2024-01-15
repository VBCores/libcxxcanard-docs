cmake_minimum_required(VERSION 3.16)
project(hbeat_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

if(DEFINED ENV{<CYPHAL_DIR>})
    set(CYPHAL_DIR $ENV{CYPHAL_DIR})
else()
    get_filename_component(CYPHAL_DIR
                           "../../libcyphal"
                           ABSOLUTE)
endif()
message(STATUS "${CMAKE_CURRENT_LIST_DIR} | Using <${CYPHAL_DIR}> as directory of libcyphal")

add_subdirectory(${CYPHAL_DIR} ${PROJECT_BINARY_DIR}/build/libcyphal)

add_executable(hbeat_example.run heartbeat_sub.cpp)

target_include_directories(hbeat_example.run PUBLIC
  ${CYPHAL_DIR}
  ${COMMON_LIBS_DIR}
)

target_link_libraries(hbeat_example.run libcyphal)
