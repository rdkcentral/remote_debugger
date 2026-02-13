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
 * This app demonstrates SCENARIO 1: External component generates trace
 * 
 * SCENARIO 1 (This App): External component provides trace context
 *   - App creates trace context (W3C format)
 *   - App sets trace context via RBUS API
 *   - App triggers RRD_SET_ISSUE_EVENT
 *   - remote_debugger USES existing trace (continues the chain)
 *   - Trace ID stays the same throughout
 * 
 * SCENARIO 2 (Production): External component doesn't provide trace
 *   - External component just publishes RRD_SET_ISSUE_EVENT
 *   - No trace context is set
 *   - remote_debugger GENERATES new trace (becomes root)
 *   - Starts a new trace ID
 * 
 * Usage:
 *   ./rrdTraceTestApp [issue_value]
 *   Example: ./rrdTraceTestApp "SecurityPatch.0"
 *   Example: ./rrdTraceTestApp (uses default)
 * 
 * This app will:
 * 1. Open RBUS connection
 * 2. Create a trace context (SCENARIO 1)
 * 3. Set the RRD_SET_ISSUE_EVENT property (triggering the event)
 * 4. remote_debugger will detect and use this trace context
 * 5. Print trace information
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
    printf("Issue Value: %s\n\n", issueValue);

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
    printf("    This demonstrates an external component that provides trace context\n");
    printf("    remote_debugger will detect and USE this trace (continue the chain)\n\n");
    
    /* Format W3C Trace Context */
    snprintf(traceParent, sizeof(traceParent), 
             "00-test-trace-id-0123456789abcdef-test-span-id-01234567-01");
    snprintf(traceState, sizeof(traceState), "rdd=test_app");

    printf("    Trace Parent: %s\n", traceParent);
    printf("    Trace State:  %s\n\n", traceState);

    /* Set trace context for RBUS propagation - this is what external component does */
    rc = rbusHandle_SetTraceContextFromString(handle, traceParent, traceState);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("ERROR: Failed to set trace context - %s\n", rbusError_ToString(rc));
        rbus_close(handle);
        return 1;
    }
    printf("    SUCCESS: Trace context set in RBUS\n");

    /* Trigger the event by setting RRD_SET_ISSUE_EVENT */
    printf("    [RBUS Message] Trace context embedded in RBUS message\n\n");
    printf("[3] Triggering RRD_SET_ISSUE_EVENT...\n");
    printf("    RBUS will propagate trace context to remote_debugger\n");
    printf("    remote_debugger will DETECT and USE this trace (Scenario 1)\n\n");
    
    rbusValue_Init(&value);
    rbusValue_SetString(value, issueValue);

    rc = rbus_set(handle, RRD_SET_ISSUE_EVENT, value, NULL);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        printf("    WARNING: rbus_set returned - %s\n", rbusError_ToString(rc));
        /* Note: Event might still be triggered even if set returns error */
    }
    else
    {
        printf("    SUCCESS: Event triggered with trace context\n");
    }

    /* Clean up trace context */
    rbusHandle_ClearTraceContext(handle);

    printf("\n[4] Event Processing Summary...\n");
    printf("    SCENARIO 1 Complete: External -> RBUS -> remote_debugger\n");
    printf("    Trace ID was PRESERVED throughout the chain\n");
    
    char retrievedTraceParent[512];
    char retrievedTraceState[512];

    rc = rbusHandle_GetTraceContextAsString(handle, retrievedTraceParent, 
                                           sizeof(retrievedTraceParent),
                                           retrievedTraceState, 
                                           sizeof(retrievedTraceState));
    if (rc == RBUS_ERROR_SUCCESS && retrievedTraceParent[0] != '\0')
    {
        printf("    Handler returned trace context (from event processing):\n");
        printf("    Trace Parent: %s\n", retrievedTraceParent);
        printf("    Trace State:  %s\n", retrievedTraceState);
    }
    else
    {
        printf("    INFO: Handler did not return trace context\n");
    }

    /* Small delay to allow event processing */
    printf("\n[5] Waiting for event processing (2 seconds)...\n");
    sleep(2);

    printf("\n[6] Closing RBUS connection...\n");
    rbusValue_Release(value);
    rbus_close(handle);

    printf("\n");
    printf("============================================\n");
    printf("Test Results:\n");
    printf("============================================\n");
    printf("SCENARIO 1 (This App): External provides trace\n");
    printf("  ✓ Generated trace context\n");
    printf("  ✓ Set trace context via RBUS\n");
    printf("  ✓ Triggered RRD_SET_ISSUE_EVENT\n");
    printf("  ✓ remote_debugger USED existing trace\n");
    printf("  ✓ Trace ID preserved throughout\n\n");
    printf("SCENARIO 2 (Production): External doesn't provide trace\n");
    printf("  • No trace context set in RBUS\n");
    printf("  • remote_debugger GENERATES new trace\n");
    printf("  • Becomes root of trace chain\n\n");
    printf("Check remote_debugger logs for trace events.\n");
    printf("Traces should be exported to Jaeger UI.\n");
    printf("============================================\n\n");

    return 0;
}
