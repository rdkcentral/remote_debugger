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

#include "rrdDynamic.h"
#include "rrdRunCmdThread.h"
#include "rrdInterface.h"

extern rbusHandle_t rrdRbusHandle;
extern devicePropertiesData devPropData;

/*
 * @function RRDGetDeepSleepdynJSONPathLen
 * @brief Calculates the length of the file path for the deep sleep dynamic JSON file 
 *              based on various components like media apps path, package prefix, device profile, etc.
 * @param None.
 * @return int - Returns the total length of the deep sleep dynamic JSON file path.
 */
int RRDGetDeepSleepdynJSONPathLen()
{
    int length = 0;
    length += strlen(RRD_MEDIA_APPS);
    length += strlen(RDM_PKG_PREFIX);
    length += strlen(DEEP_SLEEP_STR);
    length += strlen(getRrdProfileName(&devPropData));
    length += strlen(RRD_JSON_FILE);
    return length + 1;
}

/*
 * @function RRDProcessDeepSleepAwakeEvents
 * @brief Handles deep sleep events by extracting issue data and processing dynamic package requests 
 *              during deep sleep or awakens state.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @return void
 */
void RRDProcessDeepSleepAwakeEvents(data_buf *rbuf)
{
    issueNodeData issueStructNode = {NULL, NULL};
    char *jsonParsed = NULL;
    char *dynJSONPath = NULL;
    int dynJSONPathLen = 0;

    if ((rbuf) && (rbuf->mdata)) // issue data exits
    {
        /* Extract Issue Data*/
        getIssueInfo((char *)rbuf->mdata, &issueStructNode); // issue data extract
        switch (rbuf->dsEvent)
        {
        case RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE:

            /*Prepare Dynamic JSON File Path*/
            dynJSONPathLen = RRDGetDeepSleepdynJSONPathLen();
            dynJSONPath = (char *)malloc(dynJSONPathLen);
            if (dynJSONPath)
            {
		/* Wdiscarded-qualifiers : Function returns Const char* value and assigned to a char* variable effectively discards the const qualifier */
                const char *profile = getRrdProfileName(&devPropData);
                if(profile && strlen(profile) > 0)
                {
                    dynJSONPathLen += 1;
                    snprintf(dynJSONPath, dynJSONPathLen, "%s%s%s%s%s%s", RRD_MEDIA_APPS, RDM_PKG_PREFIX, profile, "-", issueStructNode.Node, RRD_JSON_FILE);
                }
                else
                {
                    snprintf(dynJSONPath, dynJSONPathLen, "%s%s%s%s", RRD_MEDIA_APPS, RDM_PKG_PREFIX, issueStructNode.Node, RRD_JSON_FILE);
                }
                /*Initiate RDM Manager Download Request*/
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Sending RDM Download Request for DeepSleep dynamic package...\n", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
                RRDRdmManagerDownloadRequest(&issueStructNode, dynJSONPath, rbuf, true);
#endif
                /*Free Recieved Buffer and Dynamic Json Path Pointer*/
                free(dynJSONPath);
                free(rbuf->mdata);
            }
            else
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed.\n", __FUNCTION__, __LINE__);
            }
            break;
        case RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE:
            if (rbuf->inDynamic)
            {
                /*Check Issue in Dynamic Profile*/
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Processing DeepSleep dynamic Profile Package...\n", __FUNCTION__, __LINE__);
                jsonParsed = RRDCheckIssueInDynamicProfile(rbuf, &issueStructNode);
                if (jsonParsed)
                {
                    checkIssueNodeInfo(&issueStructNode, cJSON_Parse(jsonParsed), rbuf, true, NULL);
                }
            }
            break;
        default:
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Deep Sleep Status %d!!!\n", __FUNCTION__, __LINE__, rbuf->dsEvent);
            break;
        }
    }
}

/*
 * @function RRDGetProfileStringLength
 * @brief Calculates the JSON path length for a given profile, taking into account different
 *              device types and whether it is a deep sleep awake event.
 * @param issueNodeData *pissueStructNode - Pointer to structure containing issue node data.
 * @param bool isDeepSleepAwakeEvent - Flag to indicate if this is a deep sleep awake event.
 * @return int - Returns the calculated JSON path length.
 */
int RRDGetProfileStringLength(issueNodeData *pissueStructNode, bool isDeepSleepAwakeEvent)
{
    int length = -1;
    unsigned int prefixlen = strlen(RDM_PKG_PREFIX);
    unsigned int suffixlen = strlen(RDM_PKG_SUFFIX);
    unsigned int nodelen = 0;

    /* Calculate Length for Device Type for Deep Sleep Awake Event*/
    if (isDeepSleepAwakeEvent)
    {
        suffixlen += strlen(DEEP_SLEEP_STR);
        const char *profileName = getRrdProfileName(&devPropData);

        if(profileName && strlen(profileName) > 0){
            length = prefixlen + strlen(profileName) + suffixlen + 1;
        }
        else{
            length = prefixlen + suffixlen;
        }
    }
    else
    {
        nodelen = strlen(pissueStructNode->Node);
        length = prefixlen + nodelen + suffixlen;
    }
    return length + 1;
}

/*
 * @function RRDRdmManagerDownloadRequest
 * @brief Sends a request to the RDM Manager to download a specific package based on the
 *              issue node data and whether it is related to a deep sleep awake event.
 * @param issueNodeData *pissueStructNode - Pointer to the structure containing issue node data.
 * @param char *dynJSONPath - Path to the dynamic JSON file.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @param bool isDeepSleepAwakeEvent - Flag to indicate if this is a deep sleep awake event.
 * @return void
 */

void RRDRdmManagerDownloadRequest(issueNodeData *pissueStructNode, char *dynJSONPath, data_buf *rbuf, bool isDeepSleepAwakeEvent)
{
    char *paramString = NULL;
    char *msgDataString = NULL;
    char *appendData = NULL;
    unsigned char objSize = sizeof(unsigned char);
    int mSGLength = -1;
    int msgDataStringSize = -1;


    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    if (pissueStructNode)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Data Node:%s and Sub-Node:%s not found in Dynamic JSON File %s...\n", __FUNCTION__, __LINE__, pissueStructNode->Node, pissueStructNode->subNode, dynJSONPath);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Request RDM Manager Download for a new Issue Type\n", __FUNCTION__, __LINE__);

        /* Get mSGLength */
#ifdef IARMBUS_SUPPORT
	mSGLength = RRDGetProfileStringLength(pissueStructNode, isDeepSleepAwakeEvent);
#else
        mSGLength = RRDGetProfileStringLength(pissueStructNode, false);
#endif
        if (mSGLength > 0)
        {
            paramString = (char *)calloc(mSGLength, objSize);
            if (paramString)
            {
                /* Get paramString for Device */
#ifdef IARMBUS_SUPPORT
                if (isDeepSleepAwakeEvent)
                {
                    const char *profileName = getRrdProfileName(&devPropData);
                    msgDataStringSize = strlen(RDM_PKG_PREFIX) + strlen(DEEP_SLEEP_STR) + strlen(RDM_PKG_SUFFIX);

                    if(profileName && strlen(profileName) > 0) 
                    {
                        msgDataStringSize += strlen(profileName) + 1;
                        snprintf(paramString, mSGLength, "%s%s%s%s%s", RDM_PKG_PREFIX, profileName, "-", DEEP_SLEEP_STR, RDM_PKG_SUFFIX);
                    }
                    else{
                        snprintf(paramString, mSGLength, "%s%s%s", RDM_PKG_PREFIX, DEEP_SLEEP_STR, RDM_PKG_SUFFIX);
                    }
                }
                else
                {
                    snprintf(paramString, mSGLength, "%s%s%s", RDM_PKG_PREFIX, pissueStructNode->Node, RDM_PKG_SUFFIX);
                    msgDataStringSize = strlen(RDM_PKG_PREFIX) + strlen(pissueStructNode->Node) + 1;
                }
#else
                snprintf(paramString, mSGLength, "%s%s%s", RDM_PKG_PREFIX, pissueStructNode->Node, RDM_PKG_SUFFIX);
                msgDataStringSize = strlen(RDM_PKG_PREFIX) + strlen(pissueStructNode->Node) + 1;
#endif
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Package TAR File Name : %s\n", __FUNCTION__, __LINE__, paramString);

                msgDataString = (char *)calloc(msgDataStringSize, objSize);
                if (msgDataString)
                {
#ifdef IARMBUS_SUPPORT
                    if (isDeepSleepAwakeEvent)
                        strncpy(msgDataString, paramString, (msgDataStringSize - strlen(RDM_PKG_SUFFIX)));
                    else
                        snprintf(msgDataString, msgDataStringSize, "%s%s", RDM_PKG_PREFIX, pissueStructNode->Node);
#else
		    snprintf(msgDataString, msgDataStringSize, "%s%s", RDM_PKG_PREFIX, pissueStructNode->Node);
#endif

                    /* Send RDM Manager Download Request */
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Request RDM Manager Download for... %s\n", __FUNCTION__, __LINE__, paramString);
                    rbusError_t rc = RBUS_ERROR_BUS_ERROR;
                    rbusValue_t value;
	            rbusValue_Init(&value);
                    rbusValue_SetString(value,paramString);
                    rc = rbus_set(rrdRbusHandle,RDM_MGR_PKG_INST, value, NULL);
		    //tr181status = setParam("rrd", RDM_MGR_PKG_INST, paramString);
                    if (rc == RBUS_ERROR_SUCCESS)
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Setting Parameters using rbus success...\n", __FUNCTION__, __LINE__);
			/* Append Package string in Cache */
                        if (rbuf->appendMode)
                        {
                            appendData = (char *)malloc(strlen(APPEND_SUFFIX) + strlen(rbuf->mdata) + 1);
                            strcpy(appendData,rbuf->mdata);
                            strcat(appendData,APPEND_SUFFIX);
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Cache String updated in appendmode IssueStr:%s Length:%d\n", __FUNCTION__, __LINE__, appendData, strlen(appendData));
                            append_item(strdup(msgDataString), strdup(appendData));
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Setting Parameters Success and Cache Updated ...%s IssueStr:%s Length:%d\n", __FUNCTION__, __LINE__, msgDataString, appendData, strlen(appendData));
                        }
                        else
                        {
                            append_item(strdup(msgDataString), strdup((char *)rbuf->mdata));
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Setting Parameters Success and Cache Updated ...%s IssueStr:%s Length:%d\n", __FUNCTION__, __LINE__, msgDataString, (char *)rbuf->mdata, strlen((char *)rbuf->mdata));
                        }			    
                    }
                    else
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Setting Parameters failed...\n", __FUNCTION__, __LINE__);
                    }
                    free(msgDataString);
		    free(appendData);
                }
                else
                {
                    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for Request RDM Manager Download.\n", __FUNCTION__, __LINE__);
                }
                free(paramString);
            }
            else
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for Request RDM Manager Download.\n", __FUNCTION__, __LINE__);
            }
            free(pissueStructNode->Node);
            free(pissueStructNode->subNode);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Invalid Issued Struct Node\n", __FUNCTION__, __LINE__);
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function RRDCheckIssueInDynamicProfile
 * @brief Checks for a specific issue in the dynamic JSON profile associated with the given
 *              buffer. Parses the JSON file and searches for the issue.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @param issueNodeData *issueStructNode - Pointer to structure containing issue node data.
 * @return char* - Returns the parsed JSON as a string if the issue is found, otherwise returns NULL.
 */
char *RRDCheckIssueInDynamicProfile(data_buf *rbuf, issueNodeData *issueStructNode)
{
    bool isDynamicIssueInJSON = false;
    char *jsonParsed = NULL;


    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    if (rbuf->inDynamic)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Issue Marked as inDynamic... \n", __FUNCTION__, __LINE__);
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Checking Dynamic Profile... \n", __FUNCTION__, __LINE__);
        if (!rbuf->jsonPath) // Dynamic Profile JSON does not exists
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile JSON Not Found... \n", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
            free(rbuf->mdata);
#else
            if (rbuf->mdata) {
                free(rbuf->mdata);
                rbuf->mdata = NULL;
            }
#endif
        }
        else
        {
            /* Parse Dynamic Profile JSON from Package */
            /* Fix for warning : Wincompatible-pointer-types */ 
            jsonParsed = cJSON_Print(readAndParseJSON(rbuf->jsonPath));
            if (!jsonParsed)
            {
                /* Parsing Failed */
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic JSON Parse/Read failed... %s\n", __FUNCTION__, __LINE__, rbuf->jsonPath);
#if !defined(GTEST_ENABLE)
                free(rbuf->mdata);
                free(rbuf->jsonPath);
#else
                if (rbuf->mdata) {
                    free(rbuf->mdata);
                    rbuf->mdata = NULL;
                }
                if (rbuf->jsonPath) {
                    free(rbuf->jsonPath);
                    rbuf->jsonPath = NULL;
                }
#endif
            }
            else
            {
                /* Find the issue in Parsed Json File */
                /* Fix for warning : Wincompatible-pointer-types */
                isDynamicIssueInJSON = findIssueInParsedJSON(issueStructNode, cJSON_Parse(jsonParsed));
                if (!isDynamicIssueInJSON) /* Issue Data not in Dynamic Profile JSON */
                {
                    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Issue Data not found in Dynamic JSON %s... \n", __FUNCTION__, __LINE__, rbuf->jsonPath);
#if !defined(GTEST_ENABLE)
                    free(rbuf->mdata);
                    free(rbuf->jsonPath);
                    free(jsonParsed);
                    jsonParsed = NULL;
#else
                    if (rbuf->mdata) {
                        free(rbuf->mdata);
                        rbuf->mdata = NULL;
                    }
                    if (rbuf->jsonPath) {
                        free(rbuf->jsonPath);
                        rbuf->jsonPath = NULL;
                    }
                    if (jsonParsed) {
                        free(jsonParsed);
                        jsonParsed = NULL;
                    }
#endif
                }
            }
        }
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return jsonParsed;
}
