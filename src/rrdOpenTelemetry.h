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

#ifndef _RRD_OPEN_TELEMETRY_H_
#define _RRD_OPEN_TELEMETRY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* Max size for trace context strings */
#define RRD_OTEL_TRACE_PARENT_MAX 256
#define RRD_OTEL_TRACE_STATE_MAX  256

/**
 * @brief Trace context structure to store trace parent and state
 * This mirrors RBUS OpenTelemetry context for compatibility
 */
typedef struct {
    char traceParent[RRD_OTEL_TRACE_PARENT_MAX];
    char traceState[RRD_OTEL_TRACE_STATE_MAX];
} rrd_otel_context_t;

/**
 * @brief Initialize OpenTelemetry SDK
 * Must be called once at application startup
 * 
 * @param serviceName Name of the service (e.g., "remote_debugger")
 * @param collectorEndpoint OTLP collector endpoint (e.g., "http://localhost:4318")
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_Initialize(const char *serviceName, const char *collectorEndpoint);

/**
 * @brief Shutdown OpenTelemetry SDK
 * Should be called before application exit
 * 
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_Shutdown(void);

/**
 * @brief Get current trace context from thread-local storage
 * This retrieves the context that was previously set
 * 
 * @param ctx Output: pointer to trace context structure
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_GetContext(rrd_otel_context_t *ctx);

/**
 * @brief Set trace context for current thread
 * This stores trace context in thread-local storage to be used for RBUS operations
 * 
 * @param ctx Trace context to set
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_SetContext(const rrd_otel_context_t *ctx);

/**
 * @brief Generate a new trace context
 * Creates a new trace ID and span ID for starting a new trace
 * 
 * @param ctx Output: generated trace context
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_GenerateContext(rrd_otel_context_t *ctx);

/**
 * @brief Create a span for an operation
 * Starts a new span that represents an operation. Should be paired with rrdOtel_EndSpan
 * 
 * @param spanName Name of the span (e.g., "ProcessIssueEvent")
 * @param attributes Optional key-value pairs as JSON string (NULL if none)
 * @return Span handle on success, 0 on failure
 */
uint64_t rrdOtel_StartSpan(const char *spanName, const char *attributes);

/**
 * @brief End a span
 * Ends a previously started span
 * 
 * @param spanHandle Handle returned by rrdOtel_StartSpan
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_EndSpan(uint64_t spanHandle);

/**
 * @brief Add an event to current span
 * Records an event/log within a span
 * 
 * @param eventName Name of the event
 * @param eventData Event description/data
 * @return 0 on success, non-zero on failure
 */
int rrdOtel_LogEvent(const char *eventName, const char *eventData);

#ifdef __cplusplus
}
#endif

#endif /* _RRD_OPEN_TELEMETRY_H_ */
