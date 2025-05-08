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
#define RRD_TMP_DIR "/tmp/"
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

#ifdef IARMBUS_SUPPORT
#ifdef USE_L2_SUPPORT
   subscriptions[2].eventName = RDM_DOWNLOAD_EVENT;
   subscriptions[2].filter = NULL;
   subscriptions[2].duration = 0;
   subscriptions[2].handler  = _rdmDownloadEventHandler;
   subscriptions[2].userData = NULL;
   ret = rbusEvent_SubscribeEx(rrdRbusHandle, subscriptions, 3, 60);
#else
   ret = rbusEvent_SubscribeEx(rrdRbusHandle, subscriptions, 2, 60);
#endif
#else
   subscriptions[2].eventName = RDM_DOWNLOAD_EVENT;
   subscriptions[2].filter = NULL;
   subscriptions[2].duration = 0;
   subscriptions[2].handler  = _rdmDownloadEventHandler;
   subscriptions[2].userData = NULL;
   ret = rbusEvent_SubscribeEx(rrdRbusHandle, subscriptions, 3, 60);
#endif
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
    data_buf *sendbuf;
    int recPkglen = 0, rrdjsonlen = 0, recPkgNamelen = 0;
    cacheData *cache = NULL;

    rbusError_t retCode = RBUS_ERROR_BUS_ERROR;
    rbusValue_t value = NULL;
    rbusValue_Init(&value);
    char const* issue = NULL;
    retCode = rbus_get(rrdRbusHandle, RRD_SET_ISSUE_EVENT, &value);
    if(retCode != RBUS_ERROR_SUCCESS || value == NULL)
    {
         RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: RBUS get failed for the event [%s]\n", __FUNCTION__, __LINE__, RRD_SET_ISSUE_EVENT);
	 return;
    }	
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: issue type_value: = [%s]\n", __FUNCTION__, __LINE__, rbusValue_GetString(value, NULL));
    issue =rbusValue_GetString(value, NULL);
    char *dot_position = strchr(issue, '.'); // Find the first occurrence of '.'
    if (dot_position != NULL) 
    {
        *dot_position = '\0'; // Replace '.' with null terminator
    }
    size_t len = strlen(RDM_PKG_PREFIX) + strlen(issue) + 1;

    char *pkg_name = (char *)malloc(len);
    if(pkg_name == NULL)
    {
        return;
    }
    strncpy(pkg_name, RDM_PKG_PREFIX, strlen(RDM_PKG_PREFIX) + 1);
    strncat(pkg_name, issue, len - strlen(RDM_PKG_PREFIX) - 1);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: pkg_name : [%s]\n",  __FUNCTION__, __LINE__, pkg_name);

    char *pkg_inst_path = (char *)malloc(strlen(RRD_TMP_DIR) + strlen(pkg_name) + 1);
    if( pkg_inst_path == NULL)
    {
        return;
    }
    snprintf(pkg_inst_path, strlen(RRD_TMP_DIR) + strlen(pkg_name) + 1, "%s%s", RRD_TMP_DIR, pkg_name);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: pkg_inst_path : [%s]\n",  __FUNCTION__, __LINE__, pkg_inst_path);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);

    (void)(handle);
    (void)(subscription);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for RDM_DOWNLOAD_EVENT %s \n", __FUNCTION__, __LINE__, RDM_DOWNLOAD_EVENT);
    cache = findPresentInCache(pkg_name);
    if (cache != NULL)
    {
    	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package found in Cache...%s \n", __FUNCTION__, __LINE__, cache->issueString);
    	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package Details jsonPath: %s \n", __FUNCTION__, __LINE__, pkg_inst_path);
    	rrdjsonlen = strlen(RRD_JSON_FILE);
    	recPkglen = strlen(pkg_inst_path) + 1;
    	recPkgNamelen = strlen(cache->issueString) + 1;
    	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:recPkgNamelen=%d recPkglen=%d rrdjsonlen=%d \n", __FUNCTION__, __LINE__, recPkgNamelen, recPkglen, rrdjsonlen);
	sendbuf = (data_buf *)malloc(sizeof(data_buf));
    	RRD_data_buff_init(sendbuf, EVENT_MSG, RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE);
    	sendbuf->mdata = (char *) calloc(recPkgNamelen, sizeof(char));
	if(!sendbuf->mdata)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for the rdm download event \n", __FUNCTION__, __LINE__);
            RRD_data_buff_deAlloc(sendbuf);
            return;
        }
	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:JSON_PATH_LEN=%d \n", __FUNCTION__, __LINE__, recPkglen + rrdjsonlen);
    	sendbuf->jsonPath = (char *)calloc(recPkglen + rrdjsonlen, sizeof(char));
    	if (!sendbuf->jsonPath)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for the rdm download event \n", __FUNCTION__, __LINE__);
            RRD_data_buff_deAlloc(sendbuf);
            return;
        }
	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Cache.issueString=%s Cache.issueString.Len=%d\n", __FUNCTION__, __LINE__, cache->issueString, strlen(cache->issueString));
    	strncpy((char *)sendbuf->mdata, cache->issueString, recPkgNamelen);
	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType: %s...\n", __FUNCTION__, __LINE__, (char *)sendbuf->mdata);
        snprintf(sendbuf->jsonPath, strlen(pkg_inst_path) + rrdjsonlen + 1, "%s%s", pkg_inst_path, RRD_JSON_FILE);
    	sendbuf->inDynamic = true;
	if (checkAppendRequest(sendbuf->mdata))
    	{
        	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:Received command apppend request for the issue \n", __FUNCTION__, __LINE__);
        	sendbuf->inDynamic = false;
        	sendbuf->appendMode = true;
    	}		    
    	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType: %s... jsonPath: %s... \n", __FUNCTION__, __LINE__, (char *)sendbuf->mdata, sendbuf->jsonPath);
    	RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Copying Message Received to the queue.. \n", __FUNCTION__, __LINE__);
    	RRDMsgDeliver(msqid, sendbuf);
	RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: SUCCESS: Message sending Done, ID=%d MSG=%s Size=%d Type=%u AppendMode=%d! \n", __FUNCTION__, __LINE__, msqid, sendbuf->mdata, strlen(sendbuf->mdata), sendbuf->mtype, sendbuf->appendMode);
	remove_item(cache);
    }
    else
    {
    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Package not requested... %s \n", __FUNCTION__, __LINE__, pkg_name);
    }
    free(pkg_name);
    free(pkg_inst_path);
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
    RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: before copy debug print : = [%s]\n", __FUNCTION__, __LINE__, rbusValue_GetString(value, NULL));
    strncpy(dataMsg, rbusValue_GetString(value, NULL), len);
    RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: rbus valu string debug print : %s \n", __FUNCTION__, __LINE__, dataMsg);
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
    rbusEvent_UnsubscribeEx(rrdRbusHandle, subscriptions, 3);
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

