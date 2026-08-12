cmake_minimum_required(VERSION 4.2)

set(PAPER_SDK_EXAMPLES_STANDALONE OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(PAPER_SDK_EXAMPLES_STANDALONE ON)
  project(PAPERSDKExamples VERSION 0.1.0 LANGUAGES CXX)

  macro(paper_sdk_set_from_environment variable_name)
    if(NOT DEFINED ${variable_name} AND DEFINED ENV{${variable_name}})
      set(${variable_name} "$ENV{${variable_name}}")
    endif()
  endmacro()

  paper_sdk_set_from_environment(COMMON_LIB_F4VR_PATH)
  if(NOT DEFINED COMMON_LIB_F4VR_PATH OR COMMON_LIB_F4VR_PATH STREQUAL "")
    message(FATAL_ERROR
      "COMMON_LIB_F4VR_PATH is required for a standalone SDK example build")
  endif()

  set(F4SE_SUPPORT_XBYAK ON CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_NG OFF CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_F4 OFF CACHE BOOL "" FORCE)
  set(ENABLE_FALLOUT_VR ON CACHE BOOL "" FORCE)
  add_subdirectory("${COMMON_LIB_F4VR_PATH}" "CommonLibF4" EXCLUDE_FROM_ALL)

  add_library(RPSSDK_PAPER_STANDALONE INTERFACE)
  target_include_directories(RPSSDK_PAPER_STANDALONE INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/../include")
  add_library(RPS::PAPER ALIAS RPSSDK_PAPER_STANDALONE)
endif()

set(PAPER_SDK_EXAMPLE_COMMON
  "${CMAKE_CURRENT_LIST_DIR}/common/ExampleRuntime.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/common/ExampleRuntime.h")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/VerifyFo4VrLoader.cmake")
paper_sdk_verify_fo4vr_loader(
  "${CMAKE_CURRENT_LIST_DIR}/common/ExampleRuntime.cpp")

find_package(spdlog CONFIG REQUIRED)

function(add_paper_sdk_example target_name source_path)
  add_library(${target_name} SHARED
    ${PAPER_SDK_EXAMPLE_COMMON}
    "${source_path}")
  target_compile_features(${target_name} PRIVATE cxx_std_23)
  target_compile_definitions(${target_name} PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NOMMNOSOUND)
  target_include_directories(${target_name} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/common")
  target_link_libraries(${target_name} PRIVATE
    RPS::PAPER
    CommonLibF4::CommonLibF4
    spdlog::spdlog)
  target_compile_options(${target_name} PRIVATE
    /sdl
    /utf-8
    /permissive-
    /Zc:__cplusplus
    /Zc:preprocessor
    /W4
    "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC->")
  target_link_options(${target_name} PRIVATE
    "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
    "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>")
  set_target_properties(${target_name} PROPERTIES
    PREFIX ""
    FOLDER "PAPER SDK Examples")
endfunction()

add_paper_sdk_example(PAPERSDKRuntimeMonitor
  "${CMAKE_CURRENT_LIST_DIR}/mods/RuntimeMonitor.cpp")
add_paper_sdk_example(PAPERSDKNativeHandPoseReader
  "${CMAKE_CURRENT_LIST_DIR}/mods/NativeHandPoseReader.cpp")
add_paper_sdk_example(PAPERSDKReloadCatalogDumper
  "${CMAKE_CURRENT_LIST_DIR}/mods/ReloadCatalogDumper.cpp")
add_paper_sdk_example(PAPERSDKReloadFrameObserver
  "${CMAKE_CURRENT_LIST_DIR}/mods/ReloadFrameObserver.cpp")
add_paper_sdk_example(PAPERSDKAnimationTelemetryObserver
  "${CMAKE_CURRENT_LIST_DIR}/mods/AnimationTelemetryObserver.cpp")
add_paper_sdk_example(PAPERSDKReloadStageObserver
  "${CMAKE_CURRENT_LIST_DIR}/mods/ReloadStageObserver.cpp")
add_paper_sdk_example(PAPERSDKNativePoseInspector
  "${CMAKE_CURRENT_LIST_DIR}/mods/NativePoseInspector.cpp")
add_paper_sdk_example(PAPERSDKWeaponMotionCatalogReader
  "${CMAKE_CURRENT_LIST_DIR}/mods/WeaponMotionCatalogReader.cpp")
add_paper_sdk_example(PAPERSDKWeaponMotionDiagnostics
  "${CMAKE_CURRENT_LIST_DIR}/mods/WeaponMotionDiagnostics.cpp")
add_paper_sdk_example(PAPERSDKManipulationProgressObserver
  "${CMAKE_CURRENT_LIST_DIR}/mods/ManipulationProgressObserver.cpp")
add_paper_sdk_example(PAPERSDKDevelopmentCaptureController
  "${CMAKE_CURRENT_LIST_DIR}/mods/DevelopmentCaptureController.cpp")
add_paper_sdk_example(PAPERSDKAnimationAuthorityLease
  "${CMAKE_CURRENT_LIST_DIR}/mods/AnimationAuthorityLease.cpp")

add_custom_target(PAPERSDKExamplePlugins)
add_dependencies(PAPERSDKExamplePlugins
  PAPERSDKRuntimeMonitor
  PAPERSDKNativeHandPoseReader
  PAPERSDKReloadCatalogDumper
  PAPERSDKReloadFrameObserver
  PAPERSDKAnimationTelemetryObserver
  PAPERSDKReloadStageObserver
  PAPERSDKNativePoseInspector
  PAPERSDKWeaponMotionCatalogReader
  PAPERSDKWeaponMotionDiagnostics
  PAPERSDKManipulationProgressObserver
  PAPERSDKDevelopmentCaptureController
  PAPERSDKAnimationAuthorityLease)
