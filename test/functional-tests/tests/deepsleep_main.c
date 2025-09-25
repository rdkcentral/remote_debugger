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
#include "rrdDynamic.h"
#include "rrdEventProcess.h"
#include "rrdInterface.h"

#include "power_controller.h"
#include "rrdInterface.h"
#include "rbus.h"


devicePropertiesData devPropData;

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
            /*Process Deep Sleep Events*/
            RRDProcessDeepSleepAwakeEvents(rbuf);
            break;
        default:
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Message Type %d!!!\n", __FUNCTION__, __LINE__, rbuf->mtype);
            free(rbuf->mdata);
            free(rbuf);
            break;
        }
    }

    return arg;
}

bool isRRDEnabled(void)
{
    bool ret = true;
    RFC_ParamData_t param;
    WDMP_STATUS status = getRFCParameter("RDKRemoteDebugger", RRD_RFC, &param);
    if(status == WDMP_SUCCESS || status == WDMP_ERR_DEFAULT_VALUE) {
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:getRFCParameter() name=%s,type=%d,value=%s\n", __FUNCTION__, __LINE__, param.name, param.type, param.value);
            if (strcasecmp("false", param.value) == 0) {
                    ret = false;
            }
    }
    else {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:ERROR in getRFCParameter()\n", __FUNCTION__, __LINE__);
    }

    return ret;
}

int main(int argc, char *argv[])

{
    pthread_t RRDTR69ThreadID;

    rdk_logger_init(DEBUG_INI_FILE);

    /* Store Device Info.*/
    RRDStoreDeviceInfo(&devPropData);

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

    IARM_Bus_RDMMgr_EventData_t eventData;
    strcpy(eventData.rdm_pkg_info.pkg_name, "RDK-RRD-DEEPSLEEP");
    strcpy(eventData.rdm_pkg_info.pkg_inst_path, "/media/apps/RDK-RRD-DEEPSLEEP");
    eventData.rdm_pkg_info.pkg_inst_status = RDM_PKG_INSTALL_COMPLETE;

    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS;
    size_t len = sizeof(IARM_Bus_RDMMgr_EventData_t);
    pthread_create(&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
    // Give the thread a moment to start (optional but helpful)
    sleep(1);
    PowerController_PowerState_t state1 = POWER_STATE_STANDBY_DEEP_SLEEP;
    PowerController_PowerState_t state2 = POWER_STATE_STANDBY_DEEP_SLEEP;
    void* userdata = NULL;
    _pwrManagerEventHandler(state1, state2, userdata);
    state2 = POWER_STATE_ON;
    _pwrManagerEventHandler(state1, state2, userdata);
    sleep(2);
    _rdmManagerEventHandler(owner, eventId, &eventData, len);
    // Now wait for the thread to finish (if it ever does)
    pthread_join(RRDTR69ThreadID, NULL);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Stopping RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);
    RRD_unsubscribe();
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:Stopped RDK Remote Debugger Daemon \n",__FUNCTION__,__LINE__);

    return 0;
}

uint32_t PowerController_RegisterPowerModeChangedCallback(PowerController_PowerModeChangedCb callback, void* userdata)
{
    return POWER_CONTROLLER_ERROR_NONE;
}
uint32_t PowerController_Connect()
{
    return POWER_CONTROLLER_ERROR_NONE;
}
uint32_t PowerController_UnRegisterPowerModeChangedCallback(PowerController_PowerModeChangedCb callback)
{
    return POWER_CONTROLLER_ERROR_NONE;
}
void PowerController_Term()
{

}
void PowerController_Init()
{

}
