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

#include "rrdDeepSleep.h"
#include "rrdRunCmdThread.h"
#include "rrdInterface.h"

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
                RRDRdmManagerDownloadRequest(&issueStructNode, dynJSONPath, rbuf, true);
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

