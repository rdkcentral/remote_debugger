/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
 * @file pwrMgr.h
 *
 * @brief IARM-Bus Power Manager Public API.
 *
 * This API defines the structures and functions for the IARM-Bus Power Manager interface.
 */

/**
* @defgroup iarmmgrs
* @{
* @defgroup hal
* @{
**/

#ifndef _IARM_BUS_PWRMGR_H
#define _IARM_BUS_PWRMGR_H

#include "libIARM.h"
#include "libIBusDaemon.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" 
{
#endif

/**
 * @addtogroup IARM_PLAT_POWER_API
 * @{
 */

#define IARM_BUS_PWRMGR_NAME 						"PWRMgr"  /*!< Power manager IARM bus name */

/**
 * @brief Represents the published Events from PWR  Manager
 */
typedef enum _PWRMgr_EventId_t {
    IARM_BUS_PWRMGR_EVENT_MODECHANGED = 0,         /*!< Event to notify power mode change */
    IARM_BUS_PWRMGR_EVENT_DEEPSLEEP_TIMEOUT, /*!< Event to notify deepsleep timeout */
    IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE,        /*!< Event to notify progress of reset key sequence*/
    IARM_BUS_PWRMGR_EVENT_REBOOTING,             /*!< Event to notify that the device is about to reboot.*/
    IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED,        /*!< Event to notify temperature level change */
    IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED,        /*!< Event to notify ware house operation status changed */
    IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED,        /*!< Event to notify that the network standby mode changed.*/
    IARM_BUS_PWRMGR_EVENT_MAX,                     /*!< Max event id from this module */
} IARM_Bus_PWRMgr_EventId_t;


/**
 *  @brief Enumerator which represents the  possible temeperature states
 */
typedef enum _IARM_Bus_PWRMgr_ThermalState_t{
    IARM_BUS_PWRMGR_TEMPERATURE_NORMAL = 0,    /* Temp is within normal operating range */
    IARM_BUS_PWRMGR_TEMPERATURE_HIGH,          /* Temp is high, but just a warning as device can still operate */
    IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL       /* Temp is critical, should trigger a thermal reset */
} IARM_Bus_PWRMgr_ThermalState_t;

/**
 *  @brief Enumerator which represents the  possible warehouse ops
 */
typedef enum _IARM_Bus_PWRMgr_WareHouseOps_t{
    IARM_BUS_PWRMGR_WAREHOUSE_RESET = 0,    /* warehouse reset */
    IARM_BUS_PWRMGR_WAREHOUSE_CLEAR,          /* warehouse clear*/
} IARM_Bus_PWRMgr_WareHouseOps_t;

/**
 *  @brief Enumerator which represents the  possible warehouse ops
 */
typedef enum _IARM_Bus_PWRMgr_WareHouseOpsStatus_t{
    IARM_BUS_PWRMGR_WAREHOUSE_COMPLETED = 0,    /* warehouse operation completed sucessfully */
    IARM_BUS_PWRMGR_WAREHOUSE_INPROGRESS,          /* warehouse operation in progress*/
    IARM_BUS_PWRMGR_WAREHOUSE_FAILED,          /* warehouse operation failed.*/
} IARM_Bus_PWRMgr_WareHouseOpsStatus_t;

/**
 *  @brief Enumerator which represents the  possible wakeup sources
 */
typedef enum _WakeupSrcType_t {
    WAKEUPSRC_VOICE = 0,
    WAKEUPSRC_PRESENCE_DETECTION,
    WAKEUPSRC_BLUETOOTH,
    WAKEUPSRC_WIFI,
    WAKEUPSRC_IR,
    WAKEUPSRC_POWER_KEY,
    WAKEUPSRC_TIMER,
    WAKEUPSRC_CEC,
    WAKEUPSRC_LAN,
    WAKEUPSRC_MAX
} WakeupSrcType_t;

/**
 *  @brief Structure which holds the event data.
 */
typedef struct _PWRMgr_EventData_t {
    union {
        struct _MODE_DATA{
        	/* Declare Event Data structure for PWRMGR_EVENT_DUMMY0 */
        	IARM_Bus_PowerState_t curState;	/*!< Power manager current power state */
        	IARM_Bus_PowerState_t newState;	/*!< Power manager new power state */
            #ifdef ENABLE_DEEP_SLEEP
                uint32_t deep_sleep_timeout;    
            #endif
            bool nwStandbyMode;
	} state; 
	#ifdef ENABLE_THERMAL_PROTECTION
	struct _THERM_DATA{
	    	IARM_Bus_PWRMgr_ThermalState_t curLevel;
		IARM_Bus_PWRMgr_ThermalState_t newLevel;
		float curTemperature;
        } therm;
	#endif
        bool bNetworkStandbyMode;
        int32_t reset_sequence_progress;
     } data;
}IARM_Bus_PWRMgr_EventData_t;

/**
 *  @brief Structure which holds the Deep sleep timeout value.
 */
typedef struct _IARM_BUS_PWRMgr_DeepSleepTimeout_EventData_t {
	unsigned int timeout;        /*!< Timeout for deep sleep in seconds*/ 
} IARM_BUS_PWRMgr_DeepSleepTimeout_EventData_t;

/**
 *  @brief Structure which holds warehouse opn status value.
 */
typedef struct _IARM_BUS_PWRMgr_WareHouseOpn_EventData_t {
	IARM_Bus_PWRMgr_WareHouseOps_t wareHouseOpn;        /*!< WareHouse operation*/
	IARM_Bus_PWRMgr_WareHouseOpsStatus_t status;        /*!< WareHouse operation status*/
} IARM_BUS_PWRMgr_WareHouseOpn_EventData_t;

/**
 * Declare RPC API names and their arguments
 */
#define IARM_BUS_PWRMGR_API_SetPowerState 		"SetPowerState" /*!< Sets the powerstate of the device*/

/**
 *  @brief Structure which holds the new powerstate to be set to the device.
 */
typedef struct _IARM_Bus_PWRMgr_SetPowerState_Param_t {
	IARM_Bus_PowerState_t newState;        /*!< [in] New powerstate to be set */
        int keyCode;                                  /*!< [in] Key code for the last key Pressed */
} IARM_Bus_PWRMgr_SetPowerState_Param_t;

#define IARM_BUS_PWRMGR_API_GetPowerState 		"GetPowerState" /*!< Retrives current  power state of the box*/

/**
 *  @brief Structure which holds the current power state of the CPE.
 */
typedef struct _IARM_Bus_PWRMgr_GetPowerState_Param_t {
	IARM_Bus_PowerState_t curState;        /*!< Current powerstate of the box*/ 
	IARM_Bus_PowerState_t prevState;        /*!< Current powerstate of the box*/ 
} IARM_Bus_PWRMgr_GetPowerState_Param_t;

#define IARM_BUS_PWRMGR_API_WareHouseReset		"WareHouseReset" /*!< Reset the box to warehouse state*/

/**
 *  @brief Structure which holds the ware house reset time.
 */
typedef struct _IARM_Bus_PWRMgr_WareHouseReset_Param_t {
	bool suppressReboot; /*!< STB should not be rebooted */
} IARM_Bus_PWRMgr_WareHouseReset_Param_t;

#define IARM_BUS_PWRMGR_API_WareHouseClear "WarehouseClear" /*!< */

#define IARM_BUS_PWRMGR_API_ColdFactoryReset "ColdFactoryReset" /*!< Reset the box to cold factory state*/
#define IARM_BUS_PWRMGR_API_FactoryReset "FactoryReset" /*!< Reset the box to factory state*/
#define IARM_BUS_PWRMGR_API_UserFactoryReset "UserFactoryReset" /*!< Reset the box to user factory state*/

#define IARM_BUS_PWRMGR_API_SetDeepSleepTimeOut		"SetDeepSleepTimeOut" /*!< Sets the timeout for deep sleep*/

/**
 *  @brief Structure which holds the timeout value to set for Deep sleep.
 */
typedef struct _IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t {
	unsigned int timeout;        /*!< Timeout for deep sleep in seconds*/ 
} IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t;

#define IARM_BUS_PWRMGR_API_SetSleepTimer      		"SetSleepTimer" /*!< Sets sleep timer state and timeout*/
#define IARM_BUS_PWRMGR_API_GetSleepTimer      		"GetSleepTimer" /*!< Gets sleep timer state and remaining */

/**
 *  @brief Structure which holds the sleep timer information.
 */
typedef struct _IARM_Bus_PWRMgr_SleepTimer_Param_t {
	double time;        /*!< timer duration*/ 
	int    start;       /*!< timer state, started=1 or stopped=0*/ 
} IARM_Bus_PWRMgr_SleepTimer_Param_t;

#ifdef ENABLE_THERMAL_PROTECTION

/**
 *  @brief Structure which holds the data associated with thermal level.
 */
typedef struct _IARM_Bus_PWRMgr_GetThermalState_Param_t{
     IARM_Bus_PWRMgr_ThermalState_t curLevel;                     /*!<  Current Thermal level */
     float curTemperature;                                                /* !< Current temperature value */
} IARM_Bus_PWRMgr_GetThermalState_Param_t;

#define IARM_BUS_PWRMGR_API_GetThermalState             "GetThermalState" /*!< Retrieves current thermal level of the box*/

/**
 *  @brief Structure which holds the  thermal threshold value to be set to the device.
 */
typedef struct _IARM_Bus_PWRMgr_SetTempThresholds_Param_t{
     float tempHigh;                     /*!< New threshold at which TEMPERATURE_HIGH will be reported  */
     float tempCritical;                  /*!< New threshold at which TEMPERATURE_CRITICAL will be reported  */
} IARM_Bus_PWRMgr_SetTempThresholds_Param_t;

#define IARM_BUS_PWRMGR_API_SetTemperatureThresholds   "SetTemperatureThresholds" /*!< Sets the thermal threshold  for the device*/


/**
 *  @brief Structure which holds the data associated with  current temperature threshold.
 */
typedef struct _IARM_Bus_PWRMgr_GetTempThresholds_Param_t{
     float tempHigh;                     /*!< New threshold at which TEMPERATURE_HIGH will be reported  */
     float tempCritical;                  /*!< New threshold at which TEMPERATURE_CRITICAL will be reported  */
} IARM_Bus_PWRMgr_GetTempThresholds_Param_t;

#define IARM_BUS_PWRMGR_API_GetTemperatureThresholds   "GetTemperatureThresholds" /*!< Gets the thermal threshold  for the device*/

/**
 *  @brief Structure which holds the grace interval value to be set to the device.
 */
typedef struct _IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t{
     int graceInterval;                     /*!< New over teamparature grace interval  */
} IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t;

#define IARM_BUS_PWRMGR_API_SetOvertempGraceInterval   "SetOvertempGraceInterval" /*!< Sets the over temparature grace interval for the device*/

/**
 *  @brief Structure which holds the data associated with  current over temparature grace interval.
 */
typedef struct _IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t{
     int graceInterval;                     /*!< New over temparature grace interval  */
} IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t;

#define IARM_BUS_PWRMGR_API_GetOvertempGraceInterval   "GetOvertempGraceInterval" /*!< Gets the over temparature grace interval for the device*/

/** @brief This function will be used to initialize thermal protection thread */
extern void initializeThermalProtection();
#endif //ENABLE_THERMAL_PROTECTION

/**
 *  @brief Structure which holds the setting for whether video port is enabled in standby.
 */

#define PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH 16
typedef struct _IARM_Bus_PWRMgr_StandbyVideoState_Param_t{
     char port[PWRMGR_MAX_VIDEO_PORT_NAME_LENGTH]; 
     int isEnabled;
     int result;
} IARM_Bus_PWRMgr_StandbyVideoState_Param_t;
#define IARM_BUS_PWRMGR_API_SetStandbyVideoState "SetStandbyVideoState"
#define IARM_BUS_PWRMGR_API_GetStandbyVideoState "GetStandbyVideoState"

#define IARM_BUS_PWRMGR_API_SetNetworkStandbyMode        "SetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_GetNetworkStandbyMode        "GetNetworkStandbyMode"
typedef struct _IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t {
       bool bStandbyMode;        /*!< Standby mode to set and get*/
} IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t;

#define MAX_PWR_STATE_BEF_REBOOR_STR_LEN (32)
#define IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot 		"GetPowerStateBeforeReboot" /*!< Retrives power state before reboot*/
/**
 *  @brief Structure which holds the power state before reboot of the CPE.
 */
typedef struct _IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t {
    char powerStateBeforeReboot [MAX_PWR_STATE_BEF_REBOOR_STR_LEN];        /*!< Powerstate before reboot of the box*/ 
} IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t;


#define PWRMGR_MAX_REBOOT_REASON_LENGTH 100
#define PWRMGR_REBOOT_REASON_MAINTENANCE "MAINTENANCE_REBOOT"
/**
 *  @brief Structure to pass reboot reason argument with the reboot call.
 */
typedef struct _IARM_Bus_PWRMgr_RebootParam_t{
     char reboot_reason_custom[PWRMGR_MAX_REBOOT_REASON_LENGTH];
     char reboot_reason_other[PWRMGR_MAX_REBOOT_REASON_LENGTH];
     char requestor[PWRMGR_MAX_REBOOT_REASON_LENGTH];
} IARM_Bus_PWRMgr_RebootParam_t;
#define IARM_BUS_PWRMGR_API_Reboot "performReboot" /*!< Reboots device.*/
#ifdef ENABLE_SET_WAKEUP_SRC_CONFIG //ToDo Remove aftre rdkservices merge.
/**
 *  @brief Structure which holds the wakeup source type and the value to be set.
 */
typedef struct _IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t{
     WakeupSrcType_t srcType;
     bool config;
} IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t;
#endif
/**
 *  @brief Structure which holds the wakeup source type and the value to be set and the power state.
 */
typedef struct _IARM_Bus_PWRMgr_WakeupSrcConfig_Param_t{
     uint32_t pwrMode;
     uint32_t srcType;
     uint32_t config;
} IARM_Bus_PWRMgr_WakeupSrcConfig_Param_t;
#define IARM_BUS_PWRMGR_API_SetWakeupSrcConfig "setWakeupSrcConfig" /*!< sets wakup configuration*/
#define IARM_BUS_PWRMGR_API_GetWakeupSrcConfig "getWakeupSrcConfig" /*!< gets wakup configuration*/

/**
 * Declare RPC API names and their arguments
 */
#define IARM_BUS_PWRMGR_API_handleDeepsleepTimeoutWakeup 		"handleDeepsleepTimeoutWakeup" /*!< Invoke when deepsleep timeout occurs*/


#define IARM_BUS_DEEPSLEEPMGR_NAME 				"DEEPSLEEPMgr"  /*!< Power manager IARM bus name */

typedef enum _DeepSleepStatus_t {
    DeepSleepStatus_Failed = -1,        /*!< Deepsleep operation  failed*/
    DeepSleepStatus_NotStarted = 0,        /*!< Deepsleep operation not started*/
    DeepSleepStatus_InProgress,        /*!< Deepsleep operation in progress */
    DeepSleepStatus_Completed,        /*!< Deepsleep operation completed */
} DeepSleepStatus_t;

/** Sets the timer for deep sleep ,timer is set explicitly by client of deep sleep manager,
 * then the STB will accept the timer value, and go to sleep when sleep timer is expired.
 */
#define IARM_BUS_PWRMGR_API_GetLastWakeupReason		"GetLastWakeupReason"
#define IARM_BUS_PWRMGR_API_GetLastWakeupKeyCode		"GetLastWakeupKeycode"

#ifdef __cplusplus
}
#endif
#endif

/** @} */ // End of Doxygen Tag

/** @} */
/** @} */
