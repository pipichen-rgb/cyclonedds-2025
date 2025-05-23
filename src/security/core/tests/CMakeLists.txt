#
# Copyright(c) 2006 to 2022 ZettaScale Technology and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#
include (GenerateExportHeader)
include (CUnit)
include (Generate)

idlc_generate(TARGET SecurityCoreTests FILES SecurityCoreTests.idl WARNINGS no-implicit-extensibility)

function(add_wrapper libname linklibs)
  set(srcs_wrapper
    "${CMAKE_CURRENT_LIST_DIR}/common/${libname}_wrapper.c"
    "${CMAKE_CURRENT_LIST_DIR}/common/plugin_wrapper_msg_q.c")
  add_library("dds_security_${libname}_wrapper")
  if(NOT BUILD_SHARED_LIBS)
    set_property(GLOBAL APPEND PROPERTY cdds_plugin_list dds_security_${libname}_wrapper)
    set_property(GLOBAL PROPERTY dds_security_${libname}_wrapper_symbols
      init_test_${libname}_wrapped finalize_test_${libname}_wrapped
      init_test_${libname}_missing_func finalize_test_${libname}_missing_func
      init_test_${libname}_all_ok finalize_test_${libname}_all_ok)
    if(libname STREQUAL authentication)
      set_property(GLOBAL APPEND PROPERTY dds_security_${libname}_wrapper_symbols
        init_test_${libname}_init_error finalize_test_${libname}_init_error)
    elseif(libname STREQUAL access_control)
      set_property(GLOBAL APPEND PROPERTY dds_security_${libname}_wrapper_symbols
        init_test_${libname}_local_participant_not_allowed
        init_test_${libname}_local_topic_not_allowed
        init_test_${libname}_local_writer_not_allowed
        init_test_${libname}_local_reader_not_allowed
        init_test_${libname}_local_permissions_not_allowed
        init_test_${libname}_remote_participant_not_allowed
        init_test_${libname}_remote_topic_not_allowed
        init_test_${libname}_remote_writer_not_allowed
        init_test_${libname}_remote_reader_not_allowed
        init_test_${libname}_remote_reader_relay_only
        init_test_${libname}_remote_permissions_not_allowed
        finalize_test_${libname}_not_allowed)
    elseif(libname STREQUAL cryptography)
      set_property(GLOBAL APPEND PROPERTY dds_security_${libname}_wrapper_symbols
        init_test_${libname}_store_tokens finalize_test_${libname}_store_tokens
        init_test_${libname}_plain_data finalize_test_${libname}_plain_data)
    endif()
  endif()
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_target_properties("dds_security_${libname}_wrapper" PROPERTIES LINK_FLAGS "/ignore:4099")
  endif()

  generate_export_header(
    "dds_security_${libname}_wrapper"
    BASE_NAME SECURITY
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/dds/security/${libname}_wrapper_export.h")

  set_target_properties(
    "dds_security_${libname}_wrapper"
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY                ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR})

  if(APPLE)
    set_target_properties("dds_security_${libname}_wrapper" PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif()

  target_link_libraries("dds_security_${libname}_wrapper" PRIVATE CycloneDDS::ucunit)
  target_include_directories("dds_security_${libname}_wrapper" PRIVATE "${CUNIT_DIR}/include")
  if(BUILD_SHARED_LIBS)
    target_link_libraries("dds_security_${libname}_wrapper" PUBLIC ddsc ${linklibs})
  endif()

  target_sources("dds_security_${libname}_wrapper" PRIVATE ${srcs_wrapper})
  target_include_directories("dds_security_${libname}_wrapper"
    PUBLIC
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_api,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_core,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:ddsrt,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../builtin_plugins/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/src>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsc/src>")
endfunction()

set(security_core_test_sources
    "fsm.c"
    "timed_cb.c"
    "security_utils.c"
)

if(ENABLE_SSL AND OPENSSL_FOUND)
  add_wrapper(access_control dds_security_ac)
  add_wrapper(authentication dds_security_auth)
  add_wrapper(cryptography dds_security_crypto)

  FIND_PROGRAM(PKCS11_TOOL_CMD pkcs11-tool)
  FIND_PROGRAM(SOFTHSM_UTIL_CMD softhsm2-util)
  if (PKCS11_TOOL_CMD AND SOFTHSM_UTIL_CMD)
    set(PKCS11_TEST_FILE "pkcs11.c")
  else()
    set(PKCS11_TEST_FILE "")
  endif()

  list(APPEND security_core_test_sources
    "common/security_config_test_utils.c"
    "common/test_utils.c"
    "common/cert_utils.c"
    "authentication.c"
    "access_control.c"
    "builtintopic.c"
    "config.c"
    "crypto.c"
    "deserialize.c"
    "handshake.c"
    "plugin_loading.c"
    "secure_communication.c"
    ${PKCS11_TEST_FILE}
    ${CYCLONEDDS_OPENSSL_APPLINK})
  # applink.c triggers the dreaded This function or variable may be unsafe
  if(NOT "${CYCLONEDDS_OPENSSL_APPLINK}" STREQUAL "")
    set_source_files_properties("${CYCLONEDDS_OPENSSL_APPLINK}" PROPERTIES COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
  endif()
endif()

add_cunit_executable(cunit_security_core ${security_core_test_sources})
add_coverage(cunit_security_core)
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set_target_properties(cunit_security_core PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

target_include_directories(
  cunit_security_core PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/include/>"
        "$<BUILD_INTERFACE:$<TARGET_PROPERTY:ddsrt,INTERFACE_INCLUDE_DIRECTORIES>>"
        "$<BUILD_INTERFACE:$<TARGET_PROPERTY:ddsc,INTERFACE_INCLUDE_DIRECTORIES>>"
        "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_core,INTERFACE_INCLUDE_DIRECTORIES>>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsc/src>"
  )
if(ENABLE_SSL AND OPENSSL_FOUND)
  target_include_directories(
    cunit_security_core PRIVATE
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_openssl,INTERFACE_INCLUDE_DIRECTORIES>>"
  )
endif()

set(common_etc_dir "${CMAKE_CURRENT_SOURCE_DIR}/common/etc")
set(plugin_wrapper_lib_dir "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("common/config_env.h.in" "common/config_env.h")

target_link_libraries(cunit_security_core PRIVATE ddsc security_api SecurityCoreTests)
if(ENABLE_SSL AND OPENSSL_FOUND)
  target_link_libraries(cunit_security_core PRIVATE dds_security_auth dds_security_ac dds_security_crypto dds_security_access_control_wrapper dds_security_authentication_wrapper dds_security_cryptography_wrapper)
  target_link_libraries(cunit_security_core PRIVATE OpenSSL::SSL)
  target_link_libraries(cunit_security_core PRIVATE security_openssl)
endif()
target_include_directories(cunit_security_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS dds_security_authentication_wrapper EXPORT "${PROJECT_NAME}")
  install(TARGETS dds_security_cryptography_wrapper EXPORT "${PROJECT_NAME}")
  install(TARGETS dds_security_access_control_wrapper EXPORT "${PROJECT_NAME}")
endif()
