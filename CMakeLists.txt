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
        # Try different possible package names
        pkg_check_modules(OPENTELEMETRY QUIET opentelemetry-cpp)
        if(NOT OPENTELEMETRY_FOUND)
            pkg_check_modules(OPENTELEMETRY QUIET opentelemetry)
        endif()
        if(NOT OPENTELEMETRY_FOUND)
            # Try individual components
            pkg_check_modules(OTEL_TRACE QUIET opentelemetry_trace)
            pkg_check_modules(OTEL_COMMON QUIET opentelemetry_common)
            pkg_check_modules(OTEL_RESOURCES QUIET opentelemetry_resources)
            pkg_check_modules(OTEL_OTLP QUIET opentelemetry_otlp_http_exporter)
            if(OTEL_TRACE_FOUND AND OTEL_COMMON_FOUND)
                message(STATUS "OpenTelemetry-CPP found via pkg-config (individual components)")
                set(FOUND_BY_PKGCONFIG TRUE)
                # Combine all the variables
                set(OPENTELEMETRY_INCLUDE_DIRS ${OTEL_TRACE_INCLUDE_DIRS} ${OTEL_COMMON_INCLUDE_DIRS} ${OTEL_RESOURCES_INCLUDE_DIRS} ${OTEL_OTLP_INCLUDE_DIRS})
                set(OPENTELEMETRY_LIBRARIES ${OTEL_TRACE_LIBRARIES} ${OTEL_COMMON_LIBRARIES} ${OTEL_RESOURCES_LIBRARIES} ${OTEL_OTLP_LIBRARIES})
                set(OPENTELEMETRY_LIBRARY_DIRS ${OTEL_TRACE_LIBRARY_DIRS} ${OTEL_COMMON_LIBRARY_DIRS} ${OTEL_RESOURCES_LIBRARY_DIRS} ${OTEL_OTLP_LIBRARY_DIRS})
                list(REMOVE_DUPLICATES OPENTELEMETRY_INCLUDE_DIRS)
                list(REMOVE_DUPLICATES OPENTELEMETRY_LIBRARY_DIRS)
            endif()
        else()
            message(STATUS "OpenTelemetry-CPP found via pkg-config")
            set(FOUND_BY_PKGCONFIG TRUE)
        endif()
    endif()
endif()

if(NOT FOUND_BY_CMAKE AND NOT FOUND_BY_PKGCONFIG)
    message(WARNING "OpenTelemetry-CPP not found via CMake or pkg-config")
    message(WARNING "Attempting to use direct library linking...")
    # Last resort: try to find libraries directly
    find_library(OTEL_TRACE_LIB opentelemetry_trace)
    find_library(OTEL_COMMON_LIB opentelemetry_common)
    if(OTEL_TRACE_LIB AND OTEL_COMMON_LIB)
        message(STATUS "OpenTelemetry libraries found directly")
        set(FOUND_BY_PKGCONFIG TRUE)
        set(OPENTELEMETRY_LIBRARIES 
            ${OTEL_TRACE_LIB}
            ${OTEL_COMMON_LIB}
        )
        # Try to find additional libraries
        find_library(OTEL_RESOURCES_LIB opentelemetry_resources)
        find_library(OTEL_OTLP_LIB opentelemetry_otlp_http_exporter)
        if(OTEL_RESOURCES_LIB)
            list(APPEND OPENTELEMETRY_LIBRARIES ${OTEL_RESOURCES_LIB})
        endif()
        if(OTEL_OTLP_LIB)
            list(APPEND OPENTELEMETRY_LIBRARIES ${OTEL_OTLP_LIB})
        endif()
    else()
        message(FATAL_ERROR "OpenTelemetry-CPP not found. Please install opentelemetry-cpp package.")
    endif()
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
