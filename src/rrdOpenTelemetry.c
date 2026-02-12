/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "rrdOpenTelemetry.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#if !defined(GTEST_ENABLE)
#include "rdk_debug.h"
#define LOG_REMDEBUG "LOG.RDK.REMOTEDEBUGGER"
#else
#define RDK_LOG(a, b, c, ...) printf(c, ##__VA_ARGS__)
#endif

/* ✅ CRITICAL FIXES #1-6: Forward declarations for C++ wrapper functions */
/* These provide the actual OpenTelemetry-CPP SDK integration with all 6 fixes:
 * #1: Parent SpanContext conversion from W3C hex format
 * #2: StartSpan with StartSpanOptions and parent context
 * #3: Thread-local trace::Scope management for context propagation
 * #4: Resource attributes with service metadata
 * #5: Thread-safe span storage with std::map + std::mutex
 * #6: SimpleSpanProcessor (sync) instead of BatchSpanProcessor (async)
 */
extern int rrdOtel_Initialize_Cpp(const char *serviceName, const char *collectorEndpoint);
extern uint64_t rrdOtel_StartSpan_Cpp(const char *spanName, const char *attributes,
                                       const char *traceParent);
extern int rrdOtel_EndSpan_Cpp(uint64_t spanHandle);
extern int rrdOtel_LogEvent_Cpp(const char *eventName, const char *eventData);
extern int rrdOtel_Shutdown_Cpp(void);

/* Thread-local storage for trace context */
static __thread rrd_otel_context_t g_thread_local_context = {0};
static pthread_once_t g_otel_init_once = PTHREAD_ONCE_INIT;
static int g_otel_initialized = 0;

/* Simple span tracker - store span handles for current thread */
typedef struct {
    uint64_t spanId;
    char spanName[256];
    uint64_t startTime;
} otel_span_t;

#define MAX_SPANS_PER_THREAD 32
static __thread otel_span_t g_active_spans[MAX_SPANS_PER_THREAD] = {0};
static __thread int g_span_count = 0;

/**
 * @brief Generate a random hex string for IDs
 * Used to generate trace IDs and span IDs
 */
static void _generate_hex_id(char *buffer, int length)
{
    static const char hex_chars[] = "0123456789abcdef";
    srand((unsigned int)time(NULL) + pthread_self());
    
    for (int i = 0; i < length; i++)
    {
        buffer[i] = hex_chars[rand() % 16];
    }
    buffer[length] = '\0';
}

/**
 * @brief Initialize OpenTelemetry SDK
 * 
 * Initializes the OpenTelemetry-CPP SDK with OTLP HTTP exporter.
 * Creates a global TracerProvider with BatchSpanProcessor for efficient batch export.
 */
int rrdOtel_Initialize(const char *serviceName, const char *collectorEndpoint)
{
    if (!serviceName || !collectorEndpoint)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, 
                "[%s:%d]: Invalid parameters for OTel initialization\n", 
                __FUNCTION__, __LINE__);
        return -1;
    }

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG,
            "[%s:%d]: Initializing OpenTelemetry for service '%s' with collector '%s'\n",
            __FUNCTION__, __LINE__, serviceName, collectorEndpoint);

    /* ✅ CRITICAL FIXES #1-6: Call C++ wrapper with all fixes:
     * #1: W3C hex format parent context conversion (inside StartSpan_Cpp)
     * #2: StartSpanOptions with parent context linking
     * #3: Thread-local trace::Scope management
     * #4: Resource attributes with service.name, version, namespace, environment
     * #5: std::unordered_map + std::mutex for thread-safe span tracking
     * #6: SimpleSpanProcessor (sync immediate export) instead of Batch
     */
    int result = rrdOtel_Initialize_Cpp(serviceName, collectorEndpoint);
    if (result != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Failed to initialize OpenTelemetry C++ wrapper\n",
                __FUNCTION__, __LINE__);
        return -1;
    }

    g_otel_initialized = 1;
    
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG,
            "[%s:%d]: OpenTelemetry initialization completed - ready to export to %s\n",
            __FUNCTION__, __LINE__, collectorEndpoint);
    
    return 0;
}

/**
 * @brief Shutdown OpenTelemetry SDK
 * 
 * Gracefully shutdowns the OpenTelemetry SDK by:
 * - Flushing all pending spans to the collector
 * - Closing HTTP connections
 * - Releasing resources
 */
int rrdOtel_Shutdown(void)
{
    if (!g_otel_initialized)
    {
        return 0;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Shutting down OpenTelemetry\n",
            __FUNCTION__, __LINE__);

    /* ✅ CRITICAL FIXES #1-6: Call C++ wrapper shutdown with proper cleanup
     * The C++ wrapper will:
     * 1. Flush all pending spans to the OTLP collector
     * 2. Close HTTP connections gracefully
     * 3. Deallocate TracerProvider and processors
     * 4. Release all resources allocated in Initialize_Cpp
     */
    int result = rrdOtel_Shutdown_Cpp();
    
    g_otel_initialized = 0;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: OpenTelemetry shutdown completed\n",
            __FUNCTION__, __LINE__);
    
    return result;
}

/**
 * @brief Get current trace context from thread-local storage
 */
int rrdOtel_GetContext(rrd_otel_context_t *ctx)
{
    if (!ctx)
    {
        return -1;
    }

    memcpy(ctx, &g_thread_local_context, sizeof(rrd_otel_context_t));
    return 0;
}

/**
 * @brief Set trace context for current thread
 */
int rrdOtel_SetContext(const rrd_otel_context_t *ctx)
{
    if (!ctx)
    {
        return -1;
    }

    memcpy(&g_thread_local_context, ctx, sizeof(rrd_otel_context_t));

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Set trace context - parent: %s, state: %s\n",
            __FUNCTION__, __LINE__, 
            g_thread_local_context.traceParent,
            g_thread_local_context.traceState);

    return 0;
}

/**
 * @brief Generate a new trace context
 */
int rrdOtel_GenerateContext(rrd_otel_context_t *ctx)
{
    if (!ctx)
    {
        return -1;
    }

    char traceId[33];     /* 32 hex chars + null */
    char spanId[17];      /* 16 hex chars + null */
    char traceFlags[3];   /* 2 hex chars + null */

    _generate_hex_id(traceId, 32);
    _generate_hex_id(spanId, 16);
    strcpy(traceFlags, "01");

    /*
     * W3C Trace Context format:
     * traceparent: 00-<trace-id>-<span-id>-<trace-flags>
     */
    snprintf(ctx->traceParent, RRD_OTEL_TRACE_PARENT_MAX,
             "00-%s-%s-%s", traceId, spanId, traceFlags);

    /* Empty trace state for now */
    ctx->traceState[0] = '\0';

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Generated new trace context - parent: %s\n",
            __FUNCTION__, __LINE__, ctx->traceParent);

    return 0;
}

/**
 * @brief Create a span for an operation
 * 
 * Creates a new span representing a unit of work. Links to parent trace if available.
 * The span is tracked locally and will be exported to the collector via OTLP.
 */
uint64_t rrdOtel_StartSpan(const char *spanName, const char *attributes)
{
    if (!spanName)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Invalid span name\n", __FUNCTION__, __LINE__);
        return 0;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Starting span '%s'\n", __FUNCTION__, __LINE__, spanName);

    /* ✅ CRITICAL FIXES #1-3: Call C++ wrapper with parent context and all fixes:
     * #1: Parent context conversion from W3C hex format (done in wrapper)
     *      Format: 00-<32hexchars>-<16hexchars>-<2hexchars>
     * #2: StartSpanOptions with parent context linking (ensures parent-child relationship)
     * #3: Thread-local trace::Scope activation (makes span current for nested operations)
     * The wrapper also handles #4 (Resource attrs), #5 (thread-safe map+mutex), #6 (SimpleProcessor)
     */
    uint64_t spanHandle = rrdOtel_StartSpan_Cpp(spanName, attributes,
                                                 g_thread_local_context.traceParent);
    
    if (spanHandle == 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Failed to create span '%s'\n", __FUNCTION__, __LINE__, spanName);
        return 0;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Created span '%s' (handle: %llu)\n",
            __FUNCTION__, __LINE__, spanName, (unsigned long long)spanHandle);

    return spanHandle;
}

/**
 * @brief End a span and mark it for export
 * 
 * Finalizes the span by recording end time. The span is exported to the OTLP collector.
 */
int rrdOtel_EndSpan(uint64_t spanHandle)
{
    if (spanHandle == 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Invalid span handle\n", __FUNCTION__, __LINE__);
        return -1;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Ending span (handle: %llu)\n", __FUNCTION__, __LINE__,
            (unsigned long long)spanHandle);

    /* ✅ CRITICAL FIXES #1-3: Call C++ wrapper to end span with all fixes:
     * #1-2: Parent context properly linked during creation (already done in StartSpan_Cpp)
     * #3: Deactivates trace::Scope (clears thread-local current span)
     * Also handles #5 (thread-safe removal) and #6 (SimpleProcessor immediate export)
     */
    int result = rrdOtel_EndSpan_Cpp(spanHandle);

    if (result == 0)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
                "[%s:%d]: Ended span successfully, exported to collector\n",
                __FUNCTION__, __LINE__);
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Failed to end span (handle: %llu)\n", __FUNCTION__, __LINE__,
                (unsigned long long)spanHandle);
    }

    return result;
}

/**
 * @brief Add an event to current span
 * 
 * Logs a named event within the active span context.
 * Events are timestamped and included in the span export to Jaeger.
 */
int rrdOtel_LogEvent(const char *eventName, const char *eventData)
{
    if (!eventName)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Invalid event name\n", __FUNCTION__, __LINE__);
        return -1;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Logging event '%s' - data: %s\n",
            __FUNCTION__, __LINE__, eventName, eventData ? eventData : "");

    /* ✅ CRITICAL FIXES #1-3: Call C++ wrapper to log event:
     * The C++ wrapper has the currently active span via thread-local trace::Scope
     * (#3 was activated in StartSpan_Cpp)
     * 
     * This adds timestamped event to the span that will be exported with it.
     * Events help correlate RBUS property changes and other activities within trace.
     */
    int result = rrdOtel_LogEvent_Cpp(eventName, eventData);

    if (result == 0)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
                "[%s:%d]: Event logged successfully\n", __FUNCTION__, __LINE__);
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG,
                "[%s:%d]: Failed to log event (no active span?)\n", __FUNCTION__, __LINE__);
    }

    return result;
}
