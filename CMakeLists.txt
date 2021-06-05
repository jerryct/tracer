cmake_minimum_required(VERSION 3.10)

project(tracer CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS Off)
set(DEFAULT_COPTS "-fno-omit-frame-pointer" "-g" "-Wall" "-Wextra") # "-fno-builtin-memcpy" "-U_FORTIFY_SOURCE"

set(DEFAULT_LINKOPTS "")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
  list(APPEND DEFAULT_COPTS
    $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    #$<$<CONFIG:Debug>:-fsanitize=thread>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fprofile-instr-generate>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fcoverage-mapping>
    $<$<CXX_COMPILER_ID:GNU>:-ftree-vectorize>
    $<$<CXX_COMPILER_ID:GNU>:-fopt-info-vec-optimized>
    -march=nehalem
  )
  list(APPEND DEFAULT_LINKOPTS
    $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    #$<$<CONFIG:Debug>:-fsanitize=thread>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fprofile-instr-generate>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fcoverage-mapping>
  )
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.*|aarch64")
else()
  message(WARNING "Value of CMAKE_SYSTEM_PROCESSOR (${CMAKE_SYSTEM_PROCESSOR}) is unknown")
endif()

add_compile_options(
  ${DEFAULT_COPTS}
)
link_libraries(
  ${DEFAULT_LINKOPTS}
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(../benchmark _build/benchmark)
add_subdirectory(../googletest _build/googletest)
set(JERRYCT_STRING_VIEW_ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(../string_view _build/string_view)

add_library(tracing
  jerryct/tracing/tracing.cpp
)
target_include_directories(tracing PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:.>
)
target_link_libraries(tracing PUBLIC jerryct::string_view Threads::Threads)

add_executable(example
  example.cpp
)
target_link_libraries(example PRIVATE tracing)

add_executable(unit_tests
  jerryct/tracing/tracing_tests.cpp
)
target_link_libraries(unit_tests PRIVATE tracing gtest_main)
add_test(unit_tests unit_tests)

add_executable(benchmarks
  benchmark.cpp
)
target_link_libraries(benchmarks PRIVATE tracing benchmark::benchmark)

