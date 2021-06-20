cmake_minimum_required(VERSION 3.10)

project(tracer CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS Off)
set(DEFAULT_COPTS "-fno-omit-frame-pointer" "-g") # "-fno-builtin-memcpy" "-U_FORTIFY_SOURCE"

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
  jerryct/tracing/prometheus_exporter.cpp
  jerryct/tracing/r_exporter.cpp
  jerryct/tracing/tracing.cpp
)
target_include_directories(tracing PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:.>
)
target_link_libraries(tracing PUBLIC jerryct::string_view Threads::Threads)
target_compile_options(tracing PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2")

add_executable(example
  example.cpp
)
target_link_libraries(example PRIVATE tracing)
target_compile_options(example PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2")

add_executable(unit_tests
  jerryct/tracing/tracing_tests.cpp
)
target_link_libraries(unit_tests PRIVATE tracing gtest_main)
target_compile_options(unit_tests PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2")
add_test(unit_tests unit_tests)

add_executable(benchmarks
  jerryct/tracing/tracing_benchmark.cpp
)
target_link_libraries(benchmarks PRIVATE tracing benchmark::benchmark)
target_compile_options(benchmarks PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2")
