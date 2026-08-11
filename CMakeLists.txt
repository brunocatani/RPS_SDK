cmake_minimum_required(VERSION 4.2)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE FILEPATH "vcpkg toolchain")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND
       EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE
      "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE FILEPATH "vcpkg toolchain")
endif()

project(RPS_SDK VERSION 0.1.0 LANGUAGES CXX)

option(RPS_SDK_BUILD_ROCK_EXAMPLES "Build the ROCK SDK example DLLs" ON)
option(RPS_SDK_ENABLE_TESTS "Register SDK contract tests" ON)

if(RPS_SDK_BUILD_ROCK_EXAMPLES)
  if(NOT DEFINED COMMON_LIB_F4VR_PATH OR COMMON_LIB_F4VR_PATH STREQUAL "")
    set(_workspace_commonlib
        "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries_and_tools/CommonLibF4VR")
    if(EXISTS "${_workspace_commonlib}/CMakeLists.txt")
      set(COMMON_LIB_F4VR_PATH "${_workspace_commonlib}")
    else()
      message(FATAL_ERROR
        "COMMON_LIB_F4VR_PATH must name a CommonLibF4VR checkout")
    endif()
  endif()
  if(NOT EXISTS "${COMMON_LIB_F4VR_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
      "COMMON_LIB_F4VR_PATH does not contain CMakeLists.txt: ${COMMON_LIB_F4VR_PATH}")
  endif()

  set(F4SE_SUPPORT_XBYAK ON CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_NG OFF CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_F4 OFF CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_VR ON CACHE BOOL "" FORCE)
  add_subdirectory(
    "${COMMON_LIB_F4VR_PATH}"
    "dependencies/CommonLibF4"
    EXCLUDE_FROM_ALL)
  add_subdirectory("SDK/ROCK/examples" "rock-examples")
endif()

if(RPS_SDK_ENABLE_TESTS)
  enable_testing()
  find_program(RPS_SDK_PWSH_EXECUTABLE NAMES pwsh REQUIRED)
  add_test(
    NAME RpsSdkRockContractTests
    COMMAND
      "${RPS_SDK_PWSH_EXECUTABLE}"
      -NoProfile
      -ExecutionPolicy Bypass
      -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/RockSdkContractTests.ps1"
      -Root "${CMAKE_CURRENT_SOURCE_DIR}")

  set(_workspace_rock "${CMAKE_CURRENT_SOURCE_DIR}/../ROCK")
  if(EXISTS "${_workspace_rock}/src/api/ROCKProviderApi.h")
    add_test(
      NAME RpsSdkRockHeaderSyncTests
      COMMAND
        "${RPS_SDK_PWSH_EXECUTABLE}"
        -NoProfile
        -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/RockHeaderSyncTests.ps1"
        -Root "${CMAKE_CURRENT_SOURCE_DIR}"
        -RockRoot "${_workspace_rock}")
  endif()
endif()
