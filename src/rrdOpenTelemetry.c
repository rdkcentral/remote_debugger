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

    /*
     * TODO: Initialize OpenTelemetry-CPP SDK here
     * This is a placeholder. In real implementation, you would:
     * 1. Create TracerProvider
     * 2. Create OTLP exporter with collectorEndpoint
     * 3. Add BatchSpanProcessor
     * 4. Set global TracerProvider
     */

    g_otel_initialized = 1;
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: OpenTelemetry initialization completed\n",
            __FUNCTION__, __LINE__);
    
    return 0;
}

/**
 * @brief Shutdown OpenTelemetry SDK
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

    /*
     * TODO: Shutdown OpenTelemetry-CPP SDK
     * This would flush pending spans and cleanup resources
     */

    g_otel_initialized = 0;
    return 0;
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
 */
uint64_t rrdOtel_StartSpan(const char *spanName, const char *attributes)
{
    if (!spanName || g_span_count >= MAX_SPANS_PER_THREAD)
    {
        return 0;
    }

    uint64_t spanId = (uint64_t)time(NULL) * 1000000 + g_span_count;
    
    otel_span_t *span = &g_active_spans[g_span_count];
    span->spanId = spanId;
    span->startTime = (uint64_t)time(NULL);
    strncpy(span->spanName, spanName, sizeof(span->spanName) - 1);
    span->spanName[sizeof(span->spanName) - 1] = '\0';

    g_span_count++;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Started span '%s' (ID: %llu) with attributes: %s\n",
            __FUNCTION__, __LINE__, spanName, (unsigned long long)spanId, 
            attributes ? attributes : "none");

    /*
     * TODO: Create actual OpenTelemetry span
     * Extract trace context from g_thread_local_context and create child span
     */

    return spanId;
}

/**
 * @brief End a span
 */
int rrdOtel_EndSpan(uint64_t spanHandle)
{
    if (g_span_count <= 0)
    {
        return -1;
    }

    /* Find and close the span */
    for (int i = 0; i < g_span_count; i++)
    {
        if (g_active_spans[i].spanId == spanHandle)
        {
            uint64_t duration = (uint64_t)time(NULL) - g_active_spans[i].startTime;
            
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
                    "[%s:%d]: Ended span '%s' (ID: %llu) duration: %llu seconds\n",
                    __FUNCTION__, __LINE__, g_active_spans[i].spanName,
                    (unsigned long long)spanHandle, (unsigned long long)duration);

            /* Remove span from tracking */
            g_active_spans[i].spanId = 0;
            g_span_count--;

            /*
             * TODO: Finalize actual OpenTelemetry span
             * Set end time and mark as complete
             */

            return 0;
        }
    }

    return -1;
}

/**
 * @brief Add an event to current span
 */
int rrdOtel_LogEvent(const char *eventName, const char *eventData)
{
    if (!eventName || g_span_count == 0)
    {
        return -1;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG,
            "[%s:%d]: Logging event '%s' - data: %s\n",
            __FUNCTION__, __LINE__, eventName, eventData ? eventData : "");

    /*
     * TODO: Add event to current active span
     * This would be part of the OpenTelemetry C++ wrapper
     */

    return 0;
}
