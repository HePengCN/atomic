project(xqueue)
include_directories(./include)
aux_source_directory(. XQUEUE_SRCS)
add_library(${PROJECT_NAME} STATIC ${XQUEUE_SRCS})
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pthread -DCOMPILE_WITH_SYSTEM")
target_link_libraries(${PROJECT_NAME} log)

