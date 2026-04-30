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
#define RRD_SCRIPT "/lib/rdk/uploadRRDLogs.sh"
#if !defined(GTEST_ENABLE)
#include "secure_wrapper.h"
#endif

static void normalizeIssueName(char *str);

/*
 * @function uploadDebugoutput
 * @brief Calls the upload API to perform tar and upload operations for the collected issue logs.
 * @param char *outdir - Output directory string.
 * @param char *issuename - Issue type from RFC.
 * @return int - Returns 0 for success and non-zero for failure.
 */
// Modified to accept optional suffix for tar name
int uploadDebugoutput(char *outdir, char *issuename, const char *suffix)
{
    int ret = 0;

    if(outdir != NULL && issuename != NULL)
    {
        normalizeIssueName(issuename);

        // Compose tar name with suffix if provided
        char tarname[512];
        if (suffix && suffix[0] != '\0') {
            snprintf(tarname, sizeof(tarname), "%s%s.tar", issuename, suffix);
        } else {
            snprintf(tarname, sizeof(tarname), "%s.tar", issuename);
        }
#ifdef IARMBUS_SUPPORT
        RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Starting Upload Debug output via API... \n",__FUNCTION__,__LINE__);
        
        ret = rrd_upload_orchestrate(outdir, issuename);
        if(ret != 0)
        {
            RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]: Upload orchestration failed with code: %d\n",__FUNCTION__,__LINE__, ret);
        }
        else
        {
            RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Upload orchestration completed successfully\n",__FUNCTION__,__LINE__);
        }
#else
		RDK_LOG(RDK_LOG_INFO,LOG_REMDEBUG,"[%s:%d]: Starting Upload Debug output Script: %s... \n",__FUNCTION__,__LINE__,RRD_SCRIPT);
        if(v_secure_system("%s %s %s",RRD_SCRIPT,outdir,issuename) != 0)
		{
            ret = 1;
        }			
#endif
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
