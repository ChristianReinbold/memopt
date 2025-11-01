cmake_minimum_required(VERSION 3.25)
project(memopt VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Release DXC does not work out of the box. Use a custom DXC build with a minor constexpr hack
# and nested aliases for convenience.
#
# Patch see: 0001-make-constexpr-and-nested-aliases-available.patch
#
# If required, one can work around the limitations of the DXC release version. It is a bit annoying though,
# so I quickly hacked the compiler for experiments.
set(DXC_EXECUTABLE "C:/dev/repositories/dxc/build/bin/dxc.exe")

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(SOURCES
	${SOURCE_DIR}/StorageAccess/_Assert.hpp
	${SOURCE_DIR}/StorageAccess/_Internal.hpp
	${SOURCE_DIR}/StorageAccess/_Print.hpp
	${SOURCE_DIR}/StorageAccess/Attributes.hpp
	${SOURCE_DIR}/StorageAccess/BasicBlock.hpp
	${SOURCE_DIR}/StorageAccess/EmptyBlock.hpp
	${SOURCE_DIR}/StorageAccess/PackFunctions.hpp
	${SOURCE_DIR}/StorageAccess/Sequence.hpp
	${SOURCE_DIR}/StorageAccess/StorageAccess.hpp
	${SOURCE_DIR}/StorageAccess/Switch.hpp
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/main.RayGen.txt
	COMMAND ${DXC_EXECUTABLE}
	ARGS
	-T lib_6_4 -spirv -fspv-target-env=vulkan1.2 -HV 2021
	-Wno-c++14-extensions
	-O3
	-I ${SOURCE_DIR}
	-I ${SOURCE_DIR}/hlsl
	${SOURCE_DIR}/main.hlsl
	-Fc ${CMAKE_CURRENT_BINARY_DIR}/main.RayGen.txt
	DEPENDS ${SOURCE_DIR}/main.hpp ${SOURCES}
)

add_custom_target(shader ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/main.RayGen.txt
)

add_executable(memopt ${SOURCE_DIR}/main.cpp)
target_include_directories(memopt PRIVATE
	${SOURCE_DIR}
	${SOURCE_DIR}/cpp)
add_dependencies(memopt shader)