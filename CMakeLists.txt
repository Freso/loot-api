cmake_minimum_required (VERSION 2.8.12.1)
cmake_policy(SET CMP0015 NEW)
project (loot_api)
include(ExternalProject)

option(BUILD_SHARED_LIBS "Build a shared library" ON)
option(MSVC_STATIC_RUNTIME "Build with static runtime libs (/MT)" OFF)

IF (${MSVC_STATIC_RUNTIME})
    set (MSVC_SHARED_RUNTIME OFF)
ELSE()
    set (MSVC_SHARED_RUNTIME ON)
ENDIF()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

##############################
# Get Build Revision
##############################

find_package(Git)

IF (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE GIT_COMMIT_STRING
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE()
    SET (GIT_COMMIT_STRING "unknown")
ENDIF ()

message(STATUS "Git revision: ${GIT_COMMIT_STRING}")

# Write to file.
configure_file("${CMAKE_SOURCE_DIR}/src/api/loot_version.cpp.in" "${CMAKE_BINARY_DIR}/generated/loot_version.cpp" @ONLY)

##############################
# External Projects
##############################

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

IF (MSVC)
    set (Boost_USE_STATIC_RUNTIME ${MSVC_STATIC_RUNTIME})
ELSE()
    set (Boost_USE_STATIC_RUNTIME OFF)
ENDIF()

IF (NOT Boost_USE_STATIC_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
ENDIF ()

find_package(Boost REQUIRED COMPONENTS atomic log log_setup regex locale thread date_time chrono filesystem system iostreams)

ExternalProject_Add(GTest
                    PREFIX "external"
                    URL "https://github.com/google/googletest/archive/release-1.8.0.tar.gz"
                    CMAKE_ARGS -Dgtest_force_shared_crt=${MSVC_SHARED_RUNTIME}
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(GTest SOURCE_DIR BINARY_DIR)
set (GTEST_INCLUDE_DIRS "${SOURCE_DIR}/googletest/include")
set (GTEST_LIBRARIES "${BINARY_DIR}/googlemock/gtest/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(libespm
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/libespm/archive/2.5.5.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libespm SOURCE_DIR)
set (LIBESPM_INCLUDE_DIRS "${SOURCE_DIR}/include")

ExternalProject_Add(libgit2
                    PREFIX "external"
                    URL "https://github.com/libgit2/libgit2/archive/v0.26.0.tar.gz"
                    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DSTATIC_CRT=${MSVC_STATIC_RUNTIME} -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target git2 --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libgit2 SOURCE_DIR BINARY_DIR)
set(LIBGIT2_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(LIBGIT2_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}git2${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(libloadorder
                    PREFIX "external"
                    DEPENDS libespm
                    URL "https://github.com/WrinklyNinja/libloadorder/archive/9.5.4.tar.gz"
                    CMAKE_ARGS -DBOOST_ROOT=${BOOST_ROOT} -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} -DMSVC_STATIC_RUNTIME=${MSVC_STATIC_RUNTIME} -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target loadorder --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(libloadorder SOURCE_DIR BINARY_DIR)
set(LIBLOADORDER_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(LIBLOADORDER_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}loadorder${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(pseudosem
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/pseudosem/archive/1.1.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(pseudosem SOURCE_DIR)
set(PSEUDOSEM_INCLUDE_DIRS "${SOURCE_DIR}/include")

ExternalProject_Add(testing-metadata
                    PREFIX "external"
                    URL "https://github.com/loot/testing-metadata/archive/1.2.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

ExternalProject_Add(testing-plugins
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/testing-plugins/archive/1.0.0.tar.gz"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND "")

ExternalProject_Add(yaml-cpp
                    PREFIX "external"
                    URL "https://github.com/WrinklyNinja/yaml-cpp/archive/patched-for-loot.tar.gz"
                    CMAKE_ARGS -DMSVC_SHARED_RT=${MSVC_SHARED_RUNTIME} -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
                    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target yaml-cpp --config $(CONFIGURATION)
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(yaml-cpp SOURCE_DIR BINARY_DIR)
set(YAML_CPP_INCLUDE_DIRS "${SOURCE_DIR}/include")
set(YAML_CPP_LIBRARIES "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/libyaml-cpp${CMAKE_STATIC_LIBRARY_SUFFIX}")

##############################
# General Settings
##############################


set (LOOT_API_SRC "${CMAKE_BINARY_DIR}/generated/loot_version.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/api.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/api_database.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/error_categories.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_evaluator.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/conditional_metadata.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/file.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/location.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/message.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/message_content.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/plugin_cleaning_data.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/plugin_metadata.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/priority.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata/tag.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/game.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/game_cache.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/game/load_order_handler.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/metadata_list.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/masterlist.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin_sorter.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/crc.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/git_helper.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/helpers/version.cpp"
                  "${CMAKE_SOURCE_DIR}/src/api/resource.rc")

set (LOOT_API_HEADERS "${CMAKE_SOURCE_DIR}/include/loot/api.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/api_decorator.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/database_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/error_categories.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/condition_syntax_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/cyclic_interaction_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/file_access_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/exception/git_state_error.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/game_type.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/log_verbosity.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/enum/message_type.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/game_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/loot_version.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/conditional_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/file.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/location.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/message_content.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/plugin_cleaning_data.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/plugin_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/priority.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/metadata/tag.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/file.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/location.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/message_content.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/plugin_cleaning_data.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/plugin_metadata.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/set.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/yaml/tag.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/plugin_interface.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/struct/masterlist_info.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/struct/simple_message.h"
                      "${CMAKE_SOURCE_DIR}/include/loot/windows_encoding_converters.h"
                      "${CMAKE_SOURCE_DIR}/src/api/api_database.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_evaluator.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata/condition_grammar.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/game.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/game_cache.h"
                      "${CMAKE_SOURCE_DIR}/src/api/game/load_order_handler.h"
                      "${CMAKE_SOURCE_DIR}/src/api/metadata_list.h"
                      "${CMAKE_SOURCE_DIR}/src/api/masterlist.h"
                      "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin.h"
                      "${CMAKE_SOURCE_DIR}/src/api/plugin/plugin_sorter.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/git_helper.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/crc.h"
                      "${CMAKE_SOURCE_DIR}/src/api/helpers/version.h")

set (LOOT_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/tests/api/internals/main.cpp")

set (LOOT_TESTS_HEADERS "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/game_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/game_cache_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/game/load_order_handler_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/git_helper_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/crc_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/version_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/helpers/yaml_set_helpers_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/condition_evaluator_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/condition_grammar_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/conditional_metadata_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/file_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/location_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/message_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/message_content_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/plugin_cleaning_data_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/plugin_metadata_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/priority_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata/tag_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/plugin/plugin_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/plugin/plugin_sorter_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/masterlist_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/api/internals/metadata_list_test.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/common_game_test_fixture.h"
                        "${CMAKE_SOURCE_DIR}/src/tests/printers.h")

set(LOOT_API_TESTS_SRC "${CMAKE_SOURCE_DIR}/src/tests/api/interface/main.cpp")

set(LOOT_API_TESTS_HEADERS  "${CMAKE_SOURCE_DIR}/src/tests/api/interface/api_game_operations_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/create_game_handle_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/database_interface_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/game_interface_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/api/interface/is_compatible_test.h"
                            "${CMAKE_SOURCE_DIR}/src/tests/common_game_test_fixture.h")

source_group("Header Files\\api" FILES ${LOOT_API_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_TESTS_HEADERS})
source_group("Header Files\\tests" FILES ${LOOT_API_TESTS_HEADERS})

source_group("Source Files\\api" FILES ${LOOT_API_SRC})
source_group("Source Files\\tests" FILES ${LOOT_TESTS_SRC})
source_group("Source Files\\tests" FILES ${LOOT_API_TESTS_SRC})

# Include source and library directories.
include_directories ("${CMAKE_SOURCE_DIR}/src"
                     "${CMAKE_SOURCE_DIR}/include"
                     ${LIBLOADORDER_INCLUDE_DIRS}
                     ${LIBGIT2_INCLUDE_DIRS}
                     ${LIBESPM_INCLUDE_DIRS}
                     ${Boost_INCLUDE_DIRS}
                     ${YAML_CPP_INCLUDE_DIRS}
                     ${GTEST_INCLUDE_DIRS}
                     ${PSEUDOSEM_INCLUDE_DIRS})

##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -DLIBLO_STATIC)
ENDIF ()

IF (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};.")
    set (CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++14")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14")

    set (LOOT_LIBS ssl
                   curl
                   z
                   crypto
                   rt)
ENDIF ()

IF (MSVC)
    # Set /bigobj to allow building Debug tests
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")

    # Update compiler flags.
    IF (MSVC_STATIC_RUNTIME)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc")
        ENDFOREACH()
    ENDIF ()

    set (LOOT_LIBS  version
                    ws2_32
                    shlwapi
                    winhttp
                    crypt32
                    Rpcrt4)
ENDIF ()


##############################
# Define Targets
##############################

# Build tests.
add_executable       (loot_api_internals_tests ${LOOT_API_SRC} ${LOOT_API_HEADERS} ${LOOT_TESTS_SRC} ${LOOT_TESTS_HEADERS})
add_dependencies     (loot_api_internals_tests libespm libgit2 libloadorder pseudosem yaml-cpp GTest testing-metadata testing-plugins)
target_link_libraries(loot_api_internals_tests ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES} ${LIBLOADORDER_LIBRARIES} ${LOOT_LIBS} ${YAML_CPP_LIBRARIES} ${GTEST_LIBRARIES} ${LIBLOADORDER_LIBRARIES})

# Build API.
add_library          (loot_api ${LOOT_API_SRC} ${LOOT_API_HEADERS})
add_dependencies     (loot_api libespm libgit2 libloadorder pseudosem yaml-cpp)
target_link_libraries(loot_api ${Boost_LIBRARIES} ${LIBGIT2_LIBRARIES} ${LIBLOADORDER_LIBRARIES} ${LOOT_LIBS} ${YAML_CPP_LIBRARIES})

# Build API tests.
add_executable       (loot_api_tests ${LOOT_API_TESTS_SRC} ${LOOT_API_TESTS_HEADERS})
add_dependencies     (loot_api_tests loot_api GTest testing-metadata testing-plugins)
target_link_libraries(loot_api_tests loot_api ${GTEST_LIBRARIES})

##############################
# Set Target-Specific Flags
##############################

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set_target_properties (loot_api_internals_tests PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_STATIC")
    IF (BUILD_SHARED_LIBS)
        set_target_properties (loot_api PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_EXPORT")
    ELSE ()
        set_target_properties (loot_api PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} LOOT_STATIC")
    ENDIF ()
ENDIF ()


##############################
# Post-Build Steps
##############################

# Copy testing metadata
ExternalProject_Get_Property(testing-metadata SOURCE_DIR)
add_custom_command(TARGET loot_api_internals_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        "$<TARGET_FILE_DIR:loot_api_internals_tests>/testing-metadata")
add_custom_command(TARGET loot_api_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        "$<TARGET_FILE_DIR:loot_api_tests>/testing-metadata")

# Copy testing plugins
ExternalProject_Get_Property(testing-plugins SOURCE_DIR)
add_custom_command(TARGET loot_api_internals_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
        $<TARGET_FILE_DIR:loot_api_internals_tests>)
add_custom_command(TARGET loot_api_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_DIR}
      $<TARGET_FILE_DIR:loot_api_tests>)

########################################
# Install
########################################

install(TARGETS loot_api
        DESTINATION ".")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include"
        DESTINATION ".")

install(DIRECTORY "${CMAKE_BINARY_DIR}/docs/html/"
        DESTINATION "docs")

########################################
# CPack
########################################

IF (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --long --always --abbrev=7
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE GIT_DESCRIBE_STRING
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF (DEFINED ENV{APPVEYOR_REPO_BRANCH})
        set(GIT_DESCRIBE_STRING "${GIT_DESCRIBE_STRING}_$ENV{APPVEYOR_REPO_BRANCH}")
    ELSEIF (DEFINED ENV{TRAVIS_BRANCH})
        set(GIT_DESCRIBE_STRING "${GIT_DESCRIBE_STRING}_$ENV{TRAVIS_BRANCH}")
    ENDIF()
ELSE()
    SET (GIT_DESCRIBE_STRING "unknown-version")
ENDIF ()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_GENERATOR "7Z")
else()
    set(CPACK_GENERATOR "TXZ")
endif()

set(CPACK_PACKAGE_VERSION ${GIT_DESCRIBE_STRING})
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")

include(CPack)
