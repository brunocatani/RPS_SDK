if(NOT DEFINED SDK_BINARY_DIR OR
   NOT DEFINED CONSUMER_SOURCE_DIR)
  message(FATAL_ERROR "Package consumer test inputs are incomplete")
endif()

set(install_prefix "${SDK_BINARY_DIR}/package-test/install")
set(consumer_binary_dir "${SDK_BINARY_DIR}/package-test/consumer")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${SDK_BINARY_DIR}"
    --prefix "${install_prefix}" --config Release
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "RPS SDK package install failed: ${install_result}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}"
    -S "${CONSUMER_SOURCE_DIR}"
    -B "${consumer_binary_dir}"
    -G "Visual Studio 17 2022"
    -A x64
    "-DCMAKE_PREFIX_PATH=${install_prefix}"
  RESULT_VARIABLE configure_result)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR
    "External RPS SDK package consumer configure failed: ${configure_result}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${consumer_binary_dir}"
    --config Release -- /m:1 /p:CL_MPCount=2
  RESULT_VARIABLE build_result)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR
    "External RPS SDK package consumer build failed: ${build_result}")
endif()
