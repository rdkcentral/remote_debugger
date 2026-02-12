cmake_minimum_required(VERSION 3.10)
project(rrd_otel_cpp_wrapper VERSION 1.0.0 LANGUAGES CXX)


include(GNUInstallDirs)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# OpenTelemetry C++ Wrapper Library
# ============================================================================
add_library(rrd_otel_cpp_wrapper STATIC
    src/rrdOpenTelemetry_cpp_wrapper.cpp
)

target_compile_features(rrd_otel_cpp_wrapper PRIVATE cxx_std_17)

# Link OpenTelemetry libraries
    target_link_libraries(rrd_otel_cpp_wrapper PUBLIC
        opentelemetry-cpp::trace
        opentelemetry-cpp::common
        opentelemetry-cpp::ostream_span_exporter
        opentelemetry-cpp::otlp_http_exporter
        opentelemetry-cpp::resources
    )

target_include_directories(rrd_otel_cpp_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Optimize for size
if(NOT MSVC)
    target_compile_options(rrd_otel_cpp_wrapper PRIVATE
        -fno-rtti
        $<$<CONFIG:Release>:-O3>
    )
endif()

# Install library for Makefile to link against
install(TARGETS rrd_otel_cpp_wrapper
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install header for C code to include
install(FILES src/rrdOpenTelemetry_cpp_wrapper.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "C++ Wrapper Build Configuration:")
message(STATUS "  C++ Standard:       ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix:     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Output Library:     librrd_otel_cpp_wrapper.a")
message(STATUS "")
message(STATUS "NOTE: Main binary built by existing Makefile")
message(STATUS "")
