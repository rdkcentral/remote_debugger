#ifndef RRD_OPENTELEMETRY_CPP_WRAPPER_H
#define RRD_OPENTELEMETRY_CPP_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize OpenTelemetry SDK with OTLP HTTP exporter
 * 
 * @param serviceName The service name for resource attributes
 * @param collectorEndpoint OTLP collector endpoint (e.g., http://localhost:4318)
 * @return 0 on success, -1 on error
 */
int rrdOtel_Initialize_Cpp(const char *serviceName, const char *collectorEndpoint);

/**
 * Start a span (may be root or child based on traceParent)
 * 
 * @param spanName The name of the span
 * @param attributes Optional attributes (key=value format)
 * @param traceParent W3C traceparent header (00-traceId-spanId-flags) or empty for root
 * @return Span handle (uint64_t) or 0 on error
 */
uint64_t rrdOtel_StartSpan_Cpp(const char *spanName, const char *attributes,
                               const char *traceParent);

/**
 * End a span
 * 
 * @param spanHandle The span handle from rrdOtel_StartSpan_Cpp
 * @return 0 on success, -1 on error
 */
int rrdOtel_EndSpan_Cpp(uint64_t spanHandle);

/**
 * Log an event in the active span
 * 
 * @param eventName The event name
 * @param eventData Optional event data
 * @return 0 on success, -1 if no active span
 */
int rrdOtel_LogEvent_Cpp(const char *eventName, const char *eventData);

/**
 * Shutdown OpenTelemetry SDK (flushes pending spans)
 * 
 * @return 0 on success, -1 on error
 */
int rrdOtel_Shutdown_Cpp();

#ifdef __cplusplus
}
#endif

#endif // RRD_OPENTELEMETRY_CPP_WRAPPER_H
