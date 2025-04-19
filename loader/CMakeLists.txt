cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
cmake_policy(SET CMP0097 NEW)

project(geode-loader VERSION ${GEODE_VERSION} LANGUAGES C CXX)

add_subdirectory(core)

# I think CMake is trying to link zlib from the host system?
# This prevents it from doing that.
if (WIN32 AND CMAKE_CROSSCOMPILING)
	set(MZ_FORCE_FETCH_LIBS ON)
endif()

# Package info file for internal representation
set(GEODE_RESOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resources)
configure_file(resources/mod.json.in ${CMAKE_CURRENT_SOURCE_DIR}/resources/mod.json)
file(READ resources/mod.json LOADER_MOD_JSON)
configure_file(${GEODE_ROOT_PATH}/VERSION ${GEODE_RESOURCES_PATH}/version COPYONLY)
configure_file(${GEODE_RESOURCES_PATH}/about.md.in ${GEODE_RESOURCES_PATH}/about.md NEWLINE_STYLE LF)
configure_file(${GEODE_ROOT_PATH}/CHANGELOG.md ${GEODE_RESOURCES_PATH}/changelog.md NEWLINE_STYLE LF)
configure_file(${GEODE_RESOURCES_PATH}/support.md.in ${GEODE_RESOURCES_PATH}/support.md NEWLINE_STYLE LF)

# Source files
file(GLOB SOURCES CONFIGURE_DEPENDS 
	# src/cocos2d-ext/*.cpp
	# src/cocos2d-ext/zip/*.cpp
	# src/hooks/*.cpp
	# src/ids/*.cpp
	# src/utils/*.cpp
	# src/ui/*.cpp
	# src/ui/nodes/*.cpp
	# src/ui/other/*.cpp
	# src/ui/mods/*.cpp
	# src/ui/mods/list/*.cpp
	# src/ui/mods/popups/*.cpp
	# src/ui/mods/events/*.cpp
	# src/ui/mods/sources/*.cpp
	# src/ui/mods/settings/*.cpp
	# src/ui/mods/test/*.cpp
	# src/ui/*.cpp
	# src/c++stl/*.cpp
	hash/hash.cpp
)

# Obj-c sources
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
	file(GLOB OBJC_SOURCES CONFIGURE_DEPENDS
		src/hooks/AddExtraKeys.mm
	)
else()
	file(GLOB OBJC_SOURCES CONFIGURE_DEPENDS
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

elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
	file(GLOB IOS_SOURCES CONFIGURE_DEPENDS
		src/cocos2d-ext/Cocos2d.cpp # identical on ios, so we just use the mac one
	)
	list(APPEND SOURCES ${IOS_SOURCES})
	list(APPEND SOURCES ${OBJC_SOURCES})

elseif(APPLE)
	file(GLOB MAC_SOURCES CONFIGURE_DEPENDS
		src/cocos2d-ext/Cocos2d.cpp # identical on ios, so we just use the mac one
	)
	list(APPEND SOURCES ${MAC_SOURCES})
	list(APPEND SOURCES ${OBJC_SOURCES})

elseif(ANDROID)

endif()

# # Embed version info in binary
# if (WIN32)
# 	configure_file(src/platform/windows/info.rc.in info.rc)
# 	set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/info.rc)
# endif()


add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

file(MAKE_DIRECTORY "${GEODE_BIN_PATH}/nightly")

# Prevent SDK from activating a loader build
set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	OUTPUT_NAME ${GEODE_OUTPUT_NAME}
	# i'm going to say a slur that not a single soul has uttered in the last 200 years
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GEODE_BIN_PATH}/nightly"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GEODE_BIN_PATH}/nightly"
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GEODE_BIN_PATH}/nightly"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${GEODE_BIN_PATH}/nightly"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GEODE_BIN_PATH}/nightly"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${GEODE_BIN_PATH}/nightly"
	LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${GEODE_BIN_PATH}/nightly"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${GEODE_BIN_PATH}/nightly"
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${GEODE_BIN_PATH}/nightly"
	LIBRARY_OUTPUT_DIRECTORY "${GEODE_BIN_PATH}/nightly"
	RUNTIME_OUTPUT_DIRECTORY "${GEODE_BIN_PATH}/nightly"
	ARCHIVE_OUTPUT_DIRECTORY "${GEODE_BIN_PATH}/nightly"
)

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

# Package resources for UI
package_geode_resources_now(
	${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/resources
	${GEODE_BIN_PATH}/nightly/resources
	${CMAKE_CURRENT_SOURCE_DIR}/src/internal/resources.hpp
)

if (APPLE) 
	# I don't care a single bit where to put this
	file(COPY ${GEODE_LOADER_PATH}/include/link/macos/libfmod.dylib DESTINATION ${GEODE_BIN_PATH}/nightly)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	src/
	core/src/
	core/src/loader/
	core/src/internal/
	core/src/platform/
	src/gui/
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	GEODE_EXPOSE_SECRET_INTERNALS_IN_HEADERS_DO_NOT_DEFINE_PLEASE
	_CRT_SECURE_NO_WARNINGS
)

# set GEODE_MOD_ID for loader itself
target_compile_definitions(${PROJECT_NAME} PRIVATE GEODE_MOD_ID="geode.loader")

# These are only needed for building source :-)
if (NOT GEODE_BUILDING_DOCS)
	# Markdown support
	CPMAddPackage("gh:mity/md4c#release-0.5.2")

	target_include_directories(${PROJECT_NAME} PRIVATE ${md4c_SOURCE_DIR}/src)

	target_link_libraries(${PROJECT_NAME} md4c)
endif()

target_link_libraries(${PROJECT_NAME} geode-core)

if (MSVC)
	# Disable outputting .exp file
	include(CheckLinkerFlag)
	check_linker_flag(CXX /NOEXP SUPPORTS_NOEXP)
	if (SUPPORTS_NOEXP)
		target_link_options(${PROJECT_NAME} PRIVATE /NOEXP)
	endif()
endif()

if (WIN32)
	# allows forward compat on windows
	if (MSVC)
		target_link_options(${PROJECT_NAME} PRIVATE /DELAYLOAD:libcocos2d.dll /DELAYLOAD:libExtensions.dll)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_link_options(${PROJECT_NAME} PRIVATE "-Wl,/delayload:libcocos2d.dll" "-Wl,/delayload:libExtensions.dll")
	else()
		message(FATAL_ERROR "Unable to set DELAYLOAD! This is essential for Geode forward compatibility to work.")
	endif()
endif()

# Use precompiled headers for faster builds
if (NOT GEODE_DISABLE_PRECOMPILED_HEADERS)
	target_precompile_headers(${PROJECT_NAME} PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/include/Geode/Geode.hpp"
	)
endif()

# Create launcher
if (APPLE)

	if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
		# Used for File Picker API
		find_library(UNIFORM_TYPE_IDENTIFIERS_FRAMEWORK UniformTypeIdentifiers)
		target_link_libraries(${PROJECT_NAME} ${UNIFORM_TYPE_IDENTIFIERS_FRAMEWORK})
	else()
		set_target_properties(geode-loader PROPERTIES 
			SYSTEM_NAME MacOS
			OSX_DEPLOYMENT_TARGET 10.15
			APPLE_SILICON_PROCESSOR x86_64
		)

		# geodebootstrapper is unused on ios
		add_subdirectory(launcher/mac)
		set(LAUNCHER_TARGET GeodeBootstrapper)
	endif()
elseif (WIN32)
	add_subdirectory(launcher/windows)

	target_link_libraries(${PROJECT_NAME} dbghelp winmm)

	target_compile_definitions(${PROJECT_NAME} PUBLIC ISOLATION_AWARE_ENABLED=1)

	if (MSVC)
		# disable warnings about CCNode::setID
		target_link_options(${PROJECT_NAME} PUBLIC /ignore:4217)
	endif()

	set(LAUNCHER_TARGET ProxyLoader Updater)
endif()

# add_subdirectory(test)

# Add install target on CLI >= 2.10.0 (which adds `geode profile path`)
if (NOT GEODE_BUILDING_DOCS)
	# nest this because when building docs GEODE_CLI_VERSION is not defined
	if (${GEODE_CLI_VERSION} VERSION_GREATER_EQUAL "2.10.0")
		execute_process(
			COMMAND ${GEODE_CLI} profile path -d
			OUTPUT_VARIABLE GEODE_PROFILE_GD_PATH
			RESULT_VARIABLE GEODE_PROFILE_GD_PATH_RESULT
		)
		if (${GEODE_PROFILE_GD_PATH_RESULT} EQUAL 0)
			string(STRIP ${GEODE_PROFILE_GD_PATH} GEODE_PROFILE_GD_PATH)
			install(
				TARGETS geode-loader ${LAUNCHER_TARGET}
				RUNTIME
				DESTINATION "${GEODE_PROFILE_GD_PATH}"
			)
			file(GLOB RESOURCE_FILES ${GEODE_BIN_PATH}/nightly/resources/*.*)
			install(
				FILES ${RESOURCE_FILES}
				DESTINATION "${GEODE_PROFILE_GD_PATH}/geode/resources/geode.loader"
			)
		endif()
	endif()
endif()
