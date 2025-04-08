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

#include "rrdJsonParser.h"
#include "rrdRunCmdThread.h"
#include "rrdExecuteScript.h"
#include "rrdCommandSanity.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

/*
 * @function removeSpecialChar
 * @brief Removes special characters ('\r' and '\n') from the device properties parameter string.
 * @param char *str - The string from which special characters will be removed.
 * @return void
 */
void removeSpecialChar(char *str)
{
    int index = 0;
    int length = strlen(str);

    for(index = 0; index < length; index++)
    {
        if(str[index] == '\r' || str[index] == '\n')
        {
            str[index] = '\0';
            break;
        }
    }
}

/*
 * @function getParamcount
 * @brief Calculates the total number of nodes (elements) in the input string, excluding delimiters.
 * @param char *str - The string from TR181 whose nodes are to be counted.
 * @return int - The number of nodes.
 */
int getParamcount(char *str)
{
    int total_node = 0;

    while ( (str=strstr(str,".")) != NULL )
    {
        total_node++;
        str++;
    }
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Total Nodes found in TR69 Parameter = %d \n",__FUNCTION__,__LINE__,total_node);

    return total_node;
}

/*
 * @function readJsonFile
 * @brief Reads the JSON file and stores the content in a character array.
 * @param char *jsonfile - The name of the JSON file whose content is to be read.
 * @return char* - A string containing the JSON content, or NULL on failure.
 */
char * readJsonFile(char *jsonfile)
{
    FILE *fp = NULL;
    int ch_count = 0;
    char *jsonfile_content = NULL;

    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading json config file %s \n",__FUNCTION__,__LINE__,jsonfile);
    fp = fopen(jsonfile, "r");
    if (fp == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Json File %s Read Failed!!! \n",__FUNCTION__,__LINE__,jsonfile);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    ch_count = ftell(fp);
    if(ch_count < 1)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Json File %s is Empty!!! \n",__FUNCTION__,__LINE__,jsonfile);
	// CID 278332: Resource leak (RESOURCE_LEAK)
	fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    jsonfile_content = (char *) malloc(sizeof(char) * (ch_count + 1));
    fread(jsonfile_content, 1, ch_count,fp);
    jsonfile_content[ch_count] ='\0';
    fclose(fp);

    return jsonfile_content;
}

/*
 * @function readAndParseJSON
 * @brief Reads and parses the JSON file.
 * @param char *jsonFile - The JSON file path that is to be parsed.
 * @return cJSON* - Parsed JSON content, or NULL on failure.
 */
cJSON *readAndParseJSON(char *jsonFile)
{
    char *file_content = NULL;
    cJSON *jsoncfg = NULL;
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Start Reading JSON File... %s\n",__FUNCTION__,__LINE__, jsonFile);
    file_content = readJsonFile(jsonFile);
    if(file_content == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Reading json file failed, Skipping Parse!!\n",__FUNCTION__,__LINE__);
        return NULL;
    }
    // Read Success
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading json file Success, Parsing the Content...\n",__FUNCTION__,__LINE__);

    jsoncfg = cJSON_Parse(file_content);
    if(jsoncfg == NULL)
    {
        // Parse Failure
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Json File %s Parse Failed...!!\n",__FUNCTION__,__LINE__,jsonFile);
        free(file_content);     // free file content received from readJsonFile
        return NULL;
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Json File parse Success... %s\n",__FUNCTION__,__LINE__,jsonFile);
        free(file_content);
        return jsoncfg;
    }
}

/*
 * @function getIssueInfo
 * @brief Extracts the issue string and extracts its node and sub-node.
 * @param char *issuestr - The issue string.
 * @param issueNodeData *issue - Structure to store node and sub-node.
 * @return void
 */
void getIssueInfo(char *issuestr, issueNodeData *issue)
{
    int issuestrlen = 0, nodes = 0, i = 0;
    char *tokenptr = NULL;
    rrd_nodes_t ncategory = RRD_CATEGORY;
    rrd_nodes_t ntype = RRD_TYPE;
    int nodelen = 0, subnodelen = 0;

    issuestrlen = strlen(issuestr) + 1;
    char issuestrval[issuestrlen];
    memcpy(issuestrval,issuestr, issuestrlen);

    //Getting Issue Commands
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Getting Issue command Information for : %s\n",__FUNCTION__,__LINE__,issuestrval);

    nodes = getParamcount(issuestrval);
    //create Nodes
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Creating Array of Nodes: %d\n",__FUNCTION__,__LINE__,nodes+1);
    char *array[nodes+1];

    tokenptr = strtok(issuestrval, ".");
    while(tokenptr != NULL)
    {
        array[i++] = tokenptr;
        tokenptr = strtok(NULL, ".");
    }

    nodelen = strlen(array[ncategory]) + 1;
    issue->Node = (char *)malloc(nodelen);
    strcpy(issue->Node, array[ncategory]);
    if (nodes != 0)
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: SubNode found in RFC parameter!!!\n",__FUNCTION__,__LINE__);
        subnodelen = strlen(array[ntype]) + 1;
        issue->subNode = (char *)malloc(subnodelen);
        strcpy(issue->subNode, array[ntype]);
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: SubNode not found in RFC parameter!!!\n",__FUNCTION__,__LINE__);
        issue->subNode = NULL;
    }
    RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]:  Received Main Node= %s, SubNode= %s\n",__FUNCTION__,__LINE__,issue->Node,issue->subNode);
}

/*
 * @function findIssueInParsedJSON
 * @brief Finds if an issue category and issue type is present in the parsed JSON.
 * @param issueNodeData *issuestructNode - Structure with issue category and type as node and sub-node respectively.
 * @param cJSON *jsoncfg - The JSON file in which the issue is to be found.
 * @return bool - Returns true if issue is found in JSON, else returns false.
 */
bool findIssueInParsedJSON(issueNodeData *issuestructNode, cJSON *jsoncfg)
{
    cJSON *category = NULL;
    cJSON *type = NULL;
    char *issuetype = NULL;
    char *categoryname = NULL;
    bool result = false;

    if (!issuestructNode->Node)
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Issue Category missing in RFC Value!!! \n",__FUNCTION__,__LINE__);
    }
    else
    {
        category = cJSON_GetObjectItem(jsoncfg, issuestructNode->Node);
        if (!category)
        {
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Issue Category:%s not found in JSON File!!! \n",__FUNCTION__,__LINE__,issuestructNode->Node);
        }
        else
        {
            categoryname = cJSON_Print(category);   // print issue category name
	    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Issue Category:%s...\n%s\n",__FUNCTION__,__LINE__,issuestructNode->Node,categoryname);
	    
            if (categoryname && !strcmp(categoryname, DEEP_SLEEP_STR))
            {
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Type :%s, Reading all Sub Issue types \n", __FUNCTION__, __LINE__, issuestructNode->Node);
                result = true;
            }
            else
            {
                if (issuestructNode->subNode == NULL)
                {
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Type missing in RFC Value:%s, Reading all Sub Issue types \n", __FUNCTION__, __LINE__, issuestructNode->Node);
                    result = true;
                }
                else
                {
                    type = cJSON_GetObjectItem(category, issuestructNode->subNode);
                    if (!type)
                    {
                        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Type:%s of Category:%s not found in JSON File!!! \n", __FUNCTION__, __LINE__, issuestructNode->subNode, issuestructNode->Node);
                        result = false;
                    }
                    else
                    {
                        issuetype = cJSON_Print(type); // print issue type name
			if(!issuetype)
			{
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Issue Type:%s of Category:%s not found in JSON File!!! \n", __FUNCTION__, __LINE__, issuestructNode->subNode, issuestructNode->Node);
                            result = false;
			}
			else
			{
                            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Reading Issue Type:%s...\n%s\n", __FUNCTION__, __LINE__, issuestructNode->subNode, issuetype);
                            cJSON_free(issuetype); // free issue type name
                            result = true;
			}
                    }
                }
	    }
            cJSON_free(categoryname); // free issue category name
        }
    }
    return result;
}

/* Function: getIssueCommandInfo
 * Details: checks the sanity and chekc commands in the Profile JSON and gets the commands for the issues and proceeds for execution
 * Inputs: issuestructNode structure with Issue Category and Issye Type, jsoncontent for JSON Parsing, issueData issue and the received issue string
 * Returns: NULL or Structure with command information
*/

issueData * getIssueCommandInfo(issueNodeData *issuestructNode, cJSON *jsoncfg, char *buf)
{
    int nitems = 0, j = 0, ret = 0;
    issueData *issuestdata = NULL;
    cJSON *sanity = NULL;
    cJSON *check = NULL;
    cJSON *cmdlist = NULL;
    cJSON *category = NULL;
    cJSON *type = NULL;
    char *checkval = NULL;
    cJSON *elem = NULL;
    char *tmpCommand = NULL;

    category = cJSON_GetObjectItem(jsoncfg, issuestructNode->Node);
    type = cJSON_GetObjectItem(category, issuestructNode->subNode);

    issuestdata = (issueData *) malloc(sizeof(issueData));
    if(issuestdata == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Memory Allocation Failure \n",__FUNCTION__,__LINE__);
	return issuestdata;
    }
    issuestdata->rfcvalue = NULL;
    issuestdata->command = NULL;
    issuestdata->timeout = 0;

    /* Read Command and Timeout information for Issuetype */
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Command and Timeout information for Debug Issue\n",__FUNCTION__,__LINE__);
    nitems = cJSON_GetArraySize(type);
    for(j = 0; j < nitems; j++)
    {
        elem = cJSON_GetArrayItem(type, j);
        if(elem && elem->type == cJSON_Number)
        {
            issuestdata->timeout = elem->valueint;  // copy timeout info from json file
        }
        else if(elem && elem->type == cJSON_String)
        {
            tmpCommand = cJSON_Print(elem);
            if(tmpCommand)
            {
                issuestdata->command = strdup(tmpCommand);   // print command info from json file
                cJSON_free(tmpCommand);
            }
        }
    }

    if(issuestdata->command == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: No Commands found, exiting.. \n",__FUNCTION__,__LINE__);
        free(issuestdata);
    }
    else
    {
        sanity = cJSON_GetObjectItem(jsoncfg, "Sanity");
        check = cJSON_GetObjectItem(sanity, "Check");
        checkval = cJSON_Print(check);  // Print list of sanity commands received
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Sanity Check Commands List: \n%s\n",__FUNCTION__,__LINE__,checkval);
        cmdlist = cJSON_GetObjectItem(check, "Commands");
        cJSON_free(checkval); // free sanity command list
        ret = isCommandsValid(issuestdata->command, cmdlist);
        if(ret != 0)
        {
            RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Aborting Command execution due to Harmful commands!!!\n",__FUNCTION__,__LINE__);
            free(issuestdata->command);
            free(issuestdata);
        }
        else
        {
            issuestdata->rfcvalue = strdup(buf);
            if(issuestdata->timeout == 0)
            {
                issuestdata->timeout = DEFAULT_TIMEOUT; // Use Default Systemd service timeout of 90 seconds
                RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Using default timeout of %d seconds\n",__FUNCTION__,__LINE__,issuestdata->timeout);
            }
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Value of rfcvalue: %s command: %s time: %d\n",__FUNCTION__,__LINE__, issuestdata->rfcvalue,issuestdata->command,issuestdata->timeout);
        }
    }

    return issuestdata;
}

/*
 * @function invokeSanityandCommandExec
 * @brief Checks the sanity and commands in the profile JSON and gets the commands for the issues, then proceeds for execution.
 * @param issueNodeData *issuestructNode - Structure with issue category and type.
 * @param       cJSON *jsoncfg - Parsed JSON content.
 * @param char *buf - Issue string.
 * @param bool deepSleepAwkEvnt - Flag indicating if it is a deep sleep wake event.
 * @return bool - Returns true for successful execution and false for failure.
 */
bool invokeSanityandCommandExec(issueNodeData *issuestructNode, cJSON *jsoncfg, char *buf, bool deepSleepAwkEvnt)
{
    int nitems = 0, j = 0, ret = 0;
    issueData *issuestdata = NULL;
    cJSON *sanity = NULL;
    cJSON *check = NULL;
    cJSON *cmdlist = NULL;
    cJSON *root = NULL;
    cJSON *category = NULL;
    cJSON *type = NULL;
    char *checkval = NULL;
    cJSON *elem = NULL;
    bool exresult = false;
    char *tmpCommand = NULL;

    if(deepSleepAwkEvnt)
    {
        root = cJSON_GetObjectItem(jsoncfg, DEEP_SLEEP_STR);
        category = cJSON_GetObjectItem(root, issuestructNode->Node);
        type = cJSON_GetObjectItem(category, issuestructNode->subNode);
    }
    else
    {
        category = cJSON_GetObjectItem(jsoncfg, issuestructNode->Node);
        type = cJSON_GetObjectItem(category, issuestructNode->subNode);
    }
    free(issuestructNode->Node); // free main node
    free(issuestructNode->subNode); // free sub node
    issuestdata = (issueData *) malloc(sizeof(issueData));
    if(issuestdata == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Memory Allocation Failure \n",__FUNCTION__,__LINE__);
	return false;
    }
    issuestdata->rfcvalue = NULL;
    issuestdata->command = NULL;
    issuestdata->timeout = 0;

    /* Read Command and Timeout information for Issuetype */
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Command and Timeout information for Debug Issue\n",__FUNCTION__,__LINE__);
    nitems = cJSON_GetArraySize(type);
    for(j = 0; j < nitems; j++)
    {
        elem = cJSON_GetArrayItem(type, j);
        if(elem && elem->type == cJSON_Number)
        {
            issuestdata->timeout = elem->valueint;  // copy timeout info from json file
        }
        else if(elem && elem->type == cJSON_String)
        {
	    tmpCommand = cJSON_Print(elem);
	    if(tmpCommand)
	    {
               issuestdata->command = strdup(tmpCommand);   // print command info from json file
               cJSON_free(tmpCommand);
	    }
        }
    }

    if(issuestdata->command == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: No Commands found, exiting.. \n",__FUNCTION__,__LINE__);
        free(issuestdata);
    }
    else
    {
        sanity = cJSON_GetObjectItem(jsoncfg, "Sanity");
        check = cJSON_GetObjectItem(sanity, "Check");
        checkval = cJSON_Print(check);  // Print list of sanity commands received
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Sanity Check Commands List: \n%s\n",__FUNCTION__,__LINE__,checkval);
	if(checkval ==NULL)
	{
	    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Entering checkval null case : \n",__FUNCTION__,__LINE__);
            jsoncfg = readAndParseJSON(RRD_JSON_FILE);
	    sanity = cJSON_GetObjectItem(jsoncfg, "Sanity");
	    check = cJSON_GetObjectItem(sanity, "Check");
            checkval = cJSON_Print(check);  // Print list of sanity commands received
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Sanity Check Commands List: \n%s\n",__FUNCTION__,__LINE__,checkval);
        }
        cmdlist = cJSON_GetObjectItem(check, "Commands");
        cJSON_free(checkval); // free sanity command list
        ret = isCommandsValid(issuestdata->command, cmdlist);
        if(ret != 0)
        {
            RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Aborting Command execution due to Harmful commands!!!\n",__FUNCTION__,__LINE__);
            free(issuestdata->command);
            free(issuestdata);
        }
        else
        { 
            issuestdata->rfcvalue = strdup(buf);
            if(issuestdata->timeout == 0)
            {
                issuestdata->timeout = DEFAULT_TIMEOUT; // Use Default Systemd service timeout of 90 seconds
                RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Using default timeout of %d seconds\n",__FUNCTION__,__LINE__,issuestdata->timeout);
            }
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Value of rfcvalue: %s command: %s time: %d\n",__FUNCTION__,__LINE__, issuestdata->rfcvalue,issuestdata->command,issuestdata->timeout);
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Executing Commands in Runtime Service... \n",__FUNCTION__,__LINE__);
            exresult = executeCommands(issuestdata);
        }
    }

    return exresult;
}

/*
 * @function checkIssueNodeInfo
 * @brief Checks the node and calls command execution function based on sub-node information.
 * @param issueNodeData *issuestructNode - Structure with issue category and type.
 * @param cJSON *jsoncfg - Parsed JSON content.
 * @param data_buf *buff - Data buffer containing the issue string.
 * @param bool isDeepSleepAwakeEventValid - Flag indicating if it is a deep sleep wake event.
 * @return void
 */
void checkIssueNodeInfo(issueNodeData *issuestructNode, cJSON *jsoncfg, data_buf *buff, bool isDeepSleepAwakeEventValid, issueData *appendprofiledata)
{
    int status = 0,dlen = 0;
    char *rfcbuf = NULL;
    bool execstatus;
    char outdir[BUF_LEN_256] =  {'\0'};
    time_t ctime;
    struct tm *ltime;
    rfcbuf = strdup(buff->mdata);

    // Creating Directory for MainNode under /tmp/rrd Folder
    ctime = time (NULL);
    ltime = localtime (&ctime);
    dlen=snprintf(outdir,BUF_LEN_256,"%s%s-DebugReport-",RRD_OUTPUT_DIR,issuestructNode->Node);
    strftime (outdir + dlen, sizeof(outdir) - dlen, "%Y-%m-%d-%H-%M-%S", ltime);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Creating Directory %s for Issue Category to store Output data...\n",__FUNCTION__,__LINE__,outdir);
    if (mkdir(outdir,0777) != 0)
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: %s Directory creation failed!!!\n",__FUNCTION__,__LINE__,outdir);
        free(buff->mdata); // free rfc data
        free(buff->jsonPath); // free rrd path info
        return;
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Change directory %s\n",__FUNCTION__,__LINE__,outdir);
        if(chdir(outdir) == 0) /* Change Directory Success */
	{
            if (issuestructNode->subNode != NULL)
            {
                // Execute the command for received Issue Type of the Issue Category
                RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d] Run Debug Commands for %s:%s \n",__FUNCTION__,__LINE__,issuestructNode->Node,issuestructNode->subNode);
                // Execute the command for received Issue Type of the Issue Category
                if (buff->appendMode)
                {
                    execstatus = executeCommands(appendprofiledata);
                    free(issuestructNode->Node); // free main node
                    free(issuestructNode->subNode); // free sub node
                }
                else
                {
                    execstatus = invokeSanityandCommandExec(issuestructNode, jsoncfg, rfcbuf, false);
                }		
            }
            else if(isDeepSleepAwakeEventValid)
            {
                execstatus = processAllDeepSleepAwkMetricsCommands(jsoncfg, issuestructNode, rfcbuf);
            }
            else
            {
                execstatus = processAllDebugCommand(jsoncfg, issuestructNode, rfcbuf);
            }

            // Invoke Upload Script to perform S3 Log upload
            if (!execstatus)
            {
                RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Skip uploading Debug Report!!! \n",__FUNCTION__,__LINE__);
            }
            else
            {
                RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Continue uploading Debug Report for %s from %s... \n",__FUNCTION__,__LINE__,buff->mdata,outdir);
                status = uploadDebugoutput(outdir,buff->mdata);
                if(status != 0)
                {
                    RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: RRD Upload Script Execution Failed!!! status:%d\n",__FUNCTION__,__LINE__,status);
                }
                else
                {
                    RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: RRD Upload Script Execution Success...\n",__FUNCTION__,__LINE__);
                }
            }
            free(buff->mdata); // free rfc data
            free(buff->jsonPath); // free rrd path info
	}
	else
	{
            RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: No Command excuted as RRD Failed to change directory:%s\n",__FUNCTION__,__LINE__,outdir);
	}
    }
}

/*
 * @function processAllDebugCommand
 * @brief Processes all the sub-node commands for the issued command.
 * @param cJSON *jsoncfg - Parsed JSON content.
 * @param issueNodeData *issuestructNode - Structure with issue category and type.
 * @param char *rfcbuf - RFC buffer.
 * @return bool - Returns true for successful execution and false for failure.
 */
bool processAllDebugCommand(cJSON *jsoncfg, issueNodeData *issuestructNode, char *rfcbuf)
{
    cJSON *mainnode = NULL;
    char *mainnodename = NULL;
    int subitems = 0, i = 0;
    bool execstatus = false;
    int rfcDelimLen = strlen(RFC_DELIM);
    int length = 0;
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d] Run Debug Commands for all issue types in  %s \n", __FUNCTION__, __LINE__, issuestructNode->Node);
    // Execute the commands for all Sub Issue Types of the Issue Category
    mainnode = cJSON_GetObjectItem(jsoncfg, issuestructNode->Node); // Read Node information from JSON
    if(mainnode == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:Did not found main node IssueType:[%s] \n", __FUNCTION__, __LINE__,issuestructNode->Node);
	return false;
    }
    mainnodename = cJSON_Print(mainnode);
    if(mainnodename == NULL)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:Did not found main node IssueType:[%s] \n", __FUNCTION__, __LINE__,issuestructNode->Node);
	return false;
    }
    subitems = cJSON_GetArraySize(mainnode);
    if (subitems != 0)
    {
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]*********************************************\n", __FUNCTION__, __LINE__);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Total Debug Issues Found in JSON file for %s are %d\n", __FUNCTION__, __LINE__, rfcbuf, subitems);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]*********************************************\n", __FUNCTION__, __LINE__);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Issue Category:%s:\n%s \n", __FUNCTION__, __LINE__, mainnode->string, mainnodename);
        RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]*********************************************\n", __FUNCTION__, __LINE__);
        cJSON_free(mainnodename); // free main node
        cJSON *issuearray[subitems];
        char *issuetypearray[subitems];
        for (i = 0; i < subitems; i++)
        {
            length = 0;
            issuearray[i] = cJSON_GetArrayItem(mainnode, i);
	    if(issuearray[i])
	    {
                length = strlen(rfcbuf) + rfcDelimLen + strlen(issuearray[i]->string) + 1;
                issuetypearray[i] = (char *)malloc(length * sizeof(char));
                 memset(issuetypearray[i],'\0',length);
	         if(issuetypearray[i])
	         {
                     strncpy(issuetypearray[i], rfcbuf, strlen(rfcbuf));
		     /* Wstringop-overflow : strncat is to provide the length of the source argument rather than the remaining space in the destination.
		      * Fix is to call the functions with the remaining space in the destination, with room for the terminating null byte */
		     strncat(issuetypearray[i], RFC_DELIM, rfcDelimLen + 1);
                     strncat(issuetypearray[i], issuearray[i]->string, strlen(issuearray[i]->string));
                     if (i != 0)
                     {
                         issuestructNode->Node = strdup(rfcbuf);
                     }
                     issuestructNode->subNode = strdup(issuearray[i]->string);

                     RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]##################################################\n", __FUNCTION__, __LINE__);
                     RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Issue Type %d:%s\n", __FUNCTION__, __LINE__, i + 1, issuetypearray[i]);
                     RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Issue Type %s:%s\n", __FUNCTION__, __LINE__, issuestructNode->Node, issuestructNode->subNode);
                     RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]##################################################\n", __FUNCTION__, __LINE__);
                     execstatus = invokeSanityandCommandExec(issuestructNode, jsoncfg, issuetypearray[i], false);
                     free(issuetypearray[i]);
	         }
	    }
        }
        free(rfcbuf); // free rfc value
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:No Debug Issues found in JSON file!!! \n", __FUNCTION__, __LINE__);
    }
    return execstatus;
}

/*
 * @function processAllDeepSleepAwkMetricsCommands
 * @brief Processes all the commands for deep sleep metrics.
 * @param cJSON *jsoncfg - Parsed JSON content.
 * @param issueNodeData *issuestructNode - Structure with issue category and type.
 * @param char *rfcbuf - RFC buffer.
 * @return bool - Returns true for successful execution and false for failure.
 */
bool processAllDeepSleepAwkMetricsCommands(cJSON *jsoncfg, issueNodeData *issuestructNode, char *rfcbuf)
{
    cJSON *rootNode = NULL;
    char *rootNodeName = NULL;
    int issueTypeCount = 0, _sindex = 0;
    int issueCategoryCount = 0, _mindex = 0;
    bool execstatus = false;
    int rfcDelimLen = strlen(RFC_DELIM), issueTypeLen = 0, issueCategoryLen = 0;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d] Run Debug Commands for all issue types in  %s \n", __FUNCTION__, __LINE__, issuestructNode->Node);
    // Execute the commands for all Sub Issue Types of the Issue Category
    rootNode = cJSON_GetObjectItem(jsoncfg, issuestructNode->Node); // Read Node information from JSON
    rootNodeName = cJSON_Print(rootNode);
    issueCategoryCount = cJSON_GetArraySize(rootNode);

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d] Printing RootNode name %s \n", __FUNCTION__, __LINE__, rootNodeName);
    free(issuestructNode->Node); // Deep Sleep String not required.

    if (issueCategoryCount)
    {
        cJSON *issueCategoryNode[issueCategoryCount];
        for (_mindex = 0; _mindex < issueCategoryCount; _mindex++)
        {
        issueCategoryNode[_mindex] = cJSON_GetArrayItem(rootNode, _mindex);

        issueTypeCount = cJSON_GetArraySize(issueCategoryNode[_mindex]);
        if (issueTypeCount)
        {
                cJSON *issueTypeNode[issueTypeCount];
                char *issuedCommand[issueTypeCount];

                for (_sindex = 0; _sindex < issueTypeCount; _sindex++)
                {
                    issueTypeNode[_sindex] = cJSON_GetArrayItem(issueCategoryNode[_mindex], _sindex);
                    issueTypeLen = strlen(issueCategoryNode[_mindex]->string);
                    issueCategoryLen = strlen(issueTypeNode[_sindex]->string);
                    issuedCommand[_sindex] = (char *)malloc(issueTypeLen + issueCategoryLen + rfcDelimLen + 1);
                    if (issuedCommand[_sindex] == NULL)
                    {
                        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:Memory Allocation Failed \n", __FUNCTION__, __LINE__);
                        return false;
                    }
                    memset(issuedCommand[_sindex], '\0', (issueTypeLen + issueCategoryLen + rfcDelimLen + 1));
                    strncpy(issuedCommand[_sindex], issueCategoryNode[_mindex]->string, issueCategoryLen);
		    /* Wstringop-overflow: strncat is to provide the length of the source argument rather than the remaining space in the destination.
		     * Fix is to call the functions with the remaining space in the destination, with room for the terminating null byte */
		    strncat(issuedCommand[_sindex], RFC_DELIM, rfcDelimLen + 1);
                    strncat(issuedCommand[_sindex], issueTypeNode[_sindex]->string, issueTypeLen);

                    issuestructNode->Node = strdup(issueCategoryNode[_mindex]->string);
                    issuestructNode->subNode = strdup(issueTypeNode[_sindex]->string);

                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]##################################################\n", __FUNCTION__, __LINE__);
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Issue Type %d:%s\n", __FUNCTION__, __LINE__, _sindex + 1, issuedCommand[_sindex]);
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]: Reading Issue Type %s:%s\n", __FUNCTION__, __LINE__, issuestructNode->Node, issuestructNode->subNode);
                    RDK_LOG(RDK_LOG_INFO, LOG_REMDEBUG, "[%s:%d]##################################################\n", __FUNCTION__, __LINE__);
                    execstatus = invokeSanityandCommandExec(issuestructNode, jsoncfg, issuedCommand[_sindex], true);
                    free(issuedCommand[_sindex]);
                }
        }
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:No Debug Issues found in JSON file!!! \n", __FUNCTION__, __LINE__);
    }
    return execstatus;
}

/*
 * @function RRDStoreDeviceInfo
 * @brief Stores platform information.
 * @param devicePropertiesData *devPropData - Structure to store device properties.
 * @return void
 */
void RRDStoreDeviceInfo(devicePropertiesData *devPropData)
{
    FILE *fp = NULL;
    char *line = NULL;
    size_t length = 0;
    char *valueString = NULL;
    char *FileDeviceProperties = RRD_DEVICE_PROP_FILE;
   
    /* Initialized Device property to default*/
    devPropData->deviceType = NULL; 
    
    /* Open Device Property File*/ 
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading File %s \n",__FUNCTION__,__LINE__,FileDeviceProperties);
    fp = fopen(FileDeviceProperties, "r");
    if (fp)
    {
	    /* Get Line from File */
        while(getline(&line, &length, fp) !=-1)
        {
            if (strncmp(line, "DEVICE_NAME=", strlen("DEVICE_NAME=")) == 0) {
                /* Get device value and validate*/
                valueString = strtok(line + strlen("DEVICE_NAME="), "\n");
                if (valueString)
                {
                    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Read Device property %s\n",__FUNCTION__,__LINE__,valueString);
                    removeSpecialChar(valueString);

                    // Allocate memory for deviceType and copy the value
                    size_t len = strlen(valueString) + 1;
                    devPropData->deviceType = (char *)calloc(len, 1);
                    if (devPropData->deviceType != NULL)
                    {
                        snprintf(devPropData->deviceType, len, "%s", valueString);
                    }
                    else
                    {
                        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory allocation failed for deviceType\n", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Read Device property is Empty\n", __FUNCTION__, __LINE__);
                    
                    // If valueString is NULL, assign an empty string
                    devPropData->deviceType = (char *)calloc(1, 1);  // Allocate space for empty string
                    if (devPropData->deviceType == NULL)
                    {
                        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory allocation failed for empty deviceType\n", __FUNCTION__, __LINE__);
                    }
                }
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Stored Device property for %s\n", __FUNCTION__, __LINE__, devPropData->deviceType);
                break; // Break after processing the first DEVICE_NAME entry
            }
        }
        fclose(fp);
        if(line)
        {
            free(line);
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: File %s Read Failed!!! \n",__FUNCTION__,__LINE__, FileDeviceProperties);
    }
}

/*
 * @function lookupRrdProfileList
 * @brief function checks if the provided profile string is in the list profiles.
 * @param const char* profile - The profile string to validate
 * @return true if the profile is in the list of profiles, false otherwise.
 */
bool lookupRrdProfileList(const char *profile)
{
    if (profile == NULL || strlen(profile) == 0) 
    {
        return false;
    }

    char *profiles = strdup(RRD_DEVICE_PROFILE);
    if(profiles ==  NULL)
    {
        return false;
    }

    char *token = strtok(profiles, ",");
    while(token != NULL)
    {
        if(strcmp(token, profile) == 0)
        {
            free(profiles);
            return true;
        }
        token = strtok(NULL, ",");
    }
    free(profiles);
    return false;
}

/*
 * @function getRrdProfileName
 * @brief Retrieves checks if the current device type stored in devPropData
 * is a valid RRD profile name. If it is, the device type is returned;
 * otherwise, an empty string is returned.
 * @param devicePropertiesData *devPropData - Structure to store device properties.
 * @return char* - The profile name if valid, or an empty string if not
 */
const char* getRrdProfileName(devicePropertiesData *devPropData) {
    if (lookupRrdProfileList(devPropData->deviceType))
    {
        return devPropData->deviceType;
    }
    return "";
}
