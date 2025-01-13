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

#include "rrdIarm.h"
#include "rrdRunCmdThread.h"
#include "rrdRbus.h"
#if !defined(GTEST_ENABLE)
#include "webconfig_framework.h"

extern int msqid;
#else
int msqid = 0;
key_t key = 1234;
#endif


uint32_t gWebCfgBloBVersion = 0;

/*
 * @function RRD_IARM_subscribe
 * @brief Initializes and connects to IARMBus, and registers event handlers for receiving IARM events
 *              from the TR181 parameter.
 * @param None.
 * @return int - Returns 0 for success, and non-0 for failure.
 */

IARM_Result_t RRD_subscribe()
{
    int ret = 0;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Init failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Init done! \n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Connect();
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Connect failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Connect done! \n", __FUNCTION__, __LINE__);


    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, _remoteDebuggerEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RRD failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA, _remoteDebuggerWebCfgDataEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RRD failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_RegisterEventHandler for RRD done! \n", __FUNCTION__, __LINE__);

    // IARM Reg for RDM Event Handler
    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS, _rdmManagerEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RDMMGR failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }

    // IARM Reg for Deep Sleep Event Handler
    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _pwrManagerEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RDMMGR failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }

    webconfigFrameworkInit();

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_RegisterEventHandler for RDMMGR done! \n", __FUNCTION__, __LINE__);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting.. \n", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
}

/*
 * @function webconfigFrameworkInit
 * @brief Initializes the web config framework by registering the sub-document ("remotedebugger")
 *              and setting up version handlers.
 * @param None.
 * @return void
 */
void webconfigFrameworkInit()
{
    char *sub_doc = "remotedebugger";
    blobRegInfo *blobData;
    blobData = (blobRegInfo*) malloc( sizeof(blobRegInfo));
    memset(blobData, 0, sizeof(blobRegInfo));
    strncpy( blobData->subdoc_name, sub_doc, strlen(sub_doc) + 1);

    register_sub_docs(blobData, 1 /*SubDoc Count*/, NULL, NULL);
}

/*
 * @function getBlobVersion
 * @brief Retrieves the current version of the specified sub-document.
 * @param char *subdoc - The name of the sub-document.
 * @return uint32_t - Returns the version number of the specified sub-document.
 */
uint32_t getBlobVersion(char* subdoc)
{
        return gWebCfgBloBVersion;
}

/*
 * @function setBlobVersion
 * @brief Updates the version of the specified sub-document.
 * @param char *subdoc - The name of the sub-document.
 * @param uint32_t version - The version number to set for the sub-document.
 * @return int - Returns 0 on success.
 */
int setBlobVersion(char* subdoc,uint32_t version)
{
        gWebCfgBloBVersion = version;
        return 0;
}

/*
 * @function RRDMsgDeliver
 * @brief Sends a message to the specified message queue.
 * @param int msgqid - The message queue identifier.
 * @param data_buf *sbuf - Pointer to the data buffer to be sent.
 * @return void
 */
void RRDMsgDeliver(int msgqid, data_buf *sbuf)
{
    msgRRDHdr msgHdr;
    size_t msgLen = -1;
    msgHdr.type = RRD_EVENT_MSG_REQUEST;
    msgHdr.mbody = (void *)sbuf;
    msgLen = sizeof(msgHdr.mbody);

    if (msgsnd(msgqid, (void *)&msgHdr, msgLen, 0) < 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Message Sending failed with ID=%d MSG=%s Size=%d Type=%u MbufSize=%d Error: %s !!! \n", __FUNCTION__, __LINE__, msgqid, sbuf->mdata, sizeof(sbuf->mdata), sbuf->mtype, msgLen, strerror(errno));
        exit(1);
    }
}

/*
 * @function RRD_data_buff_init
 * @brief Initializes the data buffer for the message queue.
 * @param data_buf *sbuf - Pointer to the data buffer to be initialized.
 * @param message_type_et sndtype - The message type.
 * @param deepsleep_event_et deepSleepEvent - The deep sleep event type.
 * @return void
 */
void RRD_data_buff_init(data_buf *sbuf, message_type_et sndtype, deepsleep_event_et deepSleepEvent)
{
    sbuf->mtype = sndtype;
    sbuf->mdata = NULL;
    sbuf->jsonPath = NULL;
    sbuf->inDynamic = false;
    sbuf->dsEvent = deepSleepEvent;
}

/*
 * @function RRD_data_buff_deAlloc
 * @brief Deallocates the memory used by the data buffer.
 * @param data_buf *sbuf - Pointer to the data buffer to be deallocated.
 * @return void
 */
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
 * @function  _pwrManagerEventHandler
 * @brief Receives the power manager event and sends the value as a message in the message-queue
 *              to the thread function.
 * @param const char *owner - Owner name of the event.
 *        IARM_EventId_t eventId - Event ID.
 *        void *data - Event data.
 *        size_t len - Size of the event data.
 * Output: void
 */
void _pwrManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    IARM_Bus_PWRMgr_EventData_t *eventData = NULL;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);

    if (strcmp(owner, IARM_BUS_PWRMGR_NAME) == 0)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for IARM_BUS_PWRMGR_NAME %s \n", __FUNCTION__, __LINE__, owner);
        eventData = (IARM_Bus_PWRMgr_EventData_t *)data;

        if ((eventData) && (eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP &&
                            eventData->data.state.newState != IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP))
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event ID found for IARM_BUS_RDK_REMOTE_DEBUGGER_DEEPSLEEP_AWAKE %d \n", __FUNCTION__, __LINE__, eventId);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Received state from Power Manager Current :[%d] New[%d] \n", __FUNCTION__, __LINE__, eventData->data.state.curState, eventData->data.state.newState);
            rbusError_t rc = RBUS_ERROR_BUS_ERROR;
            rbusValue_t value;
#if !defined(ENABLE_WEBCFG_FEATURE)
	    /* RDK-55159: Fix for Warning Unused Variable */
            data_buf *sbuf = NULL;
	    int msgLen = strlen(DEEP_SLEEP_STR) + 1;
#endif

#ifdef ENABLE_WEBCFG_FEATURE
            rc = rbus_open(&rrdRbusHandle, REMOTE_DEBUGGER_RBUS_HANDLE_NAME);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Open Failed!!! \n ", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
		RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Open Failed with Error %d !!! \n ", __FUNCTION__, __LINE__, rc);
#else
                return;
#endif
            }
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: RBUS Open! \n", __FUNCTION__, __LINE__);
            rc = RBUS_ERROR_BUS_ERROR; /* Re-assign failure to check rbus_set return */
            rbusValue_Init(&value);
            rbusValue_SetString(value,"root");
            rc = rbus_set(rrdRbusHandle, RRD_WEBCFG_FORCE_SYNC, value, NULL);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: rbus_set failed for [%s] with error [%d]\n\n", __FUNCTION__, __LINE__,RRD_WEBCFG_FORCE_SYNC ,rc);
            return;
            }
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Invoking WebCfg Force Sync: %s... \n", __FUNCTION__, __LINE__, RRD_WEBCFG_FORCE_SYNC);
#else
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Copying Message Received to the queue.. \n", __FUNCTION__, __LINE__);
            sbuf = (data_buf *)malloc(sizeof(data_buf));
            if (!sbuf)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                return;
            }

            RRD_data_buff_init(sbuf, IARM_DEEPSLEEP_EVENT_MSG, RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE);
            sbuf->mdata = (char *)malloc(msgLen);
            if (!sbuf->mdata)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                RRD_data_buff_deAlloc(sbuf);
                return;
            }
            strncpy((char *)sbuf->mdata, (const char *)DEEP_SLEEP_STR, msgLen);
            RRDMsgDeliver(msqid, sbuf);
            rc = rbus_close(rrdRbusHandle);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: RBUS Termination failed!!! \n ", __FUNCTION__, __LINE__);
            }
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: RBUS Termination done!\n", __FUNCTION__, __LINE__);
#endif
        }
        else
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event Triggred for DeepSleep %d \n", __FUNCTION__, __LINE__, eventId);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Owner Name found %s, use IARM_BUS_RDK_REMOTE_DEBUGGER_NAME!!! \n", __FUNCTION__, __LINE__, owner);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exit.. \n", __FUNCTION__, __LINE__);
}

/*
 * @function  _rdmManagerEventHandler
 * @brief Receives the RDM Manager event and sends the value as a message in the message-queue
 *              to the thread function.
 * @param const char *owner - Owner name of the event.
 *        IARM_EventId_t eventId - Event ID.
 *        void *data - Event data.
 *        size_t len - Size of the event data.
 * Output: void
 */
void _rdmManagerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    data_buf *sendbuf;
    IARM_Bus_RDMMgr_EventData_t *eventData;
    int recPkglen = 0, rrdjsonlen = 0, recPkgNamelen = 0;
    cacheData *cache = NULL;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);

    if (strcmp(owner, IARM_BUS_RDMMGR_NAME) == 0)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for IARM_BUS_RDMMGR_NAME %s \n", __FUNCTION__, __LINE__, owner);
        if (eventId == IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event ID found for IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS %d \n", __FUNCTION__, __LINE__, eventId);
            eventData = (IARM_Bus_RDMMgr_EventData_t *)data;
            cache = findPresentInCache(eventData->rdm_pkg_info.pkg_name);
            if (cache != NULL)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package found in Cache...%s \n", __FUNCTION__, __LINE__, cache->issueString);
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package Details jsonPath: %s, Installation Status: %d.. \n", __FUNCTION__, __LINE__, eventData->rdm_pkg_info.pkg_inst_path, (IARM_RDMMgr_Status_t)eventData->rdm_pkg_info.pkg_inst_status);

                if (eventData->rdm_pkg_info.pkg_inst_status == RDM_PKG_INSTALL_COMPLETE)
                {
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package Installation Status Complete... \n", __FUNCTION__, __LINE__);

                    rrdjsonlen = strlen(RRD_JSON_FILE);
                    recPkglen = strlen(eventData->rdm_pkg_info.pkg_inst_path) + 1;
                    recPkgNamelen = strlen(cache->issueString) + 1;
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:recPkgNamelen=%d recPkglen=%d rrdjsonlen=%d \n", __FUNCTION__, __LINE__, recPkgNamelen, recPkglen, rrdjsonlen);

                    sendbuf = (data_buf *)malloc(sizeof(data_buf));
                    if (!sendbuf)
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                        return;
                    }
                    if (!strcmp(cache->issueString, DEEP_SLEEP_STR))
                    {
                        RRD_data_buff_init(sendbuf, IARM_DEEPSLEEP_EVENT_MSG, RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE);
                    }
                    else
                    {
                        RRD_data_buff_init(sendbuf, IARM_EVENT_MSG, RRD_DEEPSLEEP_INVALID_DEFAULT);
                    }

                    sendbuf->mdata = (char *) calloc(recPkgNamelen, sizeof(char));
                    if(!sendbuf->mdata)
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                        RRD_data_buff_deAlloc(sendbuf);
                        return;
                    }
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:JSON_PATH_LEN=%d \n", __FUNCTION__, __LINE__, recPkglen + rrdjsonlen);
                    sendbuf->jsonPath = (char *)calloc(recPkglen + rrdjsonlen, sizeof(char));
                    if (!sendbuf->jsonPath)
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                        RRD_data_buff_deAlloc(sendbuf);
                        return;
                    }
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Cache.issueString=%s Cache.issueString.Len=%d\n", __FUNCTION__, __LINE__, cache->issueString, strlen(cache->issueString));
                    strncpy((char *)sendbuf->mdata, cache->issueString, recPkgNamelen);
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType: %s...\n", __FUNCTION__, __LINE__, (char *)sendbuf->mdata);
                    snprintf(sendbuf->jsonPath, strlen(eventData->rdm_pkg_info.pkg_inst_path) + rrdjsonlen + 1, "%s%s", eventData->rdm_pkg_info.pkg_inst_path, RRD_JSON_FILE);
                    sendbuf->inDynamic = true;

                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType: %s... jsonPath: %s... \n", __FUNCTION__, __LINE__, (char *)sendbuf->mdata, sendbuf->jsonPath);
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Copying Message Received to the queue.. \n", __FUNCTION__, __LINE__);
                    RRDMsgDeliver(msqid, sendbuf);
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: SUCCESS: Message sending Done, ID=%d MSG=%s Size=%d Type=%u! \n", __FUNCTION__, __LINE__, msqid, sendbuf->mdata, strlen(sendbuf->mdata), sendbuf->mtype);
                }
                else
                {
                    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Package Installation failed for %s with %d \n", __FUNCTION__, __LINE__, eventData->rdm_pkg_info.pkg_name, (IARM_RDMMgr_Status_t)eventData->rdm_pkg_info.pkg_inst_status);
                }
                remove_item(cache);
            }
            else
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Package not requested... %s \n", __FUNCTION__, __LINE__, eventData->rdm_pkg_info.pkg_name);
            }
        }
        else
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Event ID not found for %d, use IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS %d!!! \n", __FUNCTION__, __LINE__, eventId, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Owner Name found %s, use IARM_BUS_RDMMGR_NAME!!! \n", __FUNCTION__, __LINE__, owner);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
}

/*
 * @function _remoteDebuggerEventHandler
 * @brief Receives the RBUS event and sends the value as a message in the message-queue to the thread function.
 * @param rbusHandle_t handle - RBUS handle.
 * @param rbusEvent_t const* event - RBUS event object.
 * @param rbusEventSubscription_t* subscription - RBUS event subscription object.
 * @return void
 */
void _remoteDebuggerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    char *dataMsg = NULL;
    char *inData = (char *)data;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);
    if (strcmp(owner, IARM_BUS_RDK_REMOTE_DEBUGGER_NAME) == 0)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for IARM_BUS_RDK_REMOTE_DEBUGGER_NAME %s \n", __FUNCTION__, __LINE__, owner);
        if (eventId == IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event ID found for IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE %d \n", __FUNCTION__, __LINE__, eventId);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Data from TR69 Parameter for REMOTE_DEBUGGER_ISSUETYPE %s \n", __FUNCTION__, __LINE__, inData);

            dataMsg = (char *) calloc(1, len);
            if(!dataMsg)
            {
                RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Memory Allocation Failed for EventId %d \n",__FUNCTION__,__LINE__,eventId);
                return;
            }
            strncpy(dataMsg, inData, len);
            pushIssueTypesToMsgQueue(dataMsg, IARM_EVENT_MSG);
        }
        else
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Event ID not found for %d, use IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE %d!!! \n", __FUNCTION__, __LINE__, eventId, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Owner Name found %s, use IARM_BUS_RDK_REMOTE_DEBUGGER_NAME!!! \n", __FUNCTION__, __LINE__, owner);
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
}

/*
 * @function _remoteDebuggerWebCfgDataEventHandler
 * @brief Receives the WebCfg data event via RBUS and sends the value as a message in the message-queue to the thread function.
 * @param rbusHandle_t handle - RBUS handle.
 * @param rbusEvent_t const* event - RBUS event object.
 * @param rbusEventSubscription_t* subscription - RBUS event subscription object.
 * @return void
 */
void _remoteDebuggerWebCfgDataEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    char *dataMsg = (char *)data;
    char *inString = NULL;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);
    if (strcmp(owner, IARM_BUS_RDK_REMOTE_DEBUGGER_NAME) == 0)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Received event for IARM_BUS_RDK_REMOTE_DEBUGGER_NAME %s \n", __FUNCTION__, __LINE__, owner);
        if (eventId == IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event ID found for IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA %d \n", __FUNCTION__, __LINE__, eventId);
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Data from TR69 Parameter for REMOTE_DEBUGGER_WEBCFGDATA %s \n", __FUNCTION__, __LINE__, dataMsg);
            inString = (char *)calloc(1, len);
            if(inString)
            {
                strncpy(inString, dataMsg, len);
                pushIssueTypesToMsgQueue(inString, IARM_EVENT_WEBCFG_MSG);
            }
        }
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exit... \n", __FUNCTION__, __LINE__);
}

/*
 * @function pushIssueTypesToMsgQueue
 * @brief Pushes the issue types to the message queue.
 * @param char *issueTypeList - The list of issue types.
 * @param message_type_et sndtype - The message type.
 * @return void
 */
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
        RRDMsgDeliver(msqid, sbuf);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: SUCCESS: Message sending Done, ID=%d MSG=%s Size=%d Type=%u! \n", __FUNCTION__, __LINE__, msqid, sbuf->mdata, strlen(sbuf->mdata), sbuf->mtype);
    }
}

/*
 * @function RRD_unsubscribe
 * @brief Disconnects and terminates the bus, and unregisters event handlers.
 * @param None.
 * @return int - Returns 0 for success, and non-zero for failure.
 */
IARM_Result_t RRD_unsubscribe()
{
    int ret = 0;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Disconnect();
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Disconnect failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Disconnect done!\n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Term();
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Termination failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Term done!\n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM UnRegister EventHandler for RRD failed!!! \n", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_UnRegisterEventHandler for RRD done! \n", __FUNCTION__, __LINE__);

    // IARM unregister RDM Event Handler
    ret = IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM UnRegister EventHandler for RDMMGR failed!!! \n", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }

    // IARM Unregister for Deep Sleep Event Handler

    ret = IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Unregister EventHandler for RDMMGR failed!!! \n ", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_UnRegisterEventHandler for RDMMGR done! \n", __FUNCTION__, __LINE__);


    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
#if GTEST_ENABLE
        return (IARM_Result_t)ret;
#else
        return ret;
#endif
}
