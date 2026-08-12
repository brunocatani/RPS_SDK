cmake_minimum_required(VERSION 4.2)
project(RPSSDKPackageConsumer LANGUAGES CXX)

find_package(RPS_SDK 0.1 CONFIG REQUIRED)

add_executable(RPSSDKPackageConsumer main.cpp)
target_compile_features(RPSSDKPackageConsumer PRIVATE cxx_std_23)
target_link_libraries(RPSSDKPackageConsumer PRIVATE RPS::PAPER RPS::ROCK)
