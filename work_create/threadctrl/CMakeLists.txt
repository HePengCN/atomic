project(threadctrl)
include_directories(./include)
aux_source_directory(. THD_CTRL_SRCS)
add_library(${PROJECT_NAME} STATIC ${THD_CTRL_SRCS})
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pthread -DCOMPILE_WITH_SYSTEM")
target_link_libraries(${PROJECT_NAME} log2c)

