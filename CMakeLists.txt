cmake_minimum_required(VERSION 3.8)

project (node-jsonnet)
file(GLOB SOURCE_FILES "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} libjsonnet)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-pedantic")

if(DEFINED ENV{NODE_JSONNET_ENABLE_COVERAGE})
  target_link_options(${PROJECT_NAME} PRIVATE --coverage)
  target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
endif()

execute_process(COMMAND node -p "require('node-addon-api').include.replace(/^\"(.*)\"$/, '\$1')"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsonnet/include")

set(BUILD_TESTS OFF CACHE BOOL "Disable jsonnet tests")
add_subdirectory(third_party/jsonnet EXCLUDE_FROM_ALL)
