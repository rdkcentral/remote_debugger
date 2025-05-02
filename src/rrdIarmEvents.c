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

#include "rrdInterface.h"
#include "rrdRunCmdThread.h" 
#if defined(PWRMGR_PLUGIN)
#include "power_controller.h"
#include <pthread.h>
#endif

extern int msqid;
extern rbusHandle_t rrdRbusHandle;

#if defined(PWRMGR_PLUGIN)
#define RETRYSLEEP (300 * 1000)//Retry sleep time 

static uint8_t isPwrCtlInterface = 0;

static void* getPwrCtrlInterface(void *arg)
{
    int ret;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Entry  \r\n \n", __FUNCTION__, __LINE__);

    /*TODO: remove this sleep after fix METROL-1045*/
    sleep(5);//added sleep wait for the WPEframework active. 
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: start PowerController_Init().. \n", __FUNCTION__, __LINE__);
    PowerController_Init();
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: completed PowerController_Init().. \n", __FUNCTION__, __LINE__);
    while(1)
    {
        if(POWER_CONTROLLER_ERROR_NONE == PowerController_Connect())
        {
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: PowerController_Connect is Success \r\n", __FUNCTION__, __LINE__);
            isPwrCtlInterface = 1;
            break;
        }
        else
        {
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%s] Retry PowerController_Connect after %d usec..\n", __FUNCTION__, __FILE__, RETRYSLEEP);
            usleep(RETRYSLEEP); //retry after RETRYSLEEP milli seconds.
        }
    }
    
    ret = PowerController_RegisterPowerModeChangedCallback(_pwrManagerEventHandler, NULL);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Register power mode change callback EventHandler for RDMMGR failed!!! ret = %d\n ", __FUNCTION__, __LINE__, ret);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Registered power mode change callback.. \n", __FUNCTION__, __LINE__);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Exit  \r\n \n", __FUNCTION__, __LINE__);
    pthread_exit(NULL);
}
#endif

/*
 * @function RRD_IARM_subscribe
 * @brief Initializes and connects to IARMBus, and registers event handlers for receiving IARM events
 *              from the TR181 parameter.
 * @param None.
 * @return int - Returns 0 for success, and non-0 for failure.
 */

int RRD_IARM_subscribe()
{
    int ret = 0;
#if defined(PWRMGR_PLUGIN)
    pthread_t pwrConnectThreadID;
#endif

    ret = IARM_Bus_Init(RDK_REMOTE_DEBUGGER_NAME);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Init failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Init done! \n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Connect();
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Connect failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Connect done! \n", __FUNCTION__, __LINE__);

    // IARM Reg for RDM Event Handler
    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS, _rdmManagerEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RDMMGR failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
#if defined(PWRMGR_PLUGIN)
    // Thunder client library register for Deep Sleep Event Handler
    if(pthread_create (&pwrConnectThreadID, NULL, getPwrCtrlInterface, NULL)  == 0)
    {
        if(pthread_detach(pwrConnectThreadID) != 0) 
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: getPwrCtrlInterface Thread detach Failed\r\n ", __FUNCTION__, __LINE__);
        }
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: getPwrCtrlInterface Thread started to get power manager COM-RPC.. \n", __FUNCTION__, __LINE__);
#else
    // IARM Reg for Deep Sleep Event Handler
    ret = IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _pwrManagerEventHandler);
    if (ret != IARM_RESULT_SUCCESS)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Register EventHandler for RDMMGR failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
#endif
    return ret;
}


#if defined(PWRMGR_PLUGIN)
/*
 * @function  _pwrManagerEventHandler
 * @brief Receives the power manager event and sends the value as a message in the message-queue
 *              to the thread function.
 * @param
 *        const PowerController_PowerState_t currentState - current power state.
 *        const PowerController_PowerState_t newState - New power state.
 *        void* userdata - user event data.
 * Output: void
 */
void _pwrManagerEventHandler(const PowerController_PowerState_t currentState,
    const PowerController_PowerState_t newState, void* userdata)
{
#if !defined(ENABLE_WEBCFG_FEATURE)
    data_buf *sbuf = NULL;
    int msgLen = strlen(DEEP_SLEEP_STR) + 1;
#endif
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. currentState =%d, newState = %d\n", __FUNCTION__, __LINE__, currentState, newState);

    if ((currentState == POWER_STATE_STANDBY_DEEP_SLEEP &&
            newState != POWER_STATE_STANDBY_DEEP_SLEEP))
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Received state from Power Manager Current :[%d] New[%d] \n", __FUNCTION__, __LINE__, currentState, newState);
#ifdef ENABLE_WEBCFG_FEATURE
        rbusError_t rc = RBUS_ERROR_BUS_ERROR;
        rbusValue_t value;
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

        RRD_data_buff_init(sbuf, DEEPSLEEP_EVENT_MSG, RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE);
        sbuf->mdata = (char *)malloc(msgLen);
        if (!sbuf->mdata)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
            RRD_data_buff_deAlloc(sbuf);
            return;
        }
        strncpy((char *)sbuf->mdata, (const char *)DEEP_SLEEP_STR, msgLen);
        RRDMsgDeliver(msqid, sbuf);
#endif
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event Triggred for DeepSleep\n", __FUNCTION__, __LINE__);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exit.. \n", __FUNCTION__, __LINE__);
}
#else
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
//#if !defined(ENABLE_WEBCFG_FEATURE)
    data_buf *sbuf = NULL;
    int msgLen = strlen(DEEP_SLEEP_STR) + 1;
//#endif
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
//#ifdef ENABLE_WEBCFG_FEATURE
            rbusError_t rc = RBUS_ERROR_BUS_ERROR;
            rbusValue_t value;
	    rbusValue_Init(&value);
            rbusValue_SetString(value,"root");
            rc = rbus_set(rrdRbusHandle, RRD_WEBCFG_FORCE_SYNC, value, NULL);
            if (rc != RBUS_ERROR_SUCCESS)
            {
                    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: rbus_set failed for [%s] with error [%d]\n\n", __FUNCTION__, __LINE__,RRD_WEBCFG_FORCE_SYNC ,rc);
            return;
            }
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Invoking WebCfg Force Sync: %s... \n", __FUNCTION__, __LINE__, RRD_WEBCFG_FORCE_SYNC);
//#else
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Copying Message Received to the queue.. \n", __FUNCTION__, __LINE__);
            sbuf = (data_buf *)malloc(sizeof(data_buf));
            if (!sbuf)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                return;
            }

            RRD_data_buff_init(sbuf, DEEPSLEEP_EVENT_MSG, RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE);
            sbuf->mdata = (char *)malloc(msgLen);
            if (!sbuf->mdata)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId %d \n", __FUNCTION__, __LINE__, eventId);
                RRD_data_buff_deAlloc(sbuf);
                return;
            }
            strncpy((char *)sbuf->mdata, (const char *)DEEP_SLEEP_STR, msgLen);
            RRDMsgDeliver(msqid, sbuf);
//#endif
        }
        else
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Event Triggred for DeepSleep %d \n", __FUNCTION__, __LINE__, eventId);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Owner Name found %s, use RDK_REMOTE_DEBUGGER_NAME!!! \n", __FUNCTION__, __LINE__, owner);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exit.. \n", __FUNCTION__, __LINE__);
}
#endif
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
                        RRD_data_buff_init(sendbuf, DEEPSLEEP_EVENT_MSG, RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE);
                    }
                    else
                    {
                        RRD_data_buff_init(sendbuf, EVENT_MSG, RRD_DEEPSLEEP_INVALID_DEFAULT);
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
 * @function RRD_IARM_unsubscribe
 * @brief Disconnects and terminates the bus, and unregisters event handlers.
 * @param None.
 * @return int - Returns 0 for success, and non-zero for failure.
 */
int RRD_IARM_unsubscribe()
{
    int ret = 0;

    ret = IARM_Bus_Disconnect();
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Disconnect failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Disconnect done!\n", __FUNCTION__, __LINE__);

    ret = IARM_Bus_Term();
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Termination failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_Term done!\n", __FUNCTION__, __LINE__);

    // IARM unregister RDM Event Handler
    ret = IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM UnRegister EventHandler for RDMMGR failed!!! \n", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_UnRegisterEventHandler for RDMMGR done! \n", __FUNCTION__, __LINE__);
#if defined(PWRMGR_PLUGIN)
    if (isPwrCtlInterface)
    {
        // Thunder client Unregister for Deep Sleep Event Handler
        ret = PowerController_UnRegisterPowerModeChangedCallback(_pwrManagerEventHandler);
        if (ret != 0)
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: UnRegistering power mode change EventHandler for PWRMGR-plugin failed!!! \n ", __FUNCTION__, __LINE__);
        }
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: UnRegistering power mode change EventHandler for PWRMGR-plugin done! \n", __FUNCTION__, __LINE__);
        PowerController_Term();
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: completed PowerController_Term().. \n", __FUNCTION__, __LINE__);
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Failed:  isPwrCtlInterface is %d\n", __FUNCTION__, __LINE__, isPwrCtlInterface);
    }
#else
    // IARM Unregister for Deep Sleep Event Handler
    ret = IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED);
    if (ret != 0)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: IARM Unregister EventHandler for PWRMGR failed!!! \n ", __FUNCTION__, __LINE__);
        return ret;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: SUCCESS: IARM_Bus_UnRegisterEventHandler for PWRMGR done! \n", __FUNCTION__, __LINE__);
#endif
    return ret;
}
