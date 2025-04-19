cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
cmake_policy(SET CMP0097 NEW)

project(geode-core VERSION ${GEODE_VERSION} LANGUAGES C CXX)
if (GEODE_VERSION_TAG_TYPE)
	if (GEODE_VERSION_TAG_NUMBER)
		set(PROJECT_VERSION_TAG_CONSTR "geode::VersionTag(geode::VersionTag::${GEODE_VERSION_TAG_TYPE}, ${GEODE_VERSION_TAG_NUMBER})")
	else()
		set(PROJECT_VERSION_TAG_CONSTR "geode::VersionTag::${GEODE_VERSION_TAG_TYPE}")
	endif()
else()
	set(PROJECT_VERSION_TAG_CONSTR "std::nullopt")
endif()

if (GEODE_VERSION_TAG)
	set(PROJECT_VERSION_SUFFIX "-${GEODE_VERSION_TAG}" CACHE INTERNAL "" )
else()
	set(PROJECT_VERSION_SUFFIX "" CACHE INTERNAL "" )
endif()

# https://stackoverflow.com/a/63924044/9124836
# https://stackoverflow.com/a/72396471
execute_process(
	COMMAND git describe --always --dirty
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GEODE_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)

if (NOT GEODE_COMMIT_HASH)
	set(GEODE_COMMIT_HASH "(?)")
endif()

execute_process(
	COMMAND git describe --always --dirty
	WORKING_DIRECTORY ${GEODE_BINDINGS_REPO_PATH}
	OUTPUT_VARIABLE GEODE_BINDINGS_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)

if (NOT GEODE_BINDINGS_COMMIT_HASH)
	set(GEODE_BINDINGS_COMMIT_HASH "(?)")
endif()

# Package info file for internal representation
configure_file(src/internal/about.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/internal/about.cpp)

# Source files
file(GLOB SOURCES CONFIGURE_DEPENDS 
	src/core/*.cpp
	src/internal/*.cpp
	src/server/*.cpp
	src/loader/*.cpp
	src/loader/sources/*.cpp
	src/load.cpp
	src/utils/*.cpp
	hash/hash.cpp
)

# Obj-c sources
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
	file(GLOB OBJC_SOURCES CONFIGURE_DEPENDS
		src/platform/ios/*.mm
		src/load.mm
		src/hooks/AddExtraKeys.mm
	)
else()
	file(GLOB OBJC_SOURCES CONFIGURE_DEPENDS
		src/platform/mac/*.mm
		src/load.mm
		src/hooks/AddExtraKeys.mm
	)
endif()
set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if (WIN32)
	# This allows you to compile in debug mode
	add_compile_definitions(_HAS_ITERATOR_DEBUGGING=0)
	add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
endif()


# Add platform sources
if (WIN32)
	file(GLOB WIN_SOURCES CONFIGURE_DEPENDS 
		src/platform/windows/*.cpp
	)
	list(APPEND SOURCES ${WIN_SOURCES})

elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
	file(GLOB IOS_SOURCES CONFIGURE_DEPENDS
		src/platform/ios/*.cpp
		src/platform/mac/Cocos2d.cpp # identical on ios, so we just use the mac one
	)
	list(APPEND SOURCES ${IOS_SOURCES})
	list(APPEND SOURCES ${OBJC_SOURCES})

elseif(APPLE)
	file(GLOB MAC_SOURCES CONFIGURE_DEPENDS
		src/platform/mac/*.cpp
	)
	list(APPEND SOURCES ${MAC_SOURCES})
	list(APPEND SOURCES ${OBJC_SOURCES})

elseif(ANDROID)
	file(GLOB ANDROID_SOURCES CONFIGURE_DEPENDS
		src/platform/android/*.cpp
	)
	list(APPEND SOURCES ${ANDROID_SOURCES})

endif()

# Embed version info in binary
if (WIN32)
	configure_file(src/platform/windows/info.rc.in info.rc)
	set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/info.rc)
endif()


if (ANDROID AND GEODE_USE_BREAKPAD)
	CPMAddPackage("gh:qimiko/breakpad#169df6a")
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if (GEODE_NO_UNDEFINED_VIRTUALS)
	target_compile_definitions(${PROJECT_NAME} PUBLIC GEODE_NO_UNDEFINED_VIRTUALS)
endif()

# CA Bundle
CPMAddPackage(
	GITHUB_REPOSITORY geode-sdk/net_libs
	GIT_TAG 1f5ffb3
# CPM is bugged and will not allow an empty string.
# https://github.com/cpm-cmake/CPM.cmake/issues/467
# https://github.com/cpm-cmake/CPM.cmake/issues/461
	GIT_SUBMODULES "nghttp3"
	GIT_SUBMODULES_RECURSE OFF
)
target_link_libraries(${PROJECT_NAME} ca-bundle)

if (ANDROID)
	if (GEODE_USE_BREAKPAD)
		target_compile_definitions(${PROJECT_NAME} PRIVATE -DGEODE_USE_BREAKPAD)
		target_link_libraries(${PROJECT_NAME} Breakpad)
	endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	src/
	src/loader/
	src/internal/
	src/platform/
	./
)

if (APPLE)
	# For profiling
	target_compile_options(${PROJECT_NAME} PUBLIC "-ftime-trace")
	# target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-e,_dynamicInit")
	#set_property(TARGET ${PROJECT_NAME} PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC GEODE_EXPORTING MAT_JSON_EXPORTING)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	GEODE_EXPOSE_SECRET_INTERNALS_IN_HEADERS_DO_NOT_DEFINE_PLEASE
	_CRT_SECURE_NO_WARNINGS
)

# set GEODE_MOD_ID for loader itself
target_compile_definitions(${PROJECT_NAME} PRIVATE GEODE_MOD_ID="geode.loader")

# These are only needed for building source :-)
if (NOT GEODE_BUILDING_DOCS)
	# Zip support (needed for in-memory streams, which zlib's minizip doesn't support)
	set(MZ_LZMA OFF CACHE INTERNAL "")
	set(MZ_ZSTD OFF CACHE INTERNAL "")
	set(MZ_BZIP2 OFF CACHE INTERNAL "")
	set(MZ_OPENSSL OFF CACHE INTERNAL "")
	set(MZ_LIBBSD OFF CACHE INTERNAL "")
	set(MZ_FETCH_LIBS ON CACHE INTERNAL "" FORCE)
	set(MZ_FORCE_FETCH_LIBS ${WIN32} CACHE INTERNAL "")
	set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
	CPMAddPackage("gh:geode-sdk/minizip-ng#43d9254")
	if (WIN32)
		set_target_properties(zlib PROPERTIES SUFFIX "1.dll")
	endif()

	# Silence warnings from dependencies
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag(-Wno-everything SUPPORTS_W_NO_EVERYTHING)
	if (SUPPORTS_W_NO_EVERYTHING)
		if (TARGET md4c)
			target_compile_options(md4c PRIVATE -Wno-everything)
		endif()
		if (TARGET zlib)
			target_compile_options(zlib PRIVATE -Wno-everything)
		endif()
		if (TARGET zlibstatic)
			get_property(zlibstatic_ALIASED TARGET zlibstatic PROPERTY ALIASED_TARGET)
			if ("${zlibstatic_ALIASED}" STREQUAL "")
				target_compile_options(zlibstatic PRIVATE -Wno-everything)
			endif()
		endif()
	endif()

	# Cool date library
	CPMAddPackage("gh:HowardHinnant/date#1ead671")

	target_include_directories(${PROJECT_NAME} PRIVATE ${md4c_SOURCE_DIR}/src)

	target_link_libraries(${PROJECT_NAME} minizip date)
endif()

target_link_libraries(${PROJECT_NAME} TulipHook mat-json-impl)

# Required by curl for macos
if (APPLE)
	target_link_libraries(${PROJECT_NAME} z)
endif()

if (MSVC)
	# Disable outputting .exp file
	include(CheckLinkerFlag)
	check_linker_flag(CXX /NOEXP SUPPORTS_NOEXP)
	if (SUPPORTS_NOEXP)
		target_link_options(${PROJECT_NAME} PRIVATE /NOEXP)
	endif()
endif()

# Use precompiled headers for faster builds
if (NOT GEODE_DISABLE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/include/Geode/Core.hpp"
	)
endif()
