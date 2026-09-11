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
option(RPS_SDK_BUILD_PAPER_EXAMPLES "Build the PAPER SDK example DLLs" ON)
option(RPS_SDK_ENABLE_TESTS "Register SDK contract tests" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(RPSSDK_ROCK INTERFACE)
add_library(RPS::ROCK ALIAS RPSSDK_ROCK)
set_target_properties(RPSSDK_ROCK PROPERTIES EXPORT_NAME ROCK)
target_include_directories(RPSSDK_ROCK INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDK/ROCK/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/ROCK>")

add_library(RPSSDK_PAPER INTERFACE)
add_library(RPS::PAPER ALIAS RPSSDK_PAPER)
set_target_properties(RPSSDK_PAPER PROPERTIES EXPORT_NAME PAPER)
target_include_directories(RPSSDK_PAPER INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDK/PAPER/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/PAPER>")

add_library(RPSSDK_UI INTERFACE)
add_library(RPS::UI ALIAS RPSSDK_UI)
set_target_properties(RPSSDK_UI PROPERTIES EXPORT_NAME UI)
target_compile_features(RPSSDK_UI INTERFACE cxx_std_17)
target_include_directories(RPSSDK_UI INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDK/UI/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/UI>")

add_library(RPSSDK INTERFACE)
add_library(RPS::SDK ALIAS RPSSDK)
set_target_properties(RPSSDK PROPERTIES EXPORT_NAME SDK)
target_link_libraries(RPSSDK INTERFACE RPSSDK_ROCK RPSSDK_PAPER RPSSDK_UI)

if(RPS_SDK_BUILD_ROCK_EXAMPLES OR RPS_SDK_BUILD_PAPER_EXAMPLES)
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
endif()

if(RPS_SDK_BUILD_ROCK_EXAMPLES)
  add_subdirectory("SDK/ROCK/examples" "rock-examples")
endif()

if(RPS_SDK_BUILD_PAPER_EXAMPLES)
  add_subdirectory("SDK/PAPER/examples" "paper-examples")
endif()

if(RPS_SDK_BUILD_ROCK_EXAMPLES OR RPS_SDK_BUILD_PAPER_EXAMPLES)
  add_custom_target(RPSSDKExamplePlugins)
  if(TARGET ROCKSDKExamplePlugins)
    add_dependencies(RPSSDKExamplePlugins ROCKSDKExamplePlugins)
  endif()
  if(TARGET PAPERSDKExamplePlugins)
    add_dependencies(RPSSDKExamplePlugins PAPERSDKExamplePlugins)
  endif()
endif()

install(TARGETS RPSSDK RPSSDK_ROCK RPSSDK_PAPER RPSSDK_UI
  EXPORT RPSSDKTargets)
install(DIRECTORY "SDK/ROCK/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/ROCK")
install(DIRECTORY "SDK/PAPER/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/PAPER")
install(DIRECTORY "SDK/UI/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RPS_SDK/UI")
install(EXPORT RPSSDKTargets
  FILE RPS_SDKTargets.cmake
  NAMESPACE RPS::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RPS_SDK")
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RPS_SDKConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/RPS_SDKConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RPS_SDK")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/RPS_SDKConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/RPS_SDKConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/RPS_SDKConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RPS_SDK")

if(RPS_SDK_ENABLE_TESTS)
  enable_testing()
  # Compile the inert UI integration fragment without adding an ImGui dependency
  # or producing a plugin that could accidentally be deployed.
  add_library(RPSSDKUIExample OBJECT SDK/UI/examples/PanelConsumer.cpp)
  target_link_libraries(RPSSDKUIExample PRIVATE RPS::UI)
  if(TARGET RPSSDKExamplePlugins)
    add_dependencies(RPSSDKExamplePlugins RPSSDKUIExample)
  endif()
  find_program(RPS_SDK_PWSH_EXECUTABLE NAMES pwsh REQUIRED)
  add_test(
    NAME RpsSdkRockContractTests
    COMMAND
      "${RPS_SDK_PWSH_EXECUTABLE}"
      -NoProfile
      -ExecutionPolicy Bypass
      -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/RockSdkContractTests.ps1"
      -Root "${CMAKE_CURRENT_SOURCE_DIR}")

  add_test(
    NAME RpsSdkPaperContractTests
    COMMAND
      "${RPS_SDK_PWSH_EXECUTABLE}"
      -NoProfile
      -ExecutionPolicy Bypass
      -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/PaperSdkContractTests.ps1"
      -Root "${CMAKE_CURRENT_SOURCE_DIR}")

  add_test(
    NAME RpsSdkPackageConsumerTests
    COMMAND
      "${CMAKE_COMMAND}"
      -DSDK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
      -DSDK_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
      -DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/package-consumer
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/PackageConsumerTests.cmake")

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

  set(_workspace_paper "${CMAKE_CURRENT_SOURCE_DIR}/../PAPER")
  if(EXISTS "${_workspace_paper}/src/api/PAPERApi.h")
    add_test(
      NAME RpsSdkPaperHeaderSyncTests
      COMMAND
        "${RPS_SDK_PWSH_EXECUTABLE}"
        -NoProfile
        -ExecutionPolicy Bypass
        -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/PaperHeaderSyncTests.ps1"
        -Root "${CMAKE_CURRENT_SOURCE_DIR}"
        -PaperRoot "${_workspace_paper}")
  endif()

  set(_workspace_ui "${CMAKE_CURRENT_SOURCE_DIR}/../../secondary_projects/RPS_UI_Framework")
  if(EXISTS "${_workspace_ui}/SDK/include/RPSUIFrameworkApi.h")
    add_test(
      NAME RpsSdkUiHeaderSyncTests
      COMMAND "${RPS_SDK_PWSH_EXECUTABLE}" -NoProfile
        -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/UiHeaderSyncTests.ps1"
        -Root "${CMAKE_CURRENT_SOURCE_DIR}" -UiRoot "${_workspace_ui}")
  endif()
endif()
