/*
 *
 * If not stated otherwise in this file or this component's Licenses.txt file the
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

#include "rrdInterface.h"
#include "rrdRbus.h"
#include "rrdRunCmdThread.h"
#if !defined(GTEST_ENABLE)
#include "webconfig_framework.h"

extern int msqid;
#else
int msqid = 0;
key_t key = 1234;
#endif
uint32_t gWebCfgBloBVersion = 0;
rbusHandle_t    rrdRbusHandle;

/*Function: RRD_subscribe
 *Details: This helps to perform Bus init/connect and event handler registration for receiving
 *events from the TR181 parameter.
 *Input: NULL
 *Output: 0 for success and non 0 for failure
 */

int RRD_subscribe()
{
    int ret = 0;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);
#ifdef IARMBUS_SUPPORT
    ret = RRD_IARM_subscribe();
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Subscribe!!! \n ", __FUNCTION__, __LINE__);
	return ret;
    }
#endif
    //RBUS Event Subscribe for RRD
    ret = rbus_open(&rrdRbusHandle, REMOTE_DEBUGGER_RBUS_HANDLE_NAME);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Open Failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: RBUS Open! \n", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
    subscriptions[0].eventName = RRD_SET_ISSUE_EVENT;
    subscriptions[0].filter = NULL;
    subscriptions[0].duration = 0;
    subscriptions[0].handler  = _remoteDebuggerEventHandler;
    subscriptions[0].userData = NULL;

    subscriptions[1].eventName = RRD_WEBCFG_ISSUE_EVENT;
    subscriptions[1].filter = NULL;
    subscriptions[1].duration = 0;
    subscriptions[1].handler  = _remoteDebuggerWebCfgDataEventHandler;
    subscriptions[1].userData = NULL;

    subscriptions[2].eventName = RDM_DOWNLOAD_EVENT;
    subscriptions[2].filter = NULL;
    subscriptions[2].duration = 0;
    subscriptions[2].handler  = _rdmDownloadEventHandler;
    subscriptions[2].userData = NULL;

    ret = rbusEvent_SubscribeEx(rrdRbusHandle, subscriptions, 3, 60);
#endif
    if(ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Event Subscribe for RRD return value is : %s \n ", __FUNCTION__, __LINE__, rbusError_ToString((rbusError_t)ret));
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: RBUS Event Subscribe for RRD done! \n", __FUNCTION__, __LINE__);
    }

    webconfigFrameworkInit();
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting.. \n", __FUNCTION__, __LINE__);
    return ret;
}

bool checkAppendRequest(char *issueRequest)
{
    size_t issuestr_len = strlen(issueRequest);
    size_t suffixstr_len = strlen(APPEND_SUFFIX);
    char *suffixptr = NULL;

    suffixptr = issueRequest + issuestr_len - suffixstr_len;

    if (issuestr_len >= suffixstr_len)
    {
        if (strcmp(suffixptr, APPEND_SUFFIX) == 0)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:Remove suffix from the issuetype to process the request \n", __FUNCTION__, __LINE__);
            issueRequest[issuestr_len - suffixstr_len] = '\0';
            return true;
        }
    }
    return false;
}

void webconfigFrameworkInit()
{
    char *sub_doc = "remotedebugger";

    blobRegInfo *blobData;
    blobData = (blobRegInfo*) malloc( sizeof(blobRegInfo));
    memset(blobData, 0, sizeof(blobRegInfo));
    strncpy( blobData->subdoc_name, sub_doc, strlen(sub_doc) + 1);

    register_sub_docs(blobData, 1 /*SubDoc Count*/, NULL, NULL);
}

uint32_t getBlobVersion(char* subdoc)
{
        return gWebCfgBloBVersion;
}

/* API to update the subdoc version */
int setBlobVersion(char* subdoc,uint32_t version)
{
        gWebCfgBloBVersion = version;
        return 0;
}

void RRDMsgDeliver(int msgqid, data_buf *sbuf)
{
    msgRRDHdr msgHdr;
    size_t msgLen = -1;
    msgHdr.type = RRD_EVENT_MSG_REQUEST;
    msgHdr.mbody = (void *)sbuf;
    msgLen = sizeof(msgHdr.mbody);

    if (msgsnd(msgqid, (void *)&msgHdr, msgLen, 0) < 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Message Sending failed with ID=%d MSG=%s Size=%d Type=%u MbufSize=%d !!! \n", __FUNCTION__, __LINE__, msgqid, sbuf->mdata, sizeof(sbuf->mdata), sbuf->mtype, msgLen);
        exit(1);
    }
}

/*Function:  RRD_data_buff_init
 *  *Details: Initialize the data buffer for MSG Queue
 *  *Input: Pointer to Data Buffer and Event Id
 *  *Output: NULL
 *    */
void RRD_data_buff_init(data_buf *sbuf, message_type_et sndtype, deepsleep_event_et deepSleepEvent)
{
    sbuf->mtype = sndtype;
    sbuf->mdata = NULL;
    sbuf->jsonPath = NULL;
    sbuf->inDynamic = false;
    sbuf->appendMode = false;
    sbuf->dsEvent = deepSleepEvent;
}

/*Function:  RRD_data_buff_deAlloc
 *  *Details: De Aollocate Data Buffer
 *  *Input: Pointer to Data Buffer
 *  *Output:void
 *    */
void RRD_data_buff_deAlloc(data_buf *sbuf)
{
    if (sbuf)
    {
        if (sbuf->mdata)
        {
            free(sbuf->mdata);
        }

        if (sbuf->jsonPath)
        {
            free(sbuf->jsonPath);
        }
        free(sbuf);
    }
}

/*
 * @function _remoteDebuggerEventHandler
 * @brief Receives the RBUS event and sends the value as a message in the message-queue to the thread function.
 * @param rbusHandle_t handle - RBUS handle.
 * @param rbusEvent_t const* event - RBUS event object.
 * @param rbusEventSubscription_t* subscription - RBUS event subscription object.
 * @return void
 */
#if !defined(GTEST_ENABLE)
void _rdmDownloadEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);	
}
void _remoteDebuggerEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    char *dataMsg = NULL;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);

    (void)(handle);
    (void)(subscription);

    rbusValue_t value = rbusObject_GetValue(event->data, "value");

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for RRD_SET_ISSUE_EVENT %s \n", __FUNCTION__, __LINE__, RRD_SET_ISSUE_EVENT);
    if(!value)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: event->data value is NULL \n", __FUNCTION__, __LINE__);
        return;
    }

    int len = strlen(rbusValue_GetString(value, NULL));
    dataMsg = (char *) calloc(1, len);
    if(!dataMsg)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Memory Allocation Failed for %s \n", __FUNCTION__, __LINE__, rbusValue_ToString(value, NULL, 0));
        return;
    }
    strncpy(dataMsg, rbusValue_GetString(value, NULL), len);
    if (dataMsg[0] == '\0' || len <= 0  )
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Message Received is empty, Exit Processing!!! \n", __FUNCTION__, __LINE__);
    }
    else
    {
        pushIssueTypesToMsgQueue(dataMsg, EVENT_MSG);
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
}

void _remoteDebuggerWebCfgDataEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    char *inString = NULL;

    (void)(handle);
    (void)(subscription);

    rbusValue_t value = rbusObject_GetValue(event->data, "value");

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for RRD_WEBCFG_ISSUE_EVENT %s \n", __FUNCTION__, __LINE__, RRD_WEBCFG_ISSUE_EVENT);
    if (value)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Data from TR69 Parameter for REMOTE_DEBUGGER_WEBCFGDATA %s \n", __FUNCTION__, __LINE__, 
			                        rbusValue_ToString(value, NULL, 0));
        int len = strlen(rbusValue_GetString(value, NULL));
        inString = (char *)calloc(1, len);
        if(inString)
        {
            strncpy(inString, rbusValue_GetString(value, NULL), len);
            pushIssueTypesToMsgQueue(inString, EVENT_WEBCFG_MSG);
        }
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exit... \n", __FUNCTION__, __LINE__);
}
#endif
void pushIssueTypesToMsgQueue(char *issueTypeList, message_type_et sndtype)
{
    data_buf *sbuf = NULL;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Copying Message Received to the queue.. \n", __FUNCTION__, __LINE__);
    sbuf = (data_buf *)malloc(sizeof(data_buf));
    if (!sbuf)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed\n", __FUNCTION__, __LINE__);
    }
    else
    {
        RRD_data_buff_init(sbuf, sndtype, RRD_DEEPSLEEP_INVALID_DEFAULT);
        sbuf->mdata = issueTypeList;
        if (checkAppendRequest(sbuf->mdata))
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:Received command apppend request for the issue \n", __FUNCTION__, __LINE__);
            sbuf->appendMode = true;
        }	
        RRDMsgDeliver(msqid, sbuf);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: SUCCESS: Message sending Done, ID=%d MSG=%s Size=%d Type=%u AppendMode=%d! \n", __FUNCTION__, __LINE__, msqid, sbuf->mdata, strlen(sbuf->mdata), sbuf->mtype, sbuf->appendMode);
    }
}

/*Function: RRD_unsubscribe
 *Details: This helps to perform Bus disconnect/terminate and unregister event handler.
 *Input: NULL
 *Output: 0 for success and non-zero for failure
 */

int RRD_unsubscribe()
{
    int ret = 0;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);
#ifdef IARMBUS_SUPPORT
    ret = RRD_IARM_unsubscribe();
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Unsubscribe failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus Unsubscribe done!\n", __FUNCTION__, __LINE__);
#endif
#if !defined(GTEST_ENABLE)
    rbusEvent_UnsubscribeEx(rrdRbusHandle, subscriptions, 2);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Unsubscribe EventHandler for RRD failed!!! \n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = rbus_close(rrdRbusHandle);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Termination failed!!! \n ", __FUNCTION__, __LINE__);
	return ret;
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: RBUS Termination done!\n", __FUNCTION__, __LINE__);
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
#endif
    return ret;
}

