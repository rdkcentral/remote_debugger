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

#include "rrdCommandSanity.h"
#include <ctype.h>

/*
 * @function updateBackgroundCmd
 * @brief Modifies the given command string by replacing instances of "&;" with "& "
 *              to handle commands specified for background execution properly.
 * @param char *str - The command string to be processed.
 * @return int - Returns 0 on success, or 1 if the input string is NULL.
 */
int updateBackgroundCmd(char * str)
{
    int i = 0;

    if(str == NULL)
    {
        return 1;
    }
    else
    {
        while(str[i]!='\0')
        {
            if(str[i]=='&' && str[i+1]==';') // replace '&;' in commands by '& '
            {
                str[i+1]=' ';
            }
            i++;
        }
    }

    return 0;
}

/*
 * @function replaceRRDLocation
 * @brief Replaces occurrences of the DEFAULT_DIR macro in the command string with the given
 *              directory location.
 * @param char* commandStr - The command string containing the macro to be replaced.
 * @param char* dirLocation - The directory location to replace the macro with.
 * @return char* - Returns the updated command string on success, or NULL on failure.
 */
char *replaceRRDLocation(char* commandStr, char* dirLocation)
{
    char *data = NULL;
    char *matchptr = NULL;
    char *iterptr = NULL;
    char *tempptr = NULL;
    size_t deflen = 0, dirlen = 0, cmdlen = 0;
    size_t datalen = 0,skiplen = 0;
    int cnt = 0;

    cmdlen = strlen(commandStr);
    deflen = strlen(DEFAULT_DIR);
    dirlen = strlen(dirLocation);

    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Command to be replaced: %s \n",__FUNCTION__,__LINE__,commandStr);
    iterptr = commandStr;
    while (NULL != (matchptr = strstr(iterptr, DEFAULT_DIR)))
    {
        cnt++;
        iterptr = matchptr + deflen;
    }

    datalen = cmdlen + cnt * (dirlen - deflen);
    data = (char *) malloc( sizeof(char) * (datalen + 1) ); // allocate memory for updating directory location for MACRO commands

    if (data != NULL)
    {
        tempptr = data;
        iterptr = commandStr;

        while (NULL != (matchptr = strstr(iterptr, DEFAULT_DIR)))
        {
            skiplen = matchptr - iterptr;
            strncpy(tempptr, iterptr, skiplen);
            tempptr += skiplen;
            strncpy(tempptr, dirLocation, dirlen);
            tempptr += dirlen;

            iterptr = matchptr + deflen;
        }
        strcpy(tempptr, iterptr);
    }
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Command to be executed: %s \n",__FUNCTION__,__LINE__,data);
    free(commandStr); // free old command received with MACRO
    commandStr = (char *) data; // Update new command to the argument
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Copying updated command to argument: %s \n",__FUNCTION__,__LINE__,commandStr);

    return commandStr;
}

/*
 * @function isCommandsValid
 * @brief Checks for harmful commands in the input command string against a list of
 *              sanctioned commands to prevent damage to platforms.
 * @param char *issuecmd - The command string to be validated.
 * @param cJSON *sanitylist - JSON object containing the list of sanctioned commands.
 * @return int - Returns 0 if the command is valid, or 1 if the command is harmful or invalid.
 */
int isCommandsValid(char *issuecmd,cJSON *sanitylist)
{
    int i = 0,result = 0,sitems = 0;
    cJSON *subcmd = NULL;
    char *sanitycmd = NULL;
    char *checkcmd = NULL;
    char *sanitystr = NULL;

    sanitycmd = cJSON_Print(sanitylist); // Print sanity commands data from the Json object sanitylist
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Reading Sanity Commands List: %s \n",__FUNCTION__,__LINE__,sanitycmd);
    cJSON_free(sanitycmd); // free sanity commands data
    sitems = cJSON_GetArraySize(sanitylist);
    for (i = 0 ; i < sitems; i++)
    {
         subcmd = cJSON_GetArrayItem(sanitylist, i);
         checkcmd = cJSON_Print(subcmd); // Print each command from the sanity command array in Json
         int len = strlen(checkcmd);
         // Ensure the string starts and ends with double quotes
         if (checkcmd[0] == '"' && checkcmd[len - 1] == '"') 
         {
             RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Performing operation on \"%s\" before comaring with issue_command : \"%s\" \n",__FUNCTION__,__LINE__,checkcmd, issuecmd);
             int i = len - 2; // Start from the second-last character
             while (i > 0 && isspace(checkcmd[i])) 
             {
                 checkcmd[i] = '"';  // Move the ending quote forward
                 checkcmd[i + 1] = '\0';
                 i--;
             }
         }
         RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Checking for \"%s\" string in Issue commands...issue_command : \"%s\" \n",__FUNCTION__,__LINE__,checkcmd, issuecmd);
         sanitystr = strstr(issuecmd,checkcmd);
         cJSON_free(checkcmd); // free each command from the sanity command array
         if (sanitystr)
         {
             RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Found harmful commands: %s, Exiting!!! \n",__FUNCTION__,__LINE__,sanitystr);
             return 1;
         }
         else
         {
             RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Found valid Commands, Execute... \n",__FUNCTION__,__LINE__);
         }
    }

    if(strstr(issuecmd,"&"))
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Received Commands to execute in background... \n",__FUNCTION__,__LINE__);
        result = updateBackgroundCmd(issuecmd);
        if (result != 0)
        {
            RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Removing special charater failed!!! \n",__FUNCTION__,__LINE__);
            return 1;
        }
    }

    return 0;
}
