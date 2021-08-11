# Utility to convert file to a list of integeters.

add_executable(to_c_array to_c_array.cpp)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_isMultiConfig)
  message("generator is multi config!!!!")
	set(TO_C_ARRAY ${GLOBAL_OUTPUT_PATH}/$<CONFIG>/to_c_array${CMAKE_EXECUTABLE_SUFFIX})
else()
  message("generator is not multi config!!!!")
	set(TO_C_ARRAY ${GLOBAL_OUTPUT_PATH}/to_c_array${CMAKE_EXECUTABLE_SUFFIX})
endif()

message(${GLOBAL_OUTPUT_PATH})
message(${TO_C_ARRAY})

# Custom command that will only build stdlib when it changes.
add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/core/std.jsonnet.h
	COMMAND ${TO_C_ARRAY}
			    ${PROJECT_SOURCE_DIR}/stdlib/std.jsonnet
					${PROJECT_SOURCE_DIR}/core/std.jsonnet.h
	DEPENDS to_c_array std.jsonnet)

# Standard library build target that libjsonnet can depend on.
add_custom_target(stdlib ALL
	DEPENDS ${PROJECT_SOURCE_DIR}/core/std.jsonnet.h)
