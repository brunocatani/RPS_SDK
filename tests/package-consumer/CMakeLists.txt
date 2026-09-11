cmake_minimum_required(VERSION 4.2)
project(RPSSDKPackageConsumer LANGUAGES CXX)

find_package(RPS_SDK 0.1 CONFIG REQUIRED)

add_executable(RPSSDKPackageConsumer main.cpp)
target_compile_features(RPSSDKPackageConsumer PRIVATE cxx_std_23)
target_link_libraries(RPSSDKPackageConsumer PRIVATE RPS::SDK)
# Also require the individual target to survive installation/export.
if(NOT TARGET RPS::UI)
  message(FATAL_ERROR "The installed SDK is missing RPS::UI")
endif()
