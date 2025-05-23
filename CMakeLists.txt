cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0042 NEW)

project (node-jsonnet)
file(GLOB SOURCE_FILES "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} libjsonnet)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_compile_definitions(${PROJECT_NAME} PRIVATE "NAPI_VERSION=6" "NODE_ADDON_API_DISABLE_DEPRECATED")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-pedantic")
endif()

if(DEFINED ENV{NODE_JSONNET_ENABLE_COVERAGE})
  target_link_options(${PROJECT_NAME} PRIVATE --coverage)
  target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
endif()

set(BUILD_TESTS OFF CACHE BOOL "Disable jsonnet tests")
add_subdirectory(third_party/jsonnet EXCLUDE_FROM_ALL)
