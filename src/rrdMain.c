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

#include "rrdMain.h"
#include "rrdRunCmdThread.h"
#include "rrdJsonParser.h"
#include "rrdDeepSleep.h"
#include "rrdEventProcess.h"
#include "rrdInterface.h"
extern rbusHandle_t rrdRbusHandle;

#if !defined(GTEST_ENABLE)
devicePropertiesData devPropData;
#endif

/*
 * @function RRDEventThreadFunc
 * @brief Infinite thread created from the main function to read messages received from the TR181 parameter 
 *              and perform command execution in a continuous loop.
 * @param void *arg - Argument pointer (not used).
 * @return void* - Returns the argument pointer.
 */
void *RRDEventThreadFunc(void *arg)
{
    data_buf *rbuf;
    msgRRDHdr msgHdr;

    while (1)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]:Waiting for for TR69/RBUS Events... \n", __FUNCTION__, __LINE__);

        if (msgrcv(msqid, (void *)&msgHdr, sizeof(void *), 0, 0) < 0)
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:Message Reception failed for Message Queue Id:[%d]!!! \n", __FUNCTION__, __LINE__, msqid);
            break;
        }
        rbuf = (data_buf *)msgHdr.mbody;
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:SUCCESS: Message Reception Done for ID=%d MSG=%s TYPE=%u... \n", __FUNCTION__, __LINE__, msqid, rbuf->mdata, rbuf->mtype);

        switch (rbuf->mtype)
        {
        case EVENT_MSG:
            processIssueTypeEvent(rbuf);
            break;
        case EVENT_WEBCFG_MSG:
            processWebCfgTypeEvent(rbuf);
            break;
        case DEEPSLEEP_EVENT_MSG:
#ifdef IARMBUS_SUPPORT
            /*Process Deep Sleep Events*/
            RRDProcessDeepSleepAwakeEvents(rbuf);
#endif
            break;
        default:
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Message Type %d!!!\n", __FUNCTION__, __LINE__, rbuf->mtype);
            break;
        }
        free(rbuf);
#if GTEST_ENABLE
        break;
#endif
    }
    
    return arg;
}

/*
 * @function isRRDEnabled
 * @brief Checks if the RRD RFC is enabled. If set to false, logs the message and stops the service.
 * @param None.
 * @return bool - Returns true if RRD is enabled, false if it is disabled.
 */
bool isRRDEnabled(void)
{
    bool ret = false;
    rbusError_t retCode = RBUS_ERROR_BUS_ERROR;
    rbusValue_t value = NULL;
    //rbusValue_Init(&value);

#if !defined(GTEST_ENABLE)
     retCode = rbus_get(rrdRbusHandle, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable", &value);
     RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"RemoteDebugger get value : = [%s]\n", value);
     RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"RemoteDebugger get value Status: = [%s]\n", retCode);
     if ((retCode == RBUS_ERROR_SUCCESS) && (value != NULL) ) 
     {
          RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"RemoteDebugger Status: = [%d]\n", rbusValue_GetBoolean(value));
          ret = rbusValue_GetBoolean(value);
          if(ret) 
	  {
               RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]:RFC is enabled, starting remote-debugger\n", __FUNCTION__, __LINE__);
          }
          else 
	  { 
               RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]:RFC is disabled, stopping remote-debugger\n", __FUNCTION__, __LINE__);
          }
     }
     else 
     {
          RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: rbus_get failed with error [%d]\n\n", __FUNCTION__, __LINE__,rbusError_ToString((rbusError_t)ret));
     }
#endif
    return ret;
}

/*
 * @function main
 * @brief Main function used to perform IARM Bus communication, initialize and exit control,
 *              and read data from the TR181 parameter.
 * @param int argc - Argument count (only for non-GTEST builds).
 *        char *argv[] - Argument vector (only for non-GTEST builds).
 * @return int - Returns 0 on success.
 */
#if GTEST_ENABLE
int shadowMain(void *arg)
#else
int main(int argc, char *argv[])
#endif
{
    pthread_t RRDTR69ThreadID;

    rdk_logger_init(DEBUG_INI_FILE);
#if !defined(GTEST_ENABLE)
    /* Store Device Info.*/
    RRDStoreDeviceInfo(&devPropData);
#endif
    /* Initialize Cache */
    initCache();

    /* Check RRD Enable RFC */
    bool isEnabled = isRRDEnabled();
    if(!isEnabled) {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:RFC is disabled, stopping remote-debugger\n", __FUNCTION__, __LINE__);
        exit(0);
    }
    
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Starting RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);
    if ((msqid = msgget(key, IPC_CREAT | 0666 )) < 0)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]:Message Queue ID Creation failed, msqid=%d!!!\n",__FUNCTION__,__LINE__,msqid);
        exit(1);
    }
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:SUCCESS: Message Queue ID Creation Done, msqid=%d...\n",__FUNCTION__,__LINE__,msqid);

    RRD_subscribe();
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Started RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);

    /* Create Thread for listening TR69 events */
    pthread_create (&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
    pthread_join(RRDTR69ThreadID, NULL);

    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Stopping RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);
    RRD_unsubscribe();
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Stopped RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);

    return 0;
}
