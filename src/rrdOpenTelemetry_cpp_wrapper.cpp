#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/nostd/shared_ptr.h>

#include <mutex>
#include <unordered_map>
#include <thread>
#include <string>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace trace = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace resource = opentelemetry::sdk::resource;
namespace nostd = opentelemetry::nostd;
namespace otlp = opentelemetry::exporter::otlp;

class RrdOtelWrapper {
private:
    std::string service_name_;
    nostd::shared_ptr<trace::Tracer> tracer_;
    
    // ✅ CRITICAL FIX #5: Thread-safe current span tracking
    std::unordered_map<std::thread::id, nostd::shared_ptr<trace::Span>> active_spans_;
    std::mutex spans_mutex_;
    static thread_local std::unique_ptr<trace::Scope> t_active_scope_;
    
    // ✅ CRITICAL FIX #1: Convert W3C hex format to SpanContext
    // W3C Format: 00-<trace_id_hex>-<span_id_hex>-<flags_hex>
    // Example:     00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01
    trace::SpanContext createSpanContextFromIds(const char* trace_id_str,
                                                 const char* span_id_str,
                                                 const char* trace_flags_str) {
        // Trace ID: 32 hex chars = 16 bytes
        std::array<uint8_t, 16> trace_id_bytes;
        for (int i = 0; i < 16; i++) {
            char byte_str[3] = {trace_id_str[i*2], trace_id_str[i*2+1], '\0'};
            trace_id_bytes[i] = static_cast<uint8_t>(std::strtol(byte_str, nullptr, 16));
        }
        
        // Span ID: 16 hex chars = 8 bytes
        std::array<uint8_t, 8> span_id_bytes;
        for (int i = 0; i < 8; i++) {
            char byte_str[3] = {span_id_str[i*2], span_id_str[i*2+1], '\0'};
            span_id_bytes[i] = static_cast<uint8_t>(std::strtol(byte_str, nullptr, 16));
        }
        
        // Flags: 2 hex chars = 1 byte
        uint8_t trace_flags = static_cast<uint8_t>(std::strtol(trace_flags_str, nullptr, 16));
        
        // Create OTEL SDK objects
        auto otlp_trace_id = trace::TraceId(nostd::span<const uint8_t, 16>(trace_id_bytes.data(), 16));
        auto otlp_span_id = trace::SpanId(nostd::span<const uint8_t, 8>(span_id_bytes.data(), 8));
        
        // Return SpanContext (this is what goes into StartSpanOptions)
        return trace::SpanContext(otlp_trace_id, otlp_span_id, trace::TraceFlags(trace_flags), true);
    }

public:
    RrdOtelWrapper(const char *serviceName, const char *collectorEndpoint)
        : service_name_(serviceName) {
        initialize(serviceName, collectorEndpoint);
    }

    void initialize(const char *serviceName, const char *collectorEndpoint) {
        try {
            // ✅ CRITICAL FIX #4: Create Resource with service attributes
            auto resource_attributes = resource::ResourceAttributes{
                {"service.name", serviceName},
                {"service.version", "1.0.0"},
                {"service.namespace", "rdk"},
                {"service.instance.id", "remote-debugger"},
                {"deployment.environment", "rdk-device"},
                {"telemetry.sdk.name", "opentelemetry"},
                {"telemetry.sdk.language", "cpp"},
                {"telemetry.sdk.version", "1.23.0"}
            };
            auto res = resource::Resource::Create(resource_attributes);
            
            // ✅ Create OTLP HTTP exporter
            otlp::OtlpHttpExporterOptions exporter_opts;
            exporter_opts.url = std::string(collectorEndpoint) + "/v1/traces";
            
            auto exporter = std::make_unique<otlp::OtlpHttpExporter>(exporter_opts);
            
            // ✅ CRITICAL FIX #6: Use SimpleSpanProcessor (synchronous, immediate export)
            // vs BatchSpanProcessor (asynchronous, buffered)
            // SimpleSpanProcessor: Each span exported immediately when ended
            auto processor = std::make_unique<trace_sdk::SimpleSpanProcessor>(std::move(exporter));
            
            // ✅ Create TracerProvider with Resource (MUST pass resource)
            auto provider = std::make_shared<trace_sdk::TracerProvider>(
                std::move(processor),  // span processor
                res                    // CRITICAL: resource with attributes
            );
            
            // Set as global provider
            trace::Provider::SetTracerProvider(nostd::shared_ptr<trace::TracerProvider>(provider));
            
            // Get tracer from provider
            tracer_ = provider->GetTracer(serviceName, "1.0.0");
        }
        catch (const std::exception& e) {
            // Log error if needed
        }
    }

    uint64_t StartSpan(const char *spanName, const char *attributes, 
                       const char *traceParent) {
        try {
            nostd::shared_ptr<trace::Span> span;
            
            if (traceParent && traceParent[0] != '\0') {
                // ✅ CRITICAL FIX #1 + #2: Parse parent and create with StartSpanOptions
                // W3C Format: 00-<32 hex chars>-<16 hex chars>-<2 hex chars>
                char trace_id[33] = {0};
                char span_id[17] = {0};
                char flags[3] = {0};
                
                // Extract parts from W3C format
                // Expected: 00-<traceId>-<spanId>-<flags>
                std::sscanf(traceParent, "%*2c-%32[^-]-%16[^-]-%2s", trace_id, span_id, flags);
                
                // ✅ Convert W3C hex to SpanContext (FIX #1)
                auto parent_context = createSpanContextFromIds(trace_id, span_id, flags);
                
                // ✅ CRITICAL: Create StartSpanOptions with parent (FIX #2)
                // This is what creates the parent-child relationship
                trace::StartSpanOptions options;
                options.parent = parent_context;  // CRITICAL - links to parent trace
                
                // Create child span with parent context
                span = tracer_->StartSpan(spanName, options);
            } else {
                // No parent - create root span
                span = tracer_->StartSpan(spanName);
            }
            
            if (span) {
                // Set attributes if provided
                if (attributes) {
                    span->SetAttribute("custom_attributes", attributes);
                }
                
                // ✅ CRITICAL FIX #3: Activate scope so this span becomes current
                // This is thread-local and makes this span active for nested operations
                t_active_scope_ = std::make_unique<trace::Scope>(span);
                
                // ✅ CRITICAL FIX #5: Thread-safe span tracking
                {
                    std::lock_guard<std::mutex> lock(spans_mutex_);
                    active_spans_[std::this_thread::get_id()] = span;
                }
                
                // Return span handle (pointer)
                return reinterpret_cast<uint64_t>(span.get());
            }
        }
        catch (const std::exception& e) {
            // Log error if needed
        }
        
        return 0;
    }

    int EndSpan(uint64_t spanHandle) {
        try {
            std::lock_guard<std::mutex> lock(spans_mutex_);
            auto thread_id = std::this_thread::get_id();
            auto it = active_spans_.find(thread_id);
            
            if (it != active_spans_.end()) {
                // End the span (marks end time, triggers export)
                it->second->End();
                active_spans_.erase(it);
                
                // ✅ Deactivate scope
                if (t_active_scope_) t_active_scope_.reset();
                
                return 0;
            }
        }
        catch (const std::exception& e) {
            // Log error if needed
        }
        return -1;
    }

    int LogEvent(const char *eventName, const char *eventData) {
        try {
            std::lock_guard<std::mutex> lock(spans_mutex_);
            auto it = active_spans_.find(std::this_thread::get_id());
            
            if (it != active_spans_.end()) {
                // Add event to active span
                if (eventData) {
                    // Event with attributes
                    it->second->AddEvent(eventName, {{"data", eventData}});
                } else {
                    // Event without attributes
                    it->second->AddEvent(eventName);
                }
                return 0;
            }
        }
        catch (const std::exception& e) {
            // Log error if needed
        }
        return -1;
    }

    int Shutdown() {
        try {
            tracer_ = nullptr;
            return 0;
        }
        catch (const std::exception& e) {
            return -1;
        }
    }
};

// Thread-local scope variable definition
thread_local std::unique_ptr<trace::Scope> RrdOtelWrapper::t_active_scope_;

// ===== Global wrapper instance =====
static std::unique_ptr<RrdOtelWrapper> g_wrapper;
static std::mutex g_wrapper_mutex;

// ===== C API Implementation =====

extern "C" {

int rrdOtel_Initialize_Cpp(const char *serviceName, const char *collectorEndpoint) {
    try {
        std::lock_guard<std::mutex> lock(g_wrapper_mutex);
        if (!g_wrapper) {
            g_wrapper = std::make_unique<RrdOtelWrapper>(serviceName, collectorEndpoint);
        }
        return 0;
    }
    catch (...) {
        return -1;
    }
}

uint64_t rrdOtel_StartSpan_Cpp(const char *spanName, const char *attributes,
                               const char *traceParent) {
    try {
        if (g_wrapper) {
            return g_wrapper->StartSpan(spanName, attributes, traceParent);
        }
    }
    catch (...) {
    }
    return 0;
}

int rrdOtel_EndSpan_Cpp(uint64_t spanHandle) {
    try {
        if (g_wrapper) {
            return g_wrapper->EndSpan(spanHandle);
        }
    }
    catch (...) {
    }
    return -1;
}

int rrdOtel_LogEvent_Cpp(const char *eventName, const char *eventData) {
    try {
        if (g_wrapper) {
            return g_wrapper->LogEvent(eventName, eventData);
        }
    }
    catch (...) {
    }
    return -1;
}

int rrdOtel_Shutdown_Cpp() {
    try {
        std::lock_guard<std::mutex> lock(g_wrapper_mutex);
        if (g_wrapper) {
            int result = g_wrapper->Shutdown();
            g_wrapper.reset();
            return result;
        }
    }
    catch (...) {
    }
    return 0;
}

} // extern "C"
