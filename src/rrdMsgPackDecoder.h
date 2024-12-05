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

#ifndef __RRD_WEBCONFIG_PARAM_H__
#define __RRD_WEBCONFIG_PARAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <msgpack.h>

#if !defined(GTEST_ENABLE)
#include <base64.h>
#include "webconfig_framework.h"
#endif

#define match(p, s) strncmp((p)->key.via.str.ptr, s, (p)->key.via.str.size)

typedef struct
{
    char *commandList;
    int length;    
} remotedebuggerparam_t;

typedef struct {
    remotedebuggerparam_t  *param;
    size_t       entries_count;
    char *       subdoc_name;
    uint32_t     version;
    uint16_t     transaction_id;
} remotedebuggerdoc_t;

enum
{    
    OK = 0,
    OUT_OF_MEMORY,
    INVALID_FIRST_ELEMENT,
    MISSING_ENTRY,
    INVALID_OBJECT,
    INVALID_VERSION,
};

remotedebuggerdoc_t* remotedebuggerdoc_convert( const void *buf, size_t len );
void remotedebuggerdoc_destroy( remotedebuggerdoc_t *d );
const char* remotedebuggerdoc_strerror( int errnum );
int process_remotedebuggerparams(remotedebuggerparam_t *e, msgpack_object_map *map);
int process_remotedebuggerdoc(remotedebuggerdoc_t *ld, int num, ...);
int get_base64_decodedbuffer(char *pString, char **buffer, int *size);

typedef int (*process_fn_t)(void *, int, ...);
typedef void (*destroy_fn_t)(void *);
void PrepareDataToPush(remotedebuggerparam_t *param);

#ifdef __cplusplus
}
#endif

#endif
