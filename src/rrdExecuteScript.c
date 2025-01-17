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

#include "rrdExecuteScript.h"
#if !defined(GTEST_ENABLE)
#define RRD_SCRIPT "/lib/rdk/uploadRRDLogs.sh"
#else
#define RRD_SCRIPT "./mockSampleUploadScript.sh"
#endif
#include "secure_wrapper.h"

static void normalizeIssueName(char *str);

/*
 * @function uploadDebugoutput
 * @brief Executes a script to perform tar and upload operations for the collected issue logs.
 * @param char *outdir - Output directory string.
 * @param char *issuename - Issue type from RFC.
 * @return int - Returns 0 for success and 1 for failure.
 */
int uploadDebugoutput(char *outdir, char *issuename)
{
    int ret = 0;

    if(outdir != NULL && issuename != NULL)
    {
        normalizeIssueName(issuename);
        RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Starting Upload Debug output Script: %s... \n",__FUNCTION__,__LINE__,RRD_SCRIPT);
        if(v_secure_system("%s %s %s",RRD_SCRIPT,outdir,issuename) != 0)
        {
            ret = 1;
        }
    }

    return ret;
}
	
/*
 * @function normalizeIssueName
 * @brief Replaces all occurrences of '.' with '_' in the given issue type string.
 * @param char *str - The issue type string to be normalized.
 * @return void
 */
static void normalizeIssueName(char *str)
{
    int len = strlen(str);
    int index = 0;

    for(index = 0; index < len; index++)
    {
        if(str[index] == '.')
        {
            str[index] = '_';
        }
    }
}
