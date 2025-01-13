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

#include <errno.h>
#include <string.h>
#include <msgpack.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include "rrdMsgPackDecoder.h"
#include "rrdCommon.h"

/*
 * @function __finder
 * @brief Searches for a key in a msgpack map object and returns the value if the expected type matches.
 * @param const char *name - The key name to search for.
 * @param msgpack_object_type expect_type - The expected type of the value.
 * @param msgpack_object_map *map - Pointer to the msgpack map object.
 * @return msgpack_object* - Pointer to the value if found, or NULL if not found.
 */
msgpack_object *__finder(const char *name,
                         msgpack_object_type expect_type,
                         msgpack_object_map *map)
{
    uint32_t i;

    for (i = 0; i < map->size; i++)
    {
        if (MSGPACK_OBJECT_STR == map->ptr[i].key.type)
        {
            if (expect_type == map->ptr[i].val.type)
            {
                if (0 == match(&(map->ptr[i]), name))
                {
                    return &map->ptr[i].val;
                }
            }
            else if (MSGPACK_OBJECT_STR == map->ptr[i].val.type)
            {
                if (0 == strncmp(map->ptr[i].key.via.str.ptr, name, strlen(name)))
                {
                    return &map->ptr[i].val;
                }
            }
            else
            {
                if (0 == strncmp(map->ptr[i].key.via.str.ptr, name, strlen(name)))
                {
                    return &map->ptr[i].val;
                }
            }
        }
    }
    errno = MISSING_ENTRY;
    return NULL;
}

/*
 * @function helper_convert
 * @brief Converts a buffer into a structured format using msgpack, with optional processing and destruction functions.
 * @param const void *buf - The input buffer to convert.
 * @param size_t len - Length of the input buffer.
 * @param size_t struct_size - The size of the structure to allocate.
 * @param const char *wrapper - The expected wrapper name.
 * @param msgpack_object_type expect_type - The expected type of the wrapper.
 * @param bool optional - Whether the wrapper is optional.
 * @param process_fn_t process - The processing function to apply to the structure.
 * @param destroy_fn_t destroy - The destruction function to apply to the structure on failure.
 * @return void* - Pointer to the allocated structure, or NULL on failure.
 */
void *helper_convert(const void *buf, size_t len,
                     size_t struct_size, const char *wrapper,
                     msgpack_object_type expect_type, bool optional,
                     process_fn_t process,
                     destroy_fn_t destroy)
{
    void *p = malloc(struct_size);

    if (NULL == p)
    {
        errno = OUT_OF_MEMORY;
    }
    else
    {
        memset(p, 0, struct_size);

        if (NULL != buf && 0 < len)
        {
            size_t offset = 0;
            msgpack_unpacked msg;
            msgpack_unpack_return mp_rv;

            msgpack_unpacked_init(&msg);

            /* The outermost wrapper MUST be a map. */
            mp_rv = msgpack_unpack_next(&msg, (const char *)buf, len, &offset);

            if ((MSGPACK_UNPACK_SUCCESS == mp_rv) && (0 != offset) &&
                (MSGPACK_OBJECT_MAP == msg.data.type))
            {
                msgpack_object *inner;
                msgpack_object *subdoc_name;
                msgpack_object *version;
                msgpack_object *transaction_id;
                if (NULL != wrapper && 0 == strncmp(wrapper, "parameters", strlen("parameters")))
                {
                    inner = __finder(wrapper, expect_type, &msg.data.via.map);
                    if (((NULL != inner) && (0 == (process)(p, 1, inner))) ||
                        ((true == optional) && (NULL == inner)))
                    {
                        msgpack_unpacked_destroy(&msg);
                        errno = OK;
                        return p;
                    }
                    else
                    {
                        errno = INVALID_FIRST_ELEMENT;
                    }
                }
                else if (NULL != wrapper && 0 != strcmp(wrapper, "parameters"))
                {
                    inner = __finder(wrapper, expect_type, &msg.data.via.map);
                    subdoc_name = __finder("subdoc_name", expect_type, &msg.data.via.map);
                    version = __finder("version", expect_type, &msg.data.via.map);
                    transaction_id = __finder("transaction_id", expect_type, &msg.data.via.map);
                    if (((NULL != inner) && (0 == (process)(p, 4, inner, subdoc_name, version, transaction_id))) ||
                        ((true == optional) && (NULL == inner)))
                    {
                        msgpack_unpacked_destroy(&msg);
                        errno = OK;
                        return p;
                    }
                    else
                    {
                        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:Invalid first element\n", __FUNCTION__, __LINE__);
                        errno = INVALID_FIRST_ELEMENT;
                    }
                }
            }
            msgpack_unpacked_destroy(&msg);
            if (NULL != p)
            {
                (destroy)(p);
                p = NULL;
            }
        }
    }
    return p;
}

/*
 * @function get_msgpack_unpack_status
 * @brief Unpacks a msgpack encoded buffer and returns the unpack status.
 * @param char *decodedbuf - The decoded buffer to unpack.
 * @param int size - Size of the decoded buffer.
 * @return msgpack_unpack_return - The unpack status.
 */

msgpack_unpack_return get_msgpack_unpack_status(char *decodedbuf, int size)
{

    msgpack_zone mempool;
    msgpack_object deserialized;
    msgpack_unpack_return unpack_ret;

    if (decodedbuf == NULL || !size)
        return MSGPACK_UNPACK_NOMEM_ERROR;

    msgpack_zone_init(&mempool, 2048);
    unpack_ret = msgpack_unpack(decodedbuf, size, NULL, &mempool, &deserialized);

        switch(unpack_ret)
        {
            case MSGPACK_UNPACK_SUCCESS:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: MSGPACK_UNPACK_SUCCESS\n", __FUNCTION__, __LINE__);
                break;
            case MSGPACK_UNPACK_EXTRA_BYTES:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: MSGPACK_UNPACK_EXTRA_BYTES\n", __FUNCTION__, __LINE__);
                break;
            case MSGPACK_UNPACK_CONTINUE:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: MSGPACK_UNPACK_CONTINUE\n", __FUNCTION__, __LINE__);
                break;
            case MSGPACK_UNPACK_PARSE_ERROR:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: MSGPACK_UNPACK_PARSE_ERROR\n", __FUNCTION__, __LINE__);
                break;
            case MSGPACK_UNPACK_NOMEM_ERROR:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]:MSGPACK_UNPACK_NOMEM_ERROR\n", __FUNCTION__, __LINE__);
                break;
            default:
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Message Pack decode failed with error\n", __FUNCTION__, __LINE__);
        }
    
    msgpack_zone_destroy(&mempool);
    // End of msgpack decoding

    return unpack_ret;
}

/*
 * @function remotedebuggerdoc_convert
 * @brief Converts a buffer into a remotedebuggerdoc_t structure using msgpack.
 * @param const void *buf - The input buffer to convert.
 * @param size_t len - Length of the input buffer.
 * @return remotedebuggerdoc_t* - Pointer to the allocated remotedebuggerdoc_t structure, or NULL on failure.
 */
/* See remotedebuggerdoc.h for details. */
remotedebuggerdoc_t *remotedebuggerdoc_convert(const void *buf, size_t len)
{
#if GTEST_ENABLE
        return (remotedebuggerdoc_t *) helper_convert(buf, len, sizeof(remotedebuggerdoc_t), "remotedebugger", MSGPACK_OBJECT_MAP, true, (process_fn_t)process_remotedebuggerdoc, (destroy_fn_t)remotedebuggerdoc_destroy);
#else
        return helper_convert(buf, len, sizeof(remotedebuggerdoc_t), "remotedebugger", MSGPACK_OBJECT_MAP, true, (process_fn_t)process_remotedebuggerdoc, (destroy_fn_t)remotedebuggerdoc_destroy);
#endif
}

/*
 * @function remotedebuggerdoc_destroy
 * @brief Frees the memory allocated for a remotedebuggerdoc_t structure.
 * @param remotedebuggerdoc_t *ld - Pointer to the remotedebuggerdoc_t structure to free.
 * @return void
 */
void remotedebuggerdoc_destroy(remotedebuggerdoc_t *ld)
{
    if (NULL != ld)
    {
        if (NULL != ld->param)
        {
            if (NULL != ld->param->commandList)
            {
                free(ld->param->commandList);
            }
            free(ld->param);
        }
        if (NULL != ld->subdoc_name)
        {
            free(ld->subdoc_name);
        }
        free(ld);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Remote Debugger SubDoc Destroyed. \n", __FUNCTION__, __LINE__);
    }
}

/*
 * @function remotedebuggerdoc_strerror
 * @brief Returns a string description of the given error number.
 * @param int errnum - The error number.
 * @return const char* - The string description of the error.
 */
const char *remotedebuggerdoc_strerror(int errnum)
{
    struct error_map
    {
        int v;
        const char *txt;
    } map[] = {
        {.v = OK, .txt = "No errors."},
        {.v = OUT_OF_MEMORY, .txt = "Out of memory."},
        {.v = INVALID_FIRST_ELEMENT, .txt = "Invalid first element."},
        {.v = INVALID_VERSION, .txt = "Invalid 'version' value."},
        {.v = INVALID_OBJECT, .txt = "Invalid 'value' array."},
        {.v = 0, .txt = NULL}};
    int i = 0;
    while ((map[i].v != errnum) && (NULL != map[i].txt))
    {
        i++;
    }
    if (NULL == map[i].txt)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ----remotedebuggerdoc_strerror---- \n", __FUNCTION__, __LINE__);
        return "Unknown error.";
    }
    return map[i].txt;
}

/*
 * @function getReqTotalSizeOfIssueTypesStr
 * @brief Calculates the required size of the issue type string from a msgpack array.
 * @param msgpack_object_array *arr - The msgpack array containing the issue types.
 * @return int - The required size of the issue type string.
 */
int getReqTotalSizeOfIssueTypesStr(msgpack_object_array *arr)
{
    int index = 0, size = 0;
    for(index = 0; index < arr->size; index++)
    {
        if(arr->ptr[index].type == MSGPACK_OBJECT_STR)
        {
            size += arr->ptr[index].via.str.size;
        }
    }
    return (size + arr->size + 1); /* Need to add Number of Required "," between each string*/
}

/*
 * @function process_remotedebuggerparams
 * @brief Processes the msgpack map into a remotedebuggerparam_t structure.
 * @param remotedebuggerparam_t *rrdParam - Pointer to the remotedebuggerparam_t structure to populate.
 * @param msgpack_object_map *map - Pointer to the msgpack map object.
 * @return int - Returns 0 on success, or an error code otherwise.
 */
int process_remotedebuggerparams(remotedebuggerparam_t *rrdParam, msgpack_object_map *map)
{
    int left = map->size;
    uint8_t objects_left = 1;
    int index = 0, resultLen = 0;
    msgpack_object_kv *p = NULL;
    msgpack_object_array *arr = NULL;
    p = map->ptr;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);
    while ((0 < objects_left) && (0 < left--))
    {
        if ((MSGPACK_OBJECT_STR == p->key.type) && (MSGPACK_OBJECT_ARRAY == p->val.type))
        {
            if (0 == match(p, "IssueType"))
            {
                arr = &p->val.via.array;
                resultLen = getReqTotalSizeOfIssueTypesStr(arr);
                RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Size:%d \n", __FUNCTION__, __LINE__,resultLen);
                rrdParam->length = resultLen;
                rrdParam->commandList = (char *)malloc(resultLen * sizeof(char));
                memset(rrdParam->commandList, '\0', resultLen * sizeof(char));
                if(rrdParam->commandList != NULL)
                {
                    for(index = 0; index < arr->size; index++)
                    {
                        if(arr->ptr[index].type == MSGPACK_OBJECT_STR)
                        {
                            strncat(rrdParam->commandList, arr->ptr[index].via.str.ptr, arr->ptr[index].via.str.size);
                            if(index < (arr->size - 1))
                            {
                                strcat(rrdParam->commandList,",");
                            }
                         }
                      }
                      objects_left--;
                      rrdParam->commandList[resultLen]='\0';
                      RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Decode Result String: [%s]\n", __FUNCTION__, __LINE__,rrdParam->commandList);
                 }
                 else
                 {
                     RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed\n", __FUNCTION__, __LINE__);
                     break;
                  }
            }
        }
        p++;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return (0 == objects_left) ? 0 : -1;
}

/*
 * @function process_remotedebuggerdoc
 * @brief Processes the msgpack map into a remotedebuggerdoc_t structure.
 * @param remotedebuggerdoc_t *ld - Pointer to the remotedebuggerdoc_t structure to populate.
 * @param int num - Number of additional arguments.
 * @param ... - Additional arguments for variable argument processing.
 * @return int - Returns 0 on success, or an error code otherwise.
 */
int process_remotedebuggerdoc(remotedebuggerdoc_t *ld, int num, ...)
{
    // To access the variable arguments use va_list
    va_list valist;
    va_start(valist, num); // start of variable argument loop
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);

    msgpack_object *obj = va_arg(valist, msgpack_object *); // each usage of va_arg fn argument iterates by one time
    msgpack_object_map *mapobj = &obj->via.map;

    msgpack_object *obj1 = va_arg(valist, msgpack_object *);
    ld->subdoc_name = strndup(obj1->via.str.ptr, obj1->via.str.size);

    msgpack_object *obj2 = va_arg(valist, msgpack_object *);
    ld->version = (uint32_t)obj2->via.u64;

    msgpack_object *obj3 = va_arg(valist, msgpack_object *);
    ld->transaction_id = (uint16_t)obj3->via.u64;
    va_end(valist); // End of variable argument loop

    ld->entries_count = 1;
    ld->param = (remotedebuggerparam_t *)malloc(sizeof(remotedebuggerparam_t));
    if (NULL == ld->param)
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ld->param->commandList = NULL;
    if (0 != process_remotedebuggerparams(ld->param, mapobj))
    {
        RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]: Params Processing Failed.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);

    return 0;
}

/*
 * @function rollback_Debugger
 * @brief Placeholder function for rolling back debugger changes.
 * @param None.
 * @return int - Always returns 0.
 */
int rollback_Debugger()
{
    return 0;
}

/*
 * @function FreeResources_RemoteDebugger
 * @brief Frees the resources associated with the remote debugger.
 * @param void *arg - Pointer to the execData structure to free.
 * @return void
 */
void FreeResources_RemoteDebugger(void *arg)
{

    execData *blob_exec_data = (execData *)arg;
    if (!blob_exec_data)
        return;

    remotedebuggerdoc_t *premotedebuggerInfo = (remotedebuggerdoc_t *)blob_exec_data->user_data;

    if (premotedebuggerInfo != NULL)
    {
        remotedebuggerdoc_destroy(premotedebuggerInfo);
        premotedebuggerInfo = NULL;
    }

    if (blob_exec_data != NULL)
    {
        free(blob_exec_data);
        blob_exec_data = NULL;
    }
}

/*
 * @function Process_RemoteDebugger_WebConfigRequest
 * @brief Processes the WebConfig request for the remote debugger and prepares data to push to the message queue.
 * @param void *Data - Pointer to the data to process.
 * @return pErr - Returns a pointer to the error structure with the result of the processing.
 */
pErr Process_RemoteDebugger_WebConfigRequest(void *Data)
{
    pErr retStatus = NULL;
    int ret = 0;
    remotedebuggerdoc_t *premotedebuggerInfo = NULL;

    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering... \n", __FUNCTION__, __LINE__);

    retStatus = (pErr)malloc(sizeof(Err));
    if (retStatus == NULL)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId\n", __FUNCTION__, __LINE__);
        return retStatus;
    }
    memset(retStatus, 0, sizeof(Err));
    retStatus->ErrorCode = BLOB_EXEC_SUCCESS;

    premotedebuggerInfo = (remotedebuggerdoc_t *)Data;
    
    if (premotedebuggerInfo && premotedebuggerInfo->param && premotedebuggerInfo->param->commandList)
    {
        PrepareDataToPush(premotedebuggerInfo->param);
    }
    else
    {
        retStatus->ErrorCode = SYSCFG_FAILURE;
        strncpy(retStatus->ErrorMsg, "failure while applying remote debugger subdoc", sizeof(retStatus->ErrorMsg) - 1);
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);

    return retStatus;
}

/*
 * @function get_base64_decodedbuffer
 * @brief Decodes a base64 encoded string into a buffer.
 * @param char *pString - The base64 encoded string.
 * @param char **buffer - Pointer to the buffer to store the decoded data.
 * @param int *size - Pointer to the size of the decoded data.
 * @return int - Returns 0 on success, or -1 on failure.
 */
int get_base64_decodedbuffer(char *pString, char **buffer, int *size)
{
    int decodeMsgSize = 0;
    char *decodeMsg = NULL;
  
    if (buffer == NULL || size == NULL || pString == NULL)
        return -1;

    decodeMsgSize = b64_get_decoded_buffer_size(strlen(pString));
    decodeMsg = (char *)malloc(sizeof(char) * decodeMsgSize);

    if (!decodeMsg)
        return -1;

    *size = b64_decode((const uint8_t *)pString, strlen(pString), (uint8_t *)decodeMsg);
    *buffer = decodeMsg;

    return 0;

}

/*
 * @function PrepareDataToPush
 * @brief Prepares the data to push to the message queue for the remote debugger.
 * @param remotedebuggerparam_t *param - Pointer to the remotedebuggerparam_t structure containing the data.
 * @return void
 */
void PrepareDataToPush(remotedebuggerparam_t *param)
{
    int issueTypeListLen = param->length;
    char *commandList = param->commandList;
    char *issueTypeList = NULL;
    
    issueTypeList = (char *)malloc(issueTypeListLen * sizeof(char));
    if (issueTypeList == NULL)
    {
         RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Memory Allocation Failed for EventId\n", __FUNCTION__, __LINE__);
         return;
    }
    strncpy(issueTypeList, commandList, issueTypeListLen);
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Data Prepared for Message Queue\n", __FUNCTION__, __LINE__);
    pushIssueTypesToMsgQueue(issueTypeList, EVENT_MSG);
    return;
}

/*
 * @function decodeWebCfgData
 * @brief Decodes the WebConfig data and processes it for the remote debugger.
 * @param char *pString - The base64 encoded WebConfig data string.
 * @return int - Returns 0 on success, or -1 on failure.
 */
int decodeWebCfgData(char *pString)
{
    char *decodeMsg = NULL;
    int size = 0;
    int retval = 0;
    msgpack_unpack_return unpack_ret = MSGPACK_UNPACK_SUCCESS;
  
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Entering.. \n", __FUNCTION__, __LINE__);

    retval = get_base64_decodedbuffer(pString, &decodeMsg, &size);
    if (retval == 0)
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Base64 Decoding Successful...\n", __FUNCTION__, __LINE__);
        unpack_ret = get_msgpack_unpack_status(decodeMsg, size);
    }
    else
    {
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Base64 Decoding Failed...\n", __FUNCTION__, __LINE__);
        if (decodeMsg)
        {
            free(decodeMsg);
            decodeMsg = NULL;
        }
        return -1;
    }
    if (unpack_ret == MSGPACK_UNPACK_SUCCESS)
    {
        remotedebuggerdoc_t *premotedebuggerInfo = NULL;
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Calling remotedebuggerdoc_convert...\n", __FUNCTION__, __LINE__);
        premotedebuggerInfo = remotedebuggerdoc_convert(decodeMsg, size + 1);

        if (decodeMsg)
        {
            free(decodeMsg);
            decodeMsg = NULL;
        }
        if (premotedebuggerInfo == NULL)
        {
           RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Decoding Failed.\n", __FUNCTION__, __LINE__);
           return -1;
        }

        premotedebuggerInfo->entries_count = 1; // Assigned 1 by default.
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: entries_count=%u subdoc_name:%s version : %lu transaction_id:%u\n", __FUNCTION__, __LINE__,premotedebuggerInfo->entries_count, premotedebuggerInfo->subdoc_name, (unsigned long)premotedebuggerInfo->version, premotedebuggerInfo->transaction_id);
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: RRD WebCfg configuration received\n", __FUNCTION__, __LINE__);

        execData *execDataLan = NULL;

        execDataLan = (execData *)malloc(sizeof(execData));

        if (execDataLan != NULL)
        {
            memset(execDataLan, 0, sizeof(execData));
            /* Copy RRD Sub Doc Parameters*/
            execDataLan->txid = premotedebuggerInfo->transaction_id;
            execDataLan->version = premotedebuggerInfo->version;
            execDataLan->numOfEntries = premotedebuggerInfo->entries_count;
            /* Copy Sub Doc Name */
            strncpy(execDataLan->subdoc_name, "remotedebugger", sizeof(execDataLan->subdoc_name) - 1);

            execDataLan->user_data = (void *)premotedebuggerInfo;
            execDataLan->calcTimeout = NULL;
            execDataLan->executeBlobRequest = Process_RemoteDebugger_WebConfigRequest;
            execDataLan->rollbackFunc = rollback_Debugger;
            execDataLan->freeResources = FreeResources_RemoteDebugger;
            /* Push BloB Request to Web Config Framework */
            PushBlobRequest(execDataLan);

            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: PushBlobRequest complete\n", __FUNCTION__, __LINE__);
        }
        else
        {
            RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: execData memory allocation failed\n", __FUNCTION__, __LINE__);
            remotedebuggerdoc_destroy(premotedebuggerInfo);
            return -1;
        }
    }
    else
    {
        if (decodeMsg)
        {
            free(decodeMsg);
            decodeMsg = NULL;
        }
        RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: Corrupted StaticClientsData value\n", __FUNCTION__, __LINE__);
        return -1;
    }
    RDK_LOG(RDK_LOG_DEBUG, LOG_REMDEBUG, "[%s:%d]: ...Exiting...\n", __FUNCTION__, __LINE__);
    return 0;
}
