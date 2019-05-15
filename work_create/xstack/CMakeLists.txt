project(xstack)
include_directories(./include)
aux_source_directory(. XSTACK_SRCS)
add_library(${PROJECT_NAME} STATIC ${XSTACK_SRCS})
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pthread")
target_link_libraries(${PROJECT_NAME} log)

