cmake_minimum_required(VERSION 3.10)
project(remote_debugger VERSION 1.0.0 LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenTelemetry-CPP
find_package(opentelemetry-cpp CONFIG QUIET)

if(NOT opentelemetry-cpp_FOUND)
    # Fall back to pkg-config (common in Yocto)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(OPENTELEMETRY opentelemetry-cpp)
    endif()
endif()

if(NOT opentelemetry-cpp_FOUND AND NOT OPENTELEMETRY_FOUND)
    message(FATAL_ERROR "OpenTelemetry-CPP not found. Please install it or set CMAKE_PREFIX_PATH")
endif()

message(STATUS "OpenTelemetry-CPP found - building with distributed tracing support")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# ============================================================================
# OpenTelemetry C++ Wrapper Library
# ============================================================================
if(ENABLE_OPENTELEMETRY AND HAS_OPENTELEMETRY)
    add_library(rrd_otel_cpp_wrapper STATIC
        src/rrdOpenTelemetry_cpp_wrapper.cpp
    )
    
    # Set C++17 requirement
    target_compile_features(rrd_otel_cpp_wrapper PRIVATE cxx_std_17)
    
    # Link OpenTelemetry libraries
add_library(rrd_otel_cpp_wrapper STATIC
    src/rrdOpenTelemetry_cpp_wrapper.cpp
)

# Set C++17 requirement
target_compile_features(rrd_otel_cpp_wrapper PRIVATE cxx_std_17)

# Link OpenTelemetry libraries
if(opentelemetry-cpp_FOUND)
    # Using CMake config
    target_link_libraries(rrd_otel_cpp_wrapper PUBLIC
        opentelemetry-cpp::trace
        opentelemetry-cpp::common
        opentelemetry-cpp::ostream_span_exporter
        opentelemetry-cpp::otlp_http_exporter
        opentelemetry-cpp::resources
    )
else()
    # Using pkg-config
    target_include_directories(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_INCLUDE_DIRS})
    target_link_libraries(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_LIBRARIES})
    target_compile_options(rrd_otel_cpp_wrapper PUBLIC ${OPENTELEMETRY_CFLAGS_OTHER})
endif()

# Include paths
target_include_directories(rrd_otel_cpp_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Optional: Disable RTTI and exceptions for smaller binary (if compatible)
if(NOT MSVC)
    target_compile_options(rrd_otel_cpp_wrapper PRIVATE
        -fno-rtti
        $<$<CONFIG:Release>:-O3>
    )
endif()

# Install library
install(TARGETS rrd_otel_cpp_wrapper
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install header
install(FILES src/rrdOpenTelemetry_cpp_wrapper.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rrd

add_library(remote_debugger STATIC ${RRD_SOURCES})

# Link OpenTelemetry wrapper if enabled
if(ENABLE_OPENTELEMETRY AND HAS_OPENTELEMETRY)
    target_link_libraries(re
target_link_libraries(remote_debugger PUBLIC rrd_otel_cpp_wrapperude directories for remote_debugger
target_include_directories(remote_debugger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Install library
install(TARGETS remote_debugger
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install public headers
install(FILES
    include/rrdOpenTelemetry.h
    # Add other public headers here
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rrd
)

# ============================================================================
# Optional: Example/Test Application
# ============================================================================
option(BUILD_RRD_EXAMPLES "Build remote_debugger examples" OFF)
Summary
# ============================================================================
message(STATUS "")
message(STATUS "Remote Debugger Build Configuration:")
message(STATUS "  C Standard:         ${CMAKE_C_STANDARD}")
message(STATUS "  C++ Standard:       ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix:
