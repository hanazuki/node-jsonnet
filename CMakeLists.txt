cmake_minimum_required(VERSION 3.5)

project (node-jsonnet)
include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "binding/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

execute_process(COMMAND node -p "require('node-addon-api').include.replace(/^\"(.*)\"$/, '\$1')"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/jsonnet/include")

set(BUILD_TESTS OFF CACHE BOOL "Disable jsonnet tests")
add_subdirectory(jsonnet)
set_target_properties(libjsonnet++ libjsonnet++_static jsonnet jsonnetfmt
  PROPERTIES EXCLUDE_FROM_ALL ON
  )

