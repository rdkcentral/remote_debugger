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

#include "rrdRunCmdThread.h"
#include "rrdJsonParser.h"
#include "rrdEventProcess.h"

#define COMMAND_DELIM ';'

static void processIssueType(data_buf *rbuf);
static void processIssueTypeInDynamicProfile(data_buf *rbuf, issueNodeData *pIssueNode);
static void processIssueTypeInStaticProfile(data_buf *rbuf, issueNodeData *pIssueNode);
static void processIssueTypeInInstalledPackage(data_buf *rbuf, issueNodeData *pIssueNode);
static void removeSpecialCharacterfromIssueTypeList(char *str);
static int issueTypeSplitter(char *input_str, const char delimeter, char ***args);
static void freeParsedJson(cJSON *jsonParsed);

/*
 * @function processWebCfgTypeEvent
 * @brief Processes a web configuration type event by decoding the data and freeing the buffer data.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @return void
 */
void processWebCfgTypeEvent(data_buf *rbuf)
{
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    if(rbuf && rbuf->mdata)
    {
        decodeWebCfgData(rbuf->mdata);
        free(rbuf->mdata);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function processIssueTypeEvent
 * @brief Processes an issue type event by splitting the issue types from the input data buffer,
 *              initializing command buffers and handling each issue type individually.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @return void
 */
void processIssueTypeEvent(data_buf *rbuf)
{
    char **cmdMap;
    int index = 0, count = 0, dataMsgLen = 0;
    data_buf *cmdBuff = NULL;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    if (NULL != rbuf)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType List [%s]... \n", __FUNCTION__, __LINE__, rbuf->mdata);
        count = issueTypeSplitter(rbuf->mdata, ',', &cmdMap);
        
        if (count > 0)
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType Count [%d]... \n", __FUNCTION__, __LINE__, count);
            for (index = 0; index < count; index++)
            {
                cmdBuff = NULL;
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: IssueType [%s]... \n", __FUNCTION__, __LINE__, cmdMap[index]);
                cmdBuff = (data_buf *)malloc(sizeof(data_buf));
                if (cmdBuff)
                {
                    dataMsgLen = strlen(cmdMap[index]) + 1;
                    RRD_data_buff_init(cmdBuff, EVENT_MSG, RRD_DEEPSLEEP_INVALID_DEFAULT); /* Setting Deafult Values*/
                    cmdBuff->inDynamic = rbuf->inDynamic;
                    if(cmdBuff->inDynamic)
                    {
                        cmdBuff->jsonPath = rbuf->jsonPath;
                    }
		    cmdBuff->appendMode = rbuf->appendMode;
                    cmdBuff->mdata = (char *)calloc(1, dataMsgLen);
                    if (cmdBuff->mdata)
                    {
                        strncpy((char *)cmdBuff->mdata, cmdMap[index], dataMsgLen);
                        processIssueType(cmdBuff);
                    }
                    else
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
                    }
                    free(cmdBuff);
                }
                else
                {
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
                }
                free(cmdMap[index]);
            }
            free(cmdMap);
        }
    }
    
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function processIssueType
 * @brief Processes the given issue type by extracting issue node data and determining
 *              whether to handle it as a dynamic or static profile.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @return void
 */
static void processIssueType(data_buf *rbuf)
{
    issueNodeData *pIssueNode = NULL;
    issueData *dynamicprofiledata = NULL;
    issueData *staticprofiledata = NULL;    

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    if (rbuf->mdata != NULL) // issue data exits
    {
        pIssueNode = (issueNodeData *)malloc(sizeof(issueNodeData));
        if(pIssueNode)
        {
            getIssueInfo((char *)rbuf->mdata, pIssueNode); // issue data extract
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Extracted Node %s and Sub Node %s \n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode);
            if (rbuf->appendMode)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Received append request to process static and dynamic profiles... \n", __FUNCTION__, __LINE__);
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Reading dynamic profile command info... \n", __FUNCTION__, __LINE__);
                dynamicprofiledata = processIssueTypeInDynamicProfileappend(rbuf, pIssueNode);
                if (dynamicprofiledata == NULL)
                {
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Dynamic Profie Info not found, Download RDM package... \n", __FUNCTION__, __LINE__);
                }
                else
                {
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Read complete for Dynamic Profile: RFCValue: %s, Command: %s, Timeout: %d... \n", __FUNCTION__, __LINE__, dynamicprofiledata->rfcvalue, dynamicprofiledata->command, dynamicprofiledata->timeout);
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Reading static profile command info... \n", __FUNCTION__, __LINE__);
                    staticprofiledata = processIssueTypeInStaticProfileappend(rbuf, pIssueNode);
                    if (staticprofiledata == NULL)
                    {
                        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Static Command Info not found for IssueType!!! \n", __FUNCTION__, __LINE__);
                    }
                    else
                    {
                        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Read complete for Static Profile: RFCValue: %s, Command: %s, Timeout: %d... \n", __FUNCTION__, __LINE__, staticprofiledata->rfcvalue, staticprofiledata->command, staticprofiledata->timeout);
                        //Remove the double quotes
                        size_t staticstrlen = strlen(staticprofiledata->command);
                        size_t dynamicstrlen = strlen(dynamicprofiledata->command);
                        if (staticstrlen > 0 && staticprofiledata->command[staticstrlen - 1] == '"') {
                            staticprofiledata->command[staticstrlen - 1] = '\0';
                        }
                        if (dynamicprofiledata->command[0] == '"') {
                            dynamicprofiledata->command[0] = COMMAND_DELIM;
                        }
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Static Profile Commands: %s, Dynamic Profile Commands: %s\n", __FUNCTION__, __LINE__, staticprofiledata->command, dynamicprofiledata->command);

                        size_t appendstrlen = ((staticstrlen - 1) + dynamicstrlen + 1);
                        char *appendcommandstr = (char *)realloc(staticprofiledata->command, appendstrlen);
                        if (appendcommandstr == NULL) {
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
                        }
                        strcat(appendcommandstr, dynamicprofiledata->command);
                        staticprofiledata->command = appendcommandstr;
                        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Updated command after append from dynamic and static profile: %s \n", __FUNCTION__, __LINE__, staticprofiledata->command);
                        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Executing Commands in Runtime Service... \n",__FUNCTION__,__LINE__);
                        checkIssueNodeInfo(pIssueNode, NULL, rbuf, false, staticprofiledata); 
                    }
                }
	    }	    
	    else if (rbuf->inDynamic)
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Checking if Issue marked inDynamic... \n", __FUNCTION__, __LINE__);
                processIssueTypeInDynamicProfile(rbuf, pIssueNode);
            }
            else
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue not marked as inDynamic... \n", __FUNCTION__, __LINE__);
		RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Checking Issue from Static... \n", __FUNCTION__, __LINE__);
                processIssueTypeInStaticProfile(rbuf, pIssueNode);
            }
        }
        else
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        /* Fix for warning Wformat-overflow : directive argument is null*/
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Value is empty for RFC Parameter \n", __FUNCTION__, __LINE__);
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function processIssueTypeInDynamicProfile
 * @brief Processes the given issue type by checking if the issue exists in the dynamic profile JSON.
 *              If the issue is found, it handles the issue appropriately.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @param issueNodeData *pIssueNode - Pointer to structure containing issue node data.
 * @return void
 */
static void processIssueTypeInDynamicProfile(data_buf *rbuf, issueNodeData *pIssueNode)
{
    cJSON *jsonParsed = NULL;
    bool isDynamicIssue=false;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Issue Marked as inDynamic... \n", __FUNCTION__, __LINE__);
    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Checking Dynamic Profile... \n", __FUNCTION__, __LINE__);
    if (rbuf->jsonPath == NULL) // Dynamic Profile JSON does not exists
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile JSON Not Found... \n", __FUNCTION__, __LINE__);
        free(rbuf->mdata);
        rbuf->mdata = NULL;
    }
    else
    {
        jsonParsed = readAndParseJSON(rbuf->jsonPath); // Parse Dynamic Profile JSON from Package
        if (jsonParsed == NULL)
        {
            RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic JSON Parse/Read failed... %s\n", __FUNCTION__, __LINE__, rbuf->jsonPath);
            free(rbuf->mdata);
            rbuf->mdata = NULL;
            free(rbuf->jsonPath);
            rbuf->jsonPath = NULL;
        }
        else
        {
            isDynamicIssue = findIssueInParsedJSON(pIssueNode, jsonParsed);
            if (!isDynamicIssue) // Issue Data not in Dynamic Profile JSON
            {
                RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Issue Data not found in Dynamic JSON %s... \n", __FUNCTION__, __LINE__, rbuf->jsonPath);
                free(rbuf->mdata);
                rbuf->mdata = NULL;
                free(rbuf->jsonPath);
                rbuf->jsonPath = NULL;
            }
            else
            {
                // Issue found in Dynamic Prof JSON
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Data Node: %s and Sub-Node: %s found in Dynamic JSON File %s...\n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode, rbuf->jsonPath);
                checkIssueNodeInfo(pIssueNode, jsonParsed, rbuf, false, NULL);
            }
        }
        freeParsedJson(jsonParsed);
    }

        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function processIssueTypeInStaticProfile
 * @brief Processes the given issue type by checking if the issue exists in the static profile JSON.
 *              If the issue is found, it handles the issue appropriately. If not found, it will further
 *              check in the installed package.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @param issueNodeData *pIssueNode - Pointer to structure containing issue node data.
 * @return void
 */
static void processIssueTypeInStaticProfile(data_buf *rbuf, issueNodeData *pIssueNode)
{
    cJSON *jsonParsed = NULL;
    bool isStaticIssue = false;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Checking Static Profile... \n", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
    jsonParsed = readAndParseJSON(RRD_JSON_FILE);
#else
    jsonParsed = readAndParseJSON(rbuf->jsonPath);
#endif
    if (jsonParsed == NULL)
    { // Static Profile JSON Parsing or Read Fail
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Static Profile Parse/Read failed... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
        processIssueTypeInInstalledPackage(rbuf, pIssueNode);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Static Profile Parse And Read Success... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Check if Issue in Parsed Static JSON... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
    isStaticIssue = findIssueInParsedJSON(pIssueNode, jsonParsed);
    if (isStaticIssue)
    {
        // Issue in Static Profile JSON
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Issue Data Node: %s and Sub-Node: %s found in Static JSON File %s... \n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode, RRD_JSON_FILE);
        checkIssueNodeInfo(pIssueNode, jsonParsed, rbuf, false, NULL); // sanity Check and Get Command List
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d] Issue Data Not found in Static JSON File... \n", __FUNCTION__, __LINE__);
        processIssueTypeInInstalledPackage(rbuf, pIssueNode);
    }

    freeParsedJson(jsonParsed);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

issueData* processIssueTypeInDynamicProfileappend(data_buf *rbuf, issueNodeData *pIssueNode)
{
    issueData *dynamicdata = NULL;
    cJSON *jsonParsed = NULL;
    char *dynJSONPath = NULL;
    int rrdjsonlen = 0, persistentAppslen = 0, prefixlen = 0;
    bool isDynamicIssue = false;


    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    rrdjsonlen = strlen(RRD_JSON_FILE);
    persistentAppslen = strlen(RRD_MEDIA_APPS);
    prefixlen = strlen(RDM_PKG_PREFIX);
    dynJSONPath = (char *)malloc(persistentAppslen + prefixlen + strlen(pIssueNode->Node) + rrdjsonlen + 1);

    if(dynJSONPath == NULL)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
        return dynamicdata;
    }
    sprintf(dynJSONPath, "%s%s%s%s", RRD_MEDIA_APPS, RDM_PKG_PREFIX, pIssueNode->Node, RRD_JSON_FILE);

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Checking Dynamic Profile... \n", __FUNCTION__, __LINE__);
    jsonParsed = readAndParseJSON(dynJSONPath);
    if (jsonParsed == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile Parse/Read failed... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
#ifdef IARMBUS_SUPPORT
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Going to RDM Request... \n", __FUNCTION__, __LINE__);
        RRDRdmManagerDownloadRequest(pIssueNode, dynJSONPath, rbuf, false); //goto RDM_RRD_REQ_LABEL;
#endif
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile Parse And Read Success... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Check if Issue in Parsed Dynamic JSON... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
        isDynamicIssue = findIssueInParsedJSON(pIssueNode, jsonParsed);
        if (isDynamicIssue)
        {
            // Issue Data in Dynamic Profile JSON
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Data Node:%s and Sub-Node:%s found in Dynamic JSON File %s...\n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode, dynJSONPath);
            free(dynJSONPath);
            // Get the command for received Issue Type of the Issue Category
            dynamicdata = getIssueCommandInfo(pIssueNode, jsonParsed, rbuf->mdata);
            RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile Data: RFCValue: %s, Command: %s, Timeout: %d... \n", __FUNCTION__, __LINE__, dynamicdata->rfcvalue, dynamicdata->command, dynamicdata->timeout);
        }
        else
        {
            // Issue Data not in Dynamic Profile JSON
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Requested Issue Data not found in Dynamic Profile JSON!!! \n", __FUNCTION__, __LINE__);
        }
    }
    freeParsedJson(jsonParsed);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return dynamicdata;
}

issueData* processIssueTypeInStaticProfileappend(data_buf *rbuf, issueNodeData *pIssueNode)
{
    cJSON *jsonParsed = NULL;
    bool isStaticIssue = false;
    issueData *staticdata = NULL;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Static Profile Commands... \n", __FUNCTION__, __LINE__);

    jsonParsed = readAndParseJSON(RRD_JSON_FILE);
    if (jsonParsed == NULL)
    { // Static Profile JSON Parsing or Read Fail
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Static Profile Parse/Read failed... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
        return staticdata;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Static Profile Parse And Read Success... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Check if Issue in Parsed Static JSON... %s\n", __FUNCTION__, __LINE__, RRD_JSON_FILE);
    isStaticIssue = findIssueInParsedJSON(pIssueNode, jsonParsed);
    if (isStaticIssue)
    {
        // Issue in Static Profile JSON
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Issue Data Node: %s and Sub-Node: %s found in Static JSON File %s... \n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode, RRD_JSON_FILE);
        // Get the command for received Issue Type of the Issue Category
        staticdata = getIssueCommandInfo(pIssueNode, jsonParsed, rbuf->mdata);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Static Profile Data: RFCValue: %s, Command: %s, Timeout: %d... \n", __FUNCTION__, __LINE__, staticdata->rfcvalue, staticdata->command, staticdata->timeout);
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d] Issue Data Not found in Static JSON File... \n", __FUNCTION__, __LINE__);
    }

    freeParsedJson(jsonParsed);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return staticdata;
}

/*
 * @function processIssueTypeInInstalledPackage
 * @brief Processes the given issue type by checking if the issue exists in the installed package's dynamic profile JSON.
 *              If the issue is found, it handles the issue appropriately. If the JSON read or parse fails, it sends an RDM request.
 * @param data_buf *rbuf - Buffer containing event data and metadata.
 * @param issueNodeData *pIssueNode - Pointer to structure containing issue node data.
 * @return void
 */
static void processIssueTypeInInstalledPackage(data_buf *rbuf, issueNodeData *pIssueNode)
{
    cJSON *jsonParsed = NULL;
    char *dynJSONPath = NULL;
    int rrdjsonlen = 0, persistentAppslen = 0, prefixlen = 0, suffixlen = 0;
    bool isDynamicIssue = false;


    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
#if !defined(GTEST_ENABLE)
    rrdjsonlen = strlen(RRD_JSON_FILE);
    persistentAppslen = strlen(RRD_MEDIA_APPS);
    prefixlen = strlen(RDM_PKG_PREFIX);
    suffixlen = strlen(RDM_PKG_SUFFIX);
    dynJSONPath = (char *)malloc(persistentAppslen + prefixlen + suffixlen + strlen(pIssueNode->Node) + rrdjsonlen + 1);
#else
    int utjsonlen = strlen(rbuf->jsonPath);
    dynJSONPath = (char *)malloc(utjsonlen + 1);
#endif

    if(dynJSONPath == NULL)
    {    
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed... \n", __FUNCTION__, __LINE__);
        return;
    }
#if !defined(GTEST_ENABLE)
    sprintf(dynJSONPath, "%s%s%s%s", RRD_MEDIA_APPS, RDM_PKG_PREFIX, pIssueNode->Node, RRD_JSON_FILE);
#else
    sprintf(dynJSONPath, "%s", rbuf->jsonPath);
#endif

    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Checking Dynamic Profile... \n", __FUNCTION__, __LINE__);
    jsonParsed = readAndParseJSON(dynJSONPath);
    if (jsonParsed == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile Parse/Read failed... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
#ifdef IARMBUS_SUPPORT
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Going to RDM Request... \n", __FUNCTION__, __LINE__);
        RRDRdmManagerDownloadRequest(pIssueNode, dynJSONPath, rbuf, false); //goto RDM_RRD_REQ_LABEL;
#endif
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Dynamic Profile Parse And Read Success... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Check if Issue in Parsed Dynamic JSON... %s\n", __FUNCTION__, __LINE__, dynJSONPath);
        isDynamicIssue = findIssueInParsedJSON(pIssueNode, jsonParsed);
        if (isDynamicIssue)
        {
            // Issue Data in Dynamic Profile JSON
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Data Node:%s and Sub-Node:%s found in Dynamic JSON File %s...\n", __FUNCTION__, __LINE__, pIssueNode->Node, pIssueNode->subNode, dynJSONPath);
            free(dynJSONPath);
            checkIssueNodeInfo(pIssueNode, jsonParsed, rbuf, false, NULL);
        }
        else
        {
#ifdef IARMBUS_SUPPORT
            // Issue Data not in Dynamic Profile JSON
            RRDRdmManagerDownloadRequest(pIssueNode, dynJSONPath, rbuf, false);
#endif
        }
    }
    freeParsedJson(jsonParsed);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return;
}

/*
 * @function freeParsedJson
 * @brief Frees the memory allocated to a parsed cJSON object.
 * @param cJSON *jsonParsed - Pointer to the parsed JSON object to be freed.
 * @return void
 */
static void freeParsedJson(cJSON *jsonParsed)
{
    if(jsonParsed)
    {
        cJSON_Delete(jsonParsed);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Free static JSON Parsed...\n", __FUNCTION__, __LINE__);
    }
}

/*
 * @function removeSpecialCharacterfromIssueTypeList
 * @brief Removes special characters from the issue type list, retaining only alphanumeric
 *              characters, commas, and periods.
 * @param char *str - The string from which special characters will be removed.
 * @return void
 */
static void removeSpecialCharacterfromIssueTypeList(char *str)
{
    int source = 0;
    int destination = 0;

    while (str[source] != '\0')
    {
        if (isalnum(str[source]) || str[source] == ',' || str[source] == '.')
        {
            str[destination] = str[source];
            ++destination;
        }
        ++source;
    }
    str[destination] = '\0';
}

/*
 * @function issueTypeSplitter
 * @brief Splits a given string into tokens based on a specified delimiter, and removes any
 *              special characters from the string before splitting.
 * @param char *input_str - The input string to be split.
 * @param const char delimiter - The character used to split the string.
 * @param char ***args - Pointer to an array of strings where the tokens will be stored.
 * @return int - The number of tokens found in the string.
 */
static int issueTypeSplitter(char *input_str, const char delimeter, char ***args)
{
    int cnt = 1, i = 0;
    char *str = input_str;

    removeSpecialCharacterfromIssueTypeList(str);
    while (*str == delimeter)
        str++;

    char *str2 = str;
    while (*(str2++))
        if (*str2 == delimeter && *(str2 + 1) != delimeter && *(str2 + 1) != 0)
            cnt++;

    (*args) = (char **)malloc(sizeof(char *) * cnt);

    if (*args == NULL)
    {
        cnt = 0;
    }
    else
    {
        for (i = 0; i < cnt; i++)
        {
            char *ts = str;
            while (*str != delimeter && *str != 0)
                   str++;

            int len = (str - ts + 1);
            (*args)[i] = (char *)malloc(sizeof(char) * len);
            memcpy((*args)[i], ts, sizeof(char) * (len - 1));
            (*args)[i][len - 1] = 0;

            while (*str == delimeter)
                str++;
        }
    }

    return cnt;
}

