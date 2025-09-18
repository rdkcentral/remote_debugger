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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "rrdRunCmdThread.h"
#include "rrdCommandSanity.h"
#if !defined(GTEST_ENABLE)
#include "secure_wrapper.h"
#endif

pthread_mutex_t rrdCacheMut;
static cacheData *cacheDataNode = NULL;

/*
 * @function initCache
 * @brief Initializes the mutex and the CacheDataNode.
 * @param None.
 * @return void
 */
void initCache(void)
{
    pthread_mutex_init(&rrdCacheMut, NULL);
    cacheDataNode = NULL;
}

/*
 * @function print_items
 * @brief Utility function to print all cache elements, including message data and issue strings.
 * @param cacheData *node - The cache node to start printing from.
 * @return void
 */
void print_items(cacheData *node)
{
    cacheData *rrdCachecnode = NULL;
    cacheData *cache = NULL;
    int i=0;
    i = pthread_mutex_lock(&rrdCacheMut);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: RRD Mutex Lock...%d\n",__FUNCTION__,__LINE__,i);
    rrdCachecnode = node;
    if(rrdCachecnode)
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: ------------------------------------------ \n",__FUNCTION__,__LINE__);
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Start of Print...\n",__FUNCTION__,__LINE__);
	while(rrdCachecnode != NULL)
	{
	    cache = rrdCachecnode;
	    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: %s ---- %s \n",__FUNCTION__,__LINE__,cache->mdata,cache->issueString);
	    rrdCachecnode = rrdCachecnode->next;
	}
	RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: End of Print...\n",__FUNCTION__,__LINE__);
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: ------------------------------------------ \n",__FUNCTION__,__LINE__);
   }
   else
   {
       RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Cache Empty...\n",__FUNCTION__,__LINE__);
   }
   pthread_mutex_unlock(&rrdCacheMut);
}

/*
 * @function createCache
 * @brief Creates a cache node with the provided message data and issue string.
 * @param char *pkgData - The package data to store in the cache node.
 * @param char *issueTypeData - The issue type data to store in the cache node.
 * @return cacheData* - Pointer to the created cache node, or NULL on failure.
 */
cacheData* createCache( char *pkgData, char *issueTypeData)
{
    cacheData *cache = NULL;
    cache = (cacheData *)malloc(sizeof(cacheData));
    /*Check if memory alloacted to Cache*/
    if(cache)
    {
        cache->mdata = NULL;
        cache->issueString = NULL;
	cache->next = NULL;
	cache->prev = NULL;
        cache->mdata = pkgData;
        cache->issueString = issueTypeData;
    }
    return cache;
}

/*
 * @function append_item
 * @brief Appends message data and issue string to the cache.
 * @param char *pkgData - The package data to append.
 * @param char *issueTypeData - The issue type data to append.
 * @return void
 */
void append_item(char *pkgData, char *issueTypeData)
{
    RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Append Item with PkgData: %s and issue Type: %s to Cache \n",__FUNCTION__,__LINE__,pkgData,issueTypeData);
	
    cacheData *rrdCachecnode = NULL;
    int i=0;
    i = pthread_mutex_lock(&rrdCacheMut);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: RRD Mutex Lock...%d\n",__FUNCTION__,__LINE__,i);
    
    cacheData *tmp = createCache(pkgData,issueTypeData);
    /* Check If the memory is allocated for new node*/ 
    if(!tmp)
    {
        RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Memory Allocation Failed : Cannot Append Item to Cache\n", __FUNCTION__, __LINE__);
        pthread_mutex_unlock(&rrdCacheMut);
	    return;
    }
    /* create Cache and store in node's data*/
    
    rrdCachecnode = cacheDataNode;
	
    /*Valid Cache, add node to list*/
    if(rrdCachecnode != NULL)
    {
        tmp->next = rrdCachecnode;
	rrdCachecnode->prev = tmp;
    }
    cacheDataNode = tmp;
    pthread_mutex_unlock(&rrdCacheMut);
}

/*
 * @function findPresentInCache
 * @brief Finds if the message data is already present in the cache.
 * @param char *pkgData - The package data to find in the cache.
 * @return cacheData* - Pointer to the cache node if found, or NULL if not found.
 */
cacheData* findPresentInCache(char *pkgData)
{
    RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: finding %s PkgData in Cache.. \n",__FUNCTION__,__LINE__,pkgData);
    cacheData *rrdCachecnode = NULL;
    cacheData *cache = NULL;
    int i=0;
	
    i = pthread_mutex_lock(&rrdCacheMut);
    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: RRD Mutex Lock...%d\n",__FUNCTION__,__LINE__,i);
    rrdCachecnode = cacheDataNode;
    while(rrdCachecnode != NULL)
    {    
	/*Check if pkgData is present in Cache*/
	if((rrdCachecnode->mdata) && (strcmp(rrdCachecnode->mdata, pkgData) == 0))
        {
	    cache = rrdCachecnode;
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Comparing %s & %s \n",__FUNCTION__,__LINE__,cache->mdata,pkgData);
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Issue String in Cache: %s \n",__FUNCTION__,__LINE__,cache->issueString);
            break;
        }
	rrdCachecnode = rrdCachecnode->next;
    }
    pthread_mutex_unlock(&rrdCacheMut);
    return cache;
}

/*
 * @function remove_item
 * @brief Removes an item with the specified message data and issue string from the cache.
 * @param cacheData *cache - The cache node to remove.
 * @return void
 */
void remove_item(cacheData *cache)
{
    cacheData *rrdCachecnode = NULL;
    cacheData *rrdCachenextnode = NULL;
    cacheData *rrdCacheprevnode = NULL;
    int i=0;
  
    if(cache)
    {
        i = pthread_mutex_lock(&rrdCacheMut);
	RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: RRD Mutex Lock...%d\n",__FUNCTION__,__LINE__,i);
        rrdCachecnode = cacheDataNode;
	if(rrdCachecnode == cache)
	{
	    cacheDataNode = rrdCachecnode->next;
	    if(rrdCachecnode->next)
	    {
	        rrdCachecnode->next->prev = NULL;
	    }
	}
	else
	{
            rrdCachenextnode = cache->next;
	    rrdCacheprevnode = cache->prev;
	    if(rrdCachenextnode)
	    {
	        rrdCachenextnode->prev = rrdCacheprevnode;
	    }
	    if(rrdCacheprevnode)
	    {
	        rrdCacheprevnode->next = rrdCachenextnode;
	    }
	}
	pthread_mutex_unlock(&rrdCacheMut);
        freecacheDataCacheNode(&cache);
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Cannot Delete Invalid Cache.\n",__FUNCTION__,__LINE__);
    } 
}

/*
 * @function freecacheDataCacheNode
 * @brief Frees the memory allocated for a cache node.
 * @param cacheData **node - Pointer to the cache node to free.
 * @return void
 */
void freecacheDataCacheNode(cacheData **node)
{
    cacheData *rrdCachetmpnode = *node;
    
    if(rrdCachetmpnode != NULL)
    {
        free(rrdCachetmpnode->mdata);
        free(rrdCachetmpnode->issueString);
        rrdCachetmpnode->mdata = NULL;
        rrdCachetmpnode->issueString = NULL;
        free(rrdCachetmpnode);
        *node = NULL;
    }
}

/*
 * @function removeQuotes
 * @brief Removes surrounding quotes from a string and replaces escaped quotes with regular quotes.
 * @param char* str - The string to process.
 * @return void
 */
void removeQuotes(char* str)
{
    if(str)
    {
        int len = strlen(str);
        // Check if the string has at least two characters and starts and ends with double quotes
        if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
        {
            memmove(str, str + 1, len - 2); // Move all characters one place to the left
            str[len - 2] = '\0'; // Null-terminate the string
        }

        // Replace escaped quotes (\") with regular quotes (")
        char* escaped = str;
        while ((escaped = strstr(escaped, "\\\"")))
        {
            memmove(escaped, escaped + 1, strlen(escaped));
        }
    }
}

/*
 * @function executeCommands
 * @brief Executes commands from different issue cases in parallel using multi-threading.
 * @param issueData *cmdinfo - Structure containing commands and timeout information.
 * @return bool - Returns true for success, false for failure.
 */
bool executeCommands(issueData *cmdinfo)
{
    int len = 0;
    pthread_t RRDCmdThreadID;
    time_t curtime;
    issueData *cmdData = NULL;
    struct tm *loc_time;
    char *result = NULL;
    char dirname[BUF_LEN_256] =  {'\0'};
    char pathname[BUF_LEN_256] = {'\0'};
    char *outdirpath = NULL;
    char finalOutFile[BUF_LEN_256] =  {'\0'};
    char remoteDebuggerServiceStr[BUF_LEN_256] =  {'\0'};
    char *printbuffer = NULL;
    FILE *filePointer;
    const char *remoteDebuggerPrefix = "remote_debugger_";
    int retval;

    cmdData = (issueData *)cmdinfo;
 
    if(cmdData->command)
    {
        curtime = time (NULL);
        loc_time = localtime (&curtime);
        RRDCmdThreadID = pthread_self();
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Running Thread %lu for execution of \n%s Issue Commands...\n",__FUNCTION__,__LINE__,RRDCmdThreadID,cmdData->rfcvalue);
        /*Creating Output Folder with Timestamp_Issue_Threadid*/
        len=snprintf(dirname,BUF_LEN_256,"%s_%lu_",cmdData->rfcvalue,RRDCmdThreadID);
        strftime (dirname + len, sizeof(dirname) - len, "%Y-%m-%d-%H-%M-%S", loc_time);
        RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Creating Directory %s for Storing Output data...\n",__FUNCTION__,__LINE__,dirname);
        if (mkdir(dirname,0777) != 0)
        {
           RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: %s Directory creation failed!!!\n",__FUNCTION__,__LINE__,dirname);
           free(cmdData->rfcvalue); // free rfcvalue received from RRDEventThreadFunc
           free(cmdData->command); // free command received from RRDEventThreadFunc
           free(cmdData); // free structure with command and time information
           return false;
        }
        else
        {
            /*Check for Output Directory MACRO in commands*/
            result = strstr(cmdData->command,"RRD_LOCATION");
            if(result != NULL)
            {
                getcwd(pathname, BUF_LEN_256);
                retval = asprintf(&outdirpath, "%s/%s",pathname,dirname);
                if(retval == -1)
                {
                    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Failed Setting outdirpath \n",__FUNCTION__,__LINE__);
		    cmdData->command = NULL;
                }
		else
                {
                    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Replacing default location %s with Event Specific Output Directory:%s \n",__FUNCTION__,__LINE__,result,outdirpath);
                    cmdData->command = replaceRRDLocation(cmdData->command,outdirpath);
		    free(outdirpath);
                    outdirpath = NULL;
                }

                if(cmdData->command == NULL)
                {
                    /* Fix for warning Wformat-overflow : directive argument is null */
                    RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Invalid Location found for command:\n",__FUNCTION__,__LINE__);
                    free(cmdData->rfcvalue); // free rfcvalue received from RRDEventThreadFunc
                    free(cmdData); // free structure with command and time information
                    return false;
                }
                else
                {
                    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Updated Command from replaceRRDLocation:%s \n",__FUNCTION__,__LINE__,cmdData->command);
                }
            }
            else
            {
                RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: No MACRO found, proceeding with commands:%s \n",__FUNCTION__,__LINE__,cmdData->command);
            }

            strncpy(finalOutFile, dirname, strlen(dirname) + 1);
            strncat(finalOutFile,"/", sizeof(finalOutFile) - strlen(finalOutFile) - 1);
            strncat(finalOutFile,RRD_OUTPUT_FILE, strlen(RRD_OUTPUT_FILE) + 1);

            /* Open debug_output.txt file*/
            filePointer = fopen(finalOutFile, "a+");
            if (filePointer == NULL)
            {
                RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Unable to Open File:%s\n",__FUNCTION__,__LINE__,finalOutFile);
                free(cmdData->rfcvalue); // free rfcvalue received from RRDEventThreadFunc
                free(cmdData->command); // free command received from RRDEventThreadFunc
                free(cmdData); // free structure with command and time information
                return false;
            }

            /*Printing Command Details into Output file*/
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: Adding Details of Debug commands to Output File: %s...\n",__FUNCTION__,__LINE__,RRD_OUTPUT_FILE);
            asprintf(&printbuffer, "Executing Debug Commands: \"%s\"",cmdData->command);
            fprintf(filePointer, "%s\n", printbuffer);
            RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]: PrintBuffer: %s\n",__FUNCTION__,__LINE__,printbuffer);
            free(printbuffer); // free echo message memory

            /*Executing Commands using systemd-run*/
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Executing following commands using systemd-run:\n \"%s\"\n",__FUNCTION__,__LINE__,cmdData->command);

            strncpy(remoteDebuggerServiceStr, remoteDebuggerPrefix, strlen(remoteDebuggerPrefix) + 1);
	    strncat(remoteDebuggerServiceStr, cmdData->rfcvalue, strlen(cmdData->rfcvalue));

	    removeQuotes(cmdData->command);
	    
	    FILE *systemdfp = v_secure_popen("r", "systemd-run -r --unit=%s --service-type=oneshot -p RemainAfterExit=yes /bin/sh -c %s", remoteDebuggerServiceStr, cmdData->command);
            if(systemdfp == NULL)
            {
                RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Starting remote_debugger_%s service failed!!!\n",__FUNCTION__,__LINE__,cmdData->rfcvalue);
            }
            else
            {
                RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Starting remote_debugger_%s service success...\n",__FUNCTION__,__LINE__,cmdData->rfcvalue);
		copyDebugLogDestFile(systemdfp, filePointer);
            }
            v_secure_pclose(systemdfp);

            /*Logging output using journalctl to Output file*/
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Using journalctl to log command output...\n",__FUNCTION__,__LINE__);
	    FILE *journalctlfp = v_secure_popen("r", "journalctl -a -u %s", remoteDebuggerServiceStr);
            if(journalctlfp == NULL)
            {
                RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: journalctl remote_debugger_%s service failed!!!\n",__FUNCTION__,__LINE__,cmdData->rfcvalue);
            }
            else
            {
                RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: journalctl remote_debugger_%s service success...\n",__FUNCTION__,__LINE__,cmdData->rfcvalue);
		copyDebugLogDestFile(journalctlfp, filePointer);
            }
            
	    v_secure_pclose(journalctlfp);

	    /* Close debug_output.txt file*/
	    fclose(filePointer);

            /*Sleeping before stopping service*/
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Sleeping with timeout %d sec before stopping service...\n",__FUNCTION__,__LINE__,cmdData->timeout);
            sleep(cmdData->timeout);

            /*Stop or Reset runtime service for issue*/
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Stopping remote_debugger_%s service...\n",__FUNCTION__,__LINE__,cmdData->rfcvalue);
#if !defined(GTEST_ENABLE)
	    v_secure_system("systemctl stop %s", remoteDebuggerServiceStr);
            free(cmdData->rfcvalue); // free rfcvalue received from RRDEventThreadFunc
            free(cmdData->command); // free updated command info received from RRDEventThreadFunc
            free(cmdData);
#endif
            return true;
        }
    }
    free(cmdData->rfcvalue); // free rfcvalue received from RRDEventThreadFunc
    free(cmdData);
    return false;
}

/*
 * @function copyDebugLogDestFile
 * @brief Copies the content from the source file stream to the destination file stream.
 * @param FILE *source - The source file stream.
 * @param FILE *destination - The destination file stream.
 * @return void
 */
void copyDebugLogDestFile(FILE *source, FILE *destination)
{
    char buffer[1024] = {'\0'};
    int bytesRead = 0;
    // Read from source and write to destination
    if((source != NULL) && (destination != NULL))
    {
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0)
        {
            fwrite(buffer, 1, bytesRead, destination);
        }
    }
}
