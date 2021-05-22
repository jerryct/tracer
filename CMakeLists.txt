cmake_minimum_required(VERSION 3.10)

project(tracer CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS Off)
set(DEFAULT_COPTS "-fno-omit-frame-pointer" "-g" "-Wall" "-Wextra" "-I" "${PROJECT_SOURCE_DIR}")
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

add_executable(tracer
  main.cpp
)
target_link_libraries(tracer PRIVATE Threads::Threads)

add_executable(unit_tests
  tracer_unit_tests.cpp
)
target_link_libraries(unit_tests PRIVATE Threads::Threads gtest_main)
add_test(unit_tests unit_tests)

add_executable(benchmarks
  benchmark.cpp
)
target_link_libraries(benchmarks PRIVATE Threads::Threads benchmark::benchmark)

