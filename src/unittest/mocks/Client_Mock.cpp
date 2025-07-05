/*
 * Copyright 2023 Comcast Cable Communications Management, LLC
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
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Client_Mock.h"
#include <gmock/gmock.h>
#include <cstdarg>

/* -------- IARM ---------------- */
#ifdef IARMBUS_SUPPORT
ClientIARMMock *g_mock = nullptr;

void setMock(ClientIARMMock *mock)
{
    g_mock = mock;
}

extern "C"
{
    IARM_Result_t IARM_Bus_Init(const char *name)
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_Init(name);
        }
        return IARM_RESULT_SUCCESS;
    }
    IARM_Result_t IARM_Bus_Connect()
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_Connect();
        }
        return IARM_RESULT_SUCCESS;
    }

    IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
        }
        return IARM_RESULT_SUCCESS;
    }
    IARM_Result_t IARM_Bus_Disconnect()
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_Disconnect();
        }
        return IARM_RESULT_SUCCESS;
    }
    IARM_Result_t IARM_Bus_Term()
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_Term();
        }
        return IARM_RESULT_SUCCESS;
    }
    IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId)
    {
        if (g_mock)
        {
            return g_mock->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
        }
        return IARM_RESULT_SUCCESS;
    }
}
#endif

/* ---------- RBUS --------------*/
RBusApiInterface *RBusApiWrapper::impl = nullptr;

RBusApiWrapper::RBusApiWrapper() {}

void RBusApiWrapper::setImpl(RBusApiInterface *newImpl)
{
    EXPECT_TRUE((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void RBusApiWrapper::clearImpl()
{
    impl = nullptr;
}

rbusError_t RBusApiWrapper::rbus_open(rbusHandle_t *handle, const char *componentName)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_open(handle, componentName);
}

rbusError_t RBusApiWrapper::rbus_close(rbusHandle_t handle)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_close(handle);
}

rbusError_t RBusApiWrapper::rbusValue_Init(rbusValue_t *value)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbusValue_Init(value);
}

rbusError_t RBusApiWrapper::rbusValue_SetString(rbusValue_t value, char const *str)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbusValue_SetString(value, str);
}

rbusError_t RBusApiWrapper::rbus_set(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_set(handle, objectName, value, respHandler);
}
rbusError_t RBusApiWrapper::rbus_get(rbusHandle_t handle, char const *objectName, rbusValue_t value, rbusMethodAsyncRespHandler_t respHandler)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_get(handle, objectName, value, respHandler);
}
const char* rbusError_ToString(rbusError_t e)
{
    #define rbusError_String(E, S) case E: s = S; break;

  char const * s = NULL;
  switch (e)
  {
    rbusError_String(RBUS_ERROR_SUCCESS, "ok");
    rbusError_String(RBUS_ERROR_BUS_ERROR, "generic error");
    rbusError_String(RBUS_ERROR_NOT_INITIALIZED, "not initialized");
    default:
      s = "unknown error";
  }
  return s;
}
rbusError_t (*rbus_open)(rbusHandle_t *, char const *) = &RBusApiWrapper::rbus_open;
rbusError_t (*rbus_close)(rbusHandle_t) = &RBusApiWrapper::rbus_close;
rbusError_t (*rbusValue_Init)(rbusValue_t *) = &RBusApiWrapper::rbusValue_Init;
rbusError_t (*rbusValue_SetString)(rbusValue_t, char const *) = &RBusApiWrapper::rbusValue_SetString;
rbusError_t (*rbus_set)(rbusHandle_t, char const *, rbusValue_t, rbusMethodAsyncRespHandler_t) = &RBusApiWrapper::rbus_set;

/* -------- RFC ---------------*/
SetParamInterface *SetParamWrapper::impl = nullptr;

SetParamWrapper::SetParamWrapper() {}

void SetParamWrapper::setImpl(SetParamInterface *newImpl)
{
    EXPECT_TRUE((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void SetParamWrapper::clearImpl()
{
    impl = nullptr;
}

tr181ErrorCode_t SetParamWrapper::setParam(char *arg1, const char *arg2, const char *arg3)
{
    EXPECT_NE(impl, nullptr);
    return impl->setParam(arg1, arg2, arg3);
}

tr181ErrorCode_t (*setParam)(char *, const char *, const char *) = &SetParamWrapper::setParam;

extern "C" int v_secure_system(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[2048];

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    std::string command = buffer;
    printf("command: %s\n", command.c_str());
    return system(command.c_str());
}

extern "C" FILE* v_secure_popen(const char *mode, ...)
{
    return NULL;
}

extern "C" int v_secure_pclose(FILE *fp)
{
    return 0;
}

/*------------- WebConfig ---------------*/
ClientWebConfigMock *g_webconfig_mock = nullptr;

void setWebConfigMock(ClientWebConfigMock *mock)
{
    g_webconfig_mock = mock;
}

extern "C"
{
    void register_sub_docs(blobRegInfo *bInfo, int numOfSubdocs, getVersion getv, setVersion setv)
    {
        if (g_webconfig_mock)
        {
            g_webconfig_mock->register_sub_docs_mock(bInfo, numOfSubdocs, getv, setv);
        }
    }
}

/* ----------Base64 and Blob ----------- */
MockBase64 *g_mockBase64 = nullptr;

extern "C"
{
    bool b64_decode(const uint8_t *input, size_t input_len, uint8_t *output)
    {
        return g_mockBase64->b64_decode(input, input_len, output);
    }

    int b64_get_decoded_buffer_size(size_t str_len)
    {
        return g_mockBase64->b64_get_decoded_buffer_size(str_len);
    }
    void PushBlobRequest(execData *execDataLan)
    {
        int a = 0;
        return;
    }
    void rdk_logger_init(char* testStr){
        int b = 0;
        return;
    }
}

