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
* @file rdmMgr.h
*
* @brief IARM-Bus RDM Manager Public API.
*/

/**
* @defgroup iarmmgrs
* @{
* @defgroup rdmmgr
* @{
**/


#ifndef _IARM_BUS_RDMMGR_H
#define _IARM_BUS_RDMMGR_H


#ifdef __cplusplus
extern "C" 
{
#endif

#define IARM_BUS_RDMMGR_NAME 					    "RDMMgr" /*!< IARM BUS  name for rdm manager */

#define RDM_PKG_NAME								"pkg_name"
#define RDM_PKG_VERSION								"pkg_version"
#define RDM_PKG_INST_PATH							"pkg_inst_path"
#define RDM_PKG_INST_STATUS							"pkg_inst_status"
#define RDM_PKG_NAME_MAX_SIZE							128
#define RDM_PKG_VERSION_MAX_SIZE						8
#define RDM_PKG_INST_PATH_MAX_SIZE						256

/*! Published Events from rdm manager  */
typedef enum _RDMMgr_EventId_t {
	IARM_BUS_RDMMGR_EVENT_APPDOWNLOADS_CHANGED = 0,		/*!< RDM application download status chnaged*/
	IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS,    /*!< RDM application installation status */
	IARM_BUS_RDMMGR_EVENT_MAX				/*!< Max Event Id */
} IARM_Bus_RDMMgr_EventId_t;


typedef enum _RDMMgr_Status_t {
	RDM_PKG_INSTALL_COMPLETE = 0,
	RDM_PKG_INSTALL_ERROR, // 1
	RDM_PKG_DOWNLOAD_COMPLETE, // 2
	RDM_PKG_DOWNLOAD_ERROR, // 3
	RDM_PKG_EXTRACT_COMPLETE, // 4
	RDM_PKG_EXTRACT_ERROR, // 5
	RDM_PKG_VALIDATE_COMPLETE, // 6
	RDM_PKG_VALIDATE_ERROR, // 7
	RDM_PKG_POSTINSTALL_COMPLETE, // 8
	RDM_PKG_POSTINSTALL_ERROR, // 9
	RDM_PKG_UNINSTALL, // 10
	RDM_PKG_INVALID_INPUT // 11
} IARM_RDMMgr_Status_t;


typedef struct _RDMMgr_EventData_t {
	struct _pkg_info {
	    char pkg_name[RDM_PKG_NAME_MAX_SIZE];
	    char pkg_version[RDM_PKG_VERSION_MAX_SIZE];
	    char pkg_inst_path[RDM_PKG_INST_PATH_MAX_SIZE];
	    IARM_RDMMgr_Status_t pkg_inst_status;
	} rdm_pkg_info;
} IARM_Bus_RDMMgr_EventData_t;


#ifdef __cplusplus
}
#endif

#endif

/* End of IARM_BUS_RDMMGR_API doxygen group */
/**
 * @}
 */



/** @} */
/** @} */
