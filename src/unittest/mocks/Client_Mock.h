/*
 * Copyright 2023 Comcast Cable Communications Management, LLC
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
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
typedef enum _RemoteDebugger_EventId_t {
        IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE = 0,
        IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA,
        IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT
} IARM_Bus_RemoteDebugger_EventId_t;
//#ifdef IARMBUS_SUPPORT
/* ----------------- RDMMgr ---------- */
#define IARM_BUS_RDMMGR_NAME "RDMMgr"
#define RDM_PKG_NAME_MAX_SIZE 128
#define RDM_PKG_VERSION_MAX_SIZE 8
#define RDM_PKG_INST_PATH_MAX_SIZE 256

typedef enum _RDMMgr_EventId_t
{
    IARM_BUS_RDMMGR_EVENT_APPDOWNLOADS_CHANGED = 0,
    IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS,
    IARM_BUS_RDMMGR_EVENT_MAX
} IARM_Bus_RDMMgr_EventId_t;

typedef enum _RDMMgr_Status_t
{
    RDM_PKG_INSTALL_COMPLETE = 0,
    RDM_PKG_INSTALL_ERROR,
    RDM_PKG_DOWNLOAD_COMPLETE,
    RDM_PKG_DOWNLOAD_ERROR,
    RDM_PKG_EXTRACT_COMPLETE,
    RDM_PKG_EXTRACT_ERROR,
    RDM_PKG_VALIDATE_COMPLETE,
    RDM_PKG_VALIDATE_ERROR,
    RDM_PKG_POSTINSTALL_COMPLETE,
    RDM_PKG_POSTINSTALL_ERROR,
    RDM_PKG_UNINSTALL,
    RDM_PKG_INVALID_INPUT
} IARM_RDMMgr_Status_t;

typedef struct _RDMMgr_EventData_t
{
    struct _pkg_info
    {
        char pkg_name[RDM_PKG_NAME_MAX_SIZE];
        char pkg_version[RDM_PKG_VERSION_MAX_SIZE];
        char pkg_inst_path[RDM_PKG_INST_PATH_MAX_SIZE];
        IARM_RDMMgr_Status_t pkg_inst_status;
    } rdm_pkg_info;
} IARM_Bus_RDMMgr_EventData_t;

/* ----------------- PwrMgr ---------- */
#define IARM_BUS_PWRMGR_NAME "PWRMgr"

typedef enum _IARM_Bus_Daemon_PowerState_t
{
    IARM_BUS_PWRMGR_POWERSTATE_OFF,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY,
    IARM_BUS_PWRMGR_POWERSTATE_ON,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP,
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP
} IARM_Bus_PowerState_t;

typedef IARM_Bus_PowerState_t IARM_Bus_PWRMgr_PowerState_t;

typedef enum _PWRMgr_EventId_t
{
    IARM_BUS_PWRMGR_EVENT_MODECHANGED = 0,
    IARM_BUS_PWRMGR_EVENT_DEEPSLEEP_TIMEOUT,
    IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE,
    IARM_BUS_PWRMGR_EVENT_REBOOTING,
    IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED,
    IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED,
    IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED,
    IARM_BUS_PWRMGR_EVENT_MAX,
} IARM_Bus_PWRMgr_EventId_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerState_Param_t
{
    IARM_Bus_PWRMgr_PowerState_t curState;
    IARM_Bus_PWRMgr_PowerState_t prevState;
} IARM_Bus_PWRMgr_GetPowerState_Param_t;

typedef struct _PWRMgr_EventData_t
{
    union
    {
        struct _MODE_DATA
        {
            IARM_Bus_PWRMgr_PowerState_t curState;
            IARM_Bus_PWRMgr_PowerState_t newState;
#ifdef ENABLE_DEEP_SLEEP
            uint32_t deep_sleep_timeout;
#endif
            bool nwStandbyMode;
        } state;
#ifdef ENABLE_THERMAL_PROTECTION
        struct _THERM_DATA
        {
            IARM_Bus_PWRMgr_ThermalState_t curLevel;
            IARM_Bus_PWRMgr_ThermalState_t newLevel;
            float curTemperature;
        } therm;
#endif
        bool bNetworkStandbyMode;
        int32_t reset_sequence_progress;
    } data;
} IARM_Bus_PWRMgr_EventData_t;
//#endif

/* ---------------- WebConf ------------*/
#define SUBDOC_NAME_SZ 64
#define SYSCFG_FAILURE 601
#define BLOB_EXEC_SUCCESS 300

typedef struct s_Error_
{
    uint16_t ErrorCode;
    char ErrorMsg[128];
} Err, *pErr;

typedef size_t (*calcTimeout)(size_t);

typedef pErr (*executeBlobRequest)(void *);

typedef int (*rollbackFunc)();

typedef void (*freeResources)(void *);

typedef struct _execData
{
    char subdoc_name[SUBDOC_NAME_SZ];
    uint16_t txid;
    uint32_t version;
    size_t numOfEntries;
    void *user_data;
    size_t (*calcTimeout)(size_t);
    pErr (*executeBlobRequest)(void *);
    int (*rollbackFunc)();
    void (*freeResources)(void *);
    int multiCompRequest;
} execData;

typedef uint32_t (*getVersion)(char *);
typedef int (*setVersion)(char *, uint32_t);
typedef struct _blobRegInfo
{
    int version;
    char subdoc_name[SUBDOC_NAME_SZ];
} blobRegInfo, *PblobRegInfo;

/* --------- RDK Logger MACROS ------------- */
#define RDK_LOG(level, module, format, ...) printf("[%s:%s]" format, #level, #module, __VA_ARGS__)

/* --------- RFC MACROS ------------- */
typedef enum _tr181ErrorCodes
{
    tr181Success = 0,
    tr181Failure,
    tr181InternalError = 9,
} tr181ErrorCode_t;

typedef enum
{
    WDMP_SUCCESS = 0,
    WDMP_FAILURE,
} WDMP_STATUS;

tr181ErrorCode_t getErrorCode(WDMP_STATUS status)
{
    switch (status)
    {
    case WDMP_SUCCESS:
        return tr181Success;
    case WDMP_FAILURE:
        return tr181Failure;
    default:
        return tr181InternalError;
    }
}

/* --------- IARM MACROS ------------- */
typedef int IARM_EventId_t;

typedef enum _IARM_Result_t
{
    IARM_RESULT_SUCCESS,
    IARM_RESULT_INVALID_PARAM,
    IARM_RESULT_FAILURE,
    IARM_RESULT_INVALID_STATE,
    IARM_RESULT_IPCCORE_FAIL,
    IARM_RESULT_OOM,
} IARM_Result_t;

#define IARM_BUS_DAEMON_NAME "Daemon"

typedef IARM_Result_t (*IARM_BusCall_t)(void *arg);
typedef void (*IARM_EventHandler_t)(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

/* --------- RBUS MACROS ------------*/
typedef enum _rbusError
{
    RBUS_ERROR_SUCCESS,
    RBUS_ERROR_NOT_INITIALIZED,
    RBUS_ERROR_BUS_ERROR,
} rbusError_t;

char const * rbusError_ToString(rbusError_t e);

struct _rbusHandle
{
};

typedef struct _rbusHandle *rbusHandle_t;

struct _rbusObject
{
};
typedef struct _rbusObject *rbusObject_t;
typedef enum
{
    RBUS_EVENT_OBJECT_CREATED,   
    RBUS_EVENT_OBJECT_DELETED,   
    RBUS_EVENT_VALUE_CHANGED,    
    RBUS_EVENT_GENERAL,          
    RBUS_EVENT_INITIAL_VALUE,    
    RBUS_EVENT_INTERVAL,         
    RBUS_EVENT_DURATION_COMPLETE 
} rbusEventType_t;

typedef struct
{
    char const*     name;       
    rbusEventType_t type;       
    rbusObject_t    data;       
}   rbusEvent_t;

typedef struct _rbusEventSubscription
{
    char const*         eventName;    
    uint32_t             interval;   
    uint32_t            duration;   
    void*               handler;    
    void*               userData;   
    rbusHandle_t        handle;     
    rbusSubscribeAsyncRespHandler_t asyncHandler;
    bool                publishOnSubscribe;
} rbusEventSubscription_t;

typedef struct _rbusEventSubscription rbusEventSubscription_t;


struct _rbusValue
{
};
typedef struct _rbusValue *rbusValue_t;

typedef void (*rbusMethodAsyncRespHandler_t)(rbusHandle_t handle, char const *methodName, rbusError_t error, rbusObject_t params);

/* =============== Implementations ============== */
/* ---------- IARM Impl -----------*/

class ClientIARMMock
{
public:
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Init, (const char *));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Connect, ());
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Disconnect, ());
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Term, ());
    MOCK_METHOD(IARM_Result_t, IARM_Bus_RegisterEventHandler, (const char *, IARM_EventId_t, IARM_EventHandler_t));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_UnRegisterEventHandler, (const char *, IARM_EventId_t));
};

void setMock(ClientIARMMock *mock);

/* ------------------- RBUS Impl--------------- */
class RBusApiInterface
{
public:
    virtual ~RBusApiInterface() = default;
    virtual rbusError_t rbus_open(rbusHandle_t *handle, char const *componentName) = 0;
    virtual rbusError_t rbus_close(rbusHandle_t handle) = 0;
    virtual rbusError_t rbusValue_Init(rbusValue_t *value) = 0;
    virtual rbusError_t rbusValue_SetString(rbusValue_t value, char const *str) = 0;
    virtual rbusError_t rbus_set(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler) = 0;
    virtual rbusError_t rbus_get(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler) = 0;
};

class RBusApiWrapper
{
protected:
    static RBusApiInterface *impl;

public:
    RBusApiWrapper();
    static void setImpl(RBusApiInterface *newImpl);
    static void clearImpl();
    static rbusError_t rbus_open(rbusHandle_t *handle, char const *componentName);
    static rbusError_t rbus_close(rbusHandle_t handle);
    static rbusError_t rbusValue_Init(rbusValue_t *value);
    static rbusError_t rbusValue_SetString(rbusValue_t value, char const *str);
    static rbusError_t rbus_set(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler);
    static rbusError_t rbus_get(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler);
};

extern rbusError_t (*rbus_open)(rbusHandle_t *, char const *);
extern rbusError_t (*rbus_close)(rbusHandle_t);
extern rbusError_t (*rbusValue_Init)(rbusValue_t *);
extern rbusError_t (*rbusValue_SetString)(rbusValue_t, char const *);
extern rbusError_t (*rbus_set)(rbusHandle_t, char const *, rbusValue_t, rbusMethodAsyncRespHandler_t);
extern rbusError_t (*rbus_get)(rbusHandle_t, char const *, rbusValue_t, rbusMethodAsyncRespHandler_t);

class MockRBusApi : public RBusApiInterface
{
public:
    MOCK_METHOD2(rbus_open, rbusError_t(rbusHandle_t *, char const *));
    MOCK_METHOD1(rbus_close, rbusError_t(rbusHandle_t));
    MOCK_METHOD1(rbusValue_Init, rbusError_t(rbusValue_t *));
    MOCK_METHOD2(rbusValue_SetString, rbusError_t(rbusValue_t, char const *));
    MOCK_METHOD4(rbus_set, rbusError_t(rbusHandle_t, char const *, rbusValue_t, rbusMethodAsyncRespHandler_t));  
    MOCK_METHOD4(rbus_get, rbusError_t(rbusHandle_t, char const *, rbusValue_t, rbusMethodAsyncRespHandler_t));
};

/* ------------------- WebConfig Impl ------------ */
class ClientWebConfigMock
{
public:
    MOCK_METHOD4(register_sub_docs_mock, void(blobRegInfo *, int, getVersion, setVersion));
};

/* ------------------- RFC Impl--------------- */
class SetParamInterface
{
public:
    virtual ~SetParamInterface() = default;
    virtual tr181ErrorCode_t setParam(char *, const char *, const char *) = 0;
    virtual int v_secure_system(const char *str ) = 0;
};

class SetParamWrapper
{
protected:
    static SetParamInterface *impl;

public:
    SetParamWrapper();
    static void setImpl(SetParamInterface *newImpl);
    static void clearImpl();
    static tr181ErrorCode_t setParam(char *, const char *, const char *);
    static int v_secure_system(const char *str );
};

extern tr181ErrorCode_t (*setParam)(char *, const char *, const char *);

class MockSetParam : public SetParamInterface
{
public:
    MOCK_METHOD3(setParam, tr181ErrorCode_t(char *, const char *, const char *));
    // MOCK_METHOD3(getParam, tr181ErrorCode_t(char*, const char*, char**));
    MOCK_METHOD(int, v_secure_system, (const char *str ), ());
};

/*---------------v_secure calls---------------------*/
class secureInterface
{
public:
    virtual int v_secure_system(const char *command, void *param ) = 0;
    virtual FILE* v_secure_popen(const char *mode,const char *cmd, const char *opt) = 0;
    virtual int v_secure_pclose(FILE *fp) = 0;
};

class MockSecure : public secureInterface
{
public:
    MOCK_METHOD(int, v_secure_system, (const char *command, void *param), ());
    MOCK_METHOD(FILE*, v_secure_popen, (const char *mode, const char *cmd, const char *opt ), ());
    MOCK_METHOD(int, v_secure_pclose, (FILE *fp), ());
};

/* ----------Base64 and Blob ----------- */
class MockBase64
{
public:
    MOCK_METHOD(bool, b64_decode, (const uint8_t *input, size_t input_len, uint8_t *output), ());
    MOCK_METHOD(int, b64_get_decoded_buffer_size, (size_t str_len), ());
    MOCK_METHOD(void, PushBlobRequest, (execData * execDataLan), ());
    MOCK_METHOD(void, rdk_logger_init, (char* testStr), ());
};


/*
#define POWER_CONTROLLER_ERROR_NONE 0
typedef enum PowerController_PowerState {
    POWER_STATE_UNKNOWN = 0 ,
    POWER_STATE_OFF = 1 ,
    POWER_STATE_STANDBY = 2 ,
    POWER_STATE_ON = 3 ,
POWER_STATE_STANDBY_LIGHT_SLEEP = 4 ,
      POWER_STATE_STANDBY_DEEP_SLEEP = 5
} PowerController_PowerState_t;


typedef void (*PowerController_PowerModeChangedCb)(const PowerController_PowerState_t currentState, const PowerController_PowerState_t newState, void* userdata);
typedef void (*PowerController_RebootBeginCb)(const char* rebootReasonCustom, const char* rebootReasonOther, const char* rebootRequestor, void* userdata);
uint32_t PowerController_Connect();
uint32_t PowerController_UnRegisterRebootBeginCallback(PowerController_RebootBeginCb callback);
*/
