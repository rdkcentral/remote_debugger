cmake_minimum_required(VERSION 3.10)
project(rrd_otel_cpp_wrapper VERSION 1.0.0 LANGUAGES CXX)

# Include GNUInstallDirs to define CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_INCLUDEDIR, etc.
include(GNUInstallDirs)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenTelemetry-CPP
find_package(opentelemetry-cpp CONFIG QUIET)

# Track which method found the package
set(FOUND_BY_CMAKE FALSE)
set(FOUND_BY_PKGCONFIG FALSE)

if(opentelemetry-cpp_FOUND)
    message(STATUS "OpenTelemetry-CPP found via CMake config")
    set(FOUND_BY_CMAKE TRUE)
else()
    # Fall back to pkg-config (common in Yocto)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(OPENTELEMETRY opentelemetry-cpp)
        if(OPENTELEMETRY_FOUND)
            message(STATUS "OpenTelemetry-CPP found via pkg-config")
            set(FOUND_BY_PKGCONFIG TRUE)
        endif()
    endif()
endif()

if(NOT FOUND_BY_CMAKE AND NOT FOUND_BY_PKGCONFIG)
    message(FATAL_ERROR "OpenTelemetry-CPP not found. Please install it or set CMAKE_PREFIX_PATH")
endif()

message(STATUS "OpenTelemetry-CPP found - building C++ wrapper for remote_debugger")

# ============================================================================
# OpenTelemetry C++ Wrapper Library
# ============================================================================
add_library(rrd_otel_cpp_wrapper STATIC
    src/rrdOpenTelemetry_cpp_wrapper.cpp
)

target_compile_features(rrd_otel_cpp_wrapper PRIVATE cxx_std_17)

# Link OpenTelemetry libraries based on how they were found
if(FOUND_BY_CMAKE)
    # Use CMake targets
    target_link_libraries(rrd_otel_cpp_wrapper PUBLIC
        opentelemetry-cpp::trace
        opentelemetry-cpp::common
        opentelemetry-cpp::ostream_span_exporter
        opentelemetry-cpp::otlp_http_exporter
        opentelemetry-cpp::resources
    )
else()
    # Use pkg-config variables
    target_include_directories(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_INCLUDE_DIRS})
    target_link_libraries(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_LIBRARIES})
    target_link_directories(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_LIBRARY_DIRS})
    if(OPENTELEMETRY_CFLAGS_OTHER)
        target_compile_options(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_CFLAGS_OTHER})
    endif()
endif()

target_include_directories(rrd_otel_cpp_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

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
