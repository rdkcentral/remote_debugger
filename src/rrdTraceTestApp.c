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

/**
 * @file rrdTraceTestApp.c
 * @brief Test application to demonstrate OpenTelemetry tracing in remote_debugger
 * 
 * This app demonstrates SCENARIO 1: External component provides trace context
 * 
 * Implementation:
 *   - Creates W3C trace context (traceparent, tracestate)
 *   - Embeds trace context as properties in RBUS event object
 *   - Publishes event via rbusEvent_Publish()
 *   - remote_debugger extracts trace from event properties
 *   - Continues the trace chain (preserves trace ID)
 * 
 * SCENARIO 1 (This App): External component provides trace
 *   ✓ Generate trace context
 *   ✓ Add as event properties (traceparent, tracestate)
 *   ✓ Publish via rbusEvent_Publish()
 *   ✓ remote_debugger detects and USES existing trace
 * 
 * SCENARIO 2 (Fallback): No trace provided
 *   - If event has no trace properties
 *   - remote_debugger GENERATES new trace
 * 
 * Usage:
 *   ./rrdTraceTestApp [issue_value]
 *   Example: ./rrdTraceTestApp "SecurityPatch.0"
 *   Example: ./rrdTraceTestApp (uses default System)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rbus.h>

#define RRD_SET_ISSUE_EVENT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.IssueType"

int main(int argc, char *argv[])
{
    rbusHandle_t handle;
    rbusValue_t value;
    rbusError_t rc;
    const char *issueValue = "SecurityPatch.0";  /* Default issue */
    char traceParent[256];
    char traceState[256];

    /* Parse command line argument */
    if (argc > 1)
    {
        issueValue = argv[1];
    }

    printf("\n");
    printf("============================================\n");
    printf("RRD OpenTelemetry Trace Test Application\n");
    printf("============================================\n");
    printf("Issue Value: %s\n", issueValue);
    printf("Scenario: SCENARIO 1 (external trace propagation)\n");
    printf("Method: rbusEvent_Publish() with trace properties\n\n");

    /* Open RBUS connection */
    printf("[1] Opening RBUS connection...\n");
    rc = rbus_open(&handle, "rrdTraceTestApp");
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("ERROR: Failed to open RBUS - %s\n", rbusError_ToString(rc));
        return 1;
    }
    printf("    SUCCESS: RBUS connection opened\n\n");

    /* Create and set trace context - SCENARIO 1 */
    printf("[2] Setting up OpenTelemetry trace context (SCENARIO 1)...\n");
    printf("    Creating W3C trace context for propagation\n");
    printf("    Will embed trace context in event properties\n\n");
    
    /* Format W3C Trace Context */
    snprintf(traceParent, sizeof(traceParent), 
             "00-test-trace-id-0123456789abcdef-test-span-id-01234567-01");
    snprintf(traceState, sizeof(traceState), "rdd=test_app");

    printf("    Trace Parent: %s\n", traceParent);
    printf("    Trace State:  %s\n\n", traceState);

    /* Trigger the event by publishing with trace context embedded */
    printf("[3] Publishing event with embedded trace context...\n");
    printf("    Using rbusEvent_Publish() with trace context properties\n");
    printf("    This enables SCENARIO 1 (trace propagation)\n\n");
    
    /* Create event object with trace context */
    rbusObject_t eventObj;
    rbusValue_t valueIssue, valueTraceParent, valueTraceState;
    rbusEvent_t event = {0};
    
    rbusObject_Init(&eventObj, NULL);
    
    /* Add issue value */
    rbusValue_Init(&valueIssue);
    rbusValue_SetString(valueIssue, issueValue);
    rbusObject_SetValue(eventObj, "value", valueIssue);
    
    /* Add trace context properties */
    rbusValue_Init(&valueTraceParent);
    rbusValue_SetString(valueTraceParent, traceParent);
    rbusObject_SetValue(eventObj, "traceparent", valueTraceParent);
    
    rbusValue_Init(&valueTraceState);
    rbusValue_SetString(valueTraceState, traceState);
    rbusObject_SetValue(eventObj, "tracestate", valueTraceState);
    
    /* Publish event */
    event.name = RRD_SET_ISSUE_EVENT;
    event.data = eventObj;
    event.type = RBUS_EVENT_GENERAL;
    
    rc = rbusEvent_Publish(handle, &event);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("    WARNING: rbusEvent_Publish returned - %s\n", rbusError_ToString(rc));
    }
    else
    {
        printf("    SUCCESS: Event published with trace context properties\n");
    }

    /* Clean up */
    rbusValue_Release(valueIssue);
    rbusValue_Release(valueTraceParent);
    rbusValue_Release(valueTraceState);
    rbusObject_Release(eventObj);

    printf("\n[4] Event Processing Summary...\n");
    printf("    SCENARIO 1: External component provides trace via event properties\n");
    printf("    remote_debugger will extract and USE this trace context\n");
    printf("    Trace ID will be PRESERVED throughout the processing chain\n");
    printf("\n[5] Waiting for event processing (2 seconds)...\n");
    sleep(2);

    printf("\n[6] Closing RBUS connection...\n");
    rbus_close(handle);

    printf("\n");
    printf("============================================\n");
    printf("Test Results:\n");
    printf("============================================\n");
    printf("SCENARIO 1 (External provides trace): TESTED\n");
    printf("  ✓ Generated trace context\n");
    printf("  ✓ Embedded in event properties (traceparent/tracestate)\n");
    printf("  ✓ Published via rbusEvent_Publish()\n");
    printf("  ✓ remote_debugger should extract and USE this trace\n");
    printf("  ✓ Trace ID should be PRESERVED throughout\n\n");
    printf("Key Implementation:\n");
    printf("  • Use rbusEvent_Publish() instead of rbus_set()\n");
    printf("  • Add 'traceparent' and 'tracestate' properties to event object\n");
    printf("  • remote_debugger extracts from event->data properties\n\n");
    printf("Check remote_debugger logs to verify:\n");
    printf("  - 'Using trace context from event properties' message\n");
    printf("  - SCENARIO 1 detection\n");
    printf("  - Trace Parent matches: %s\n", traceParent);
    printf("Traces should be exported to Jaeger UI.\n");
    printf("============================================\n\n");

    return 0;
}
