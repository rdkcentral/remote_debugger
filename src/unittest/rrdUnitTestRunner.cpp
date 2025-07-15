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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cJSON.h"

// general Mocks
#include "Client_Mock.h"
#include "Client_Mock.cpp"

// rrdJsonParser
#include "rrdJsonParser.h"
#include "rrdJsonParser.c"

// rrdCommandSanity
#include "rrdCommandSanity.h"
#include "rrdCommandSanity.c"

// rrdDeepSleep
#include "rrdDynamic.h"
#include "rrdDynamic.c"

// rrdExecuteScript
#include "rrdExecuteScript.h"
#include "rrdExecuteScript.c"

// rrdRunCmdThread
#include "rrdRunCmdThread.h"
#include "rrdRunCmdThread.c"

// rrdEventProcess
#include "rrdEventProcess.h"
#include "rrdEventProcess.c"

#include "rrdInterface.h"
#include "rrdInterface.c"

//rrdIarm
//#ifdef IARMBUS_SUPPORT
#include "rrdIarmEvents.c"
//#endif

// rrdMsgPackDecoder
#include "rrdMsgPackDecoder.h"
#include "rrdMsgPackDecoder.c"

// rrdMain
#include "rrdMain.h"
#include "rrdMain.c"

#define GTEST_DEFAULT_RESULT_FILEPATH "/tmp/Gtest_Report/"
#define GTEST_DEFAULT_RESULT_FILENAME "rdkRemoteDebugger_gtest_report.json"
#define GTEST_REPORT_FILEPATH_SIZE 256

using namespace std;
using ::testing::_;
using ::testing::Return;

/* ====================== rrdJsonParser ================*/
/* --------------- Test getParamcount() from rrdJsonParser --------------- */

/*
TEST(ExecuteCommandsTest, ReturnsFalseIfCommandIsNull) {
    issueData cmd;
    cmd.command = NULL;
    cmd.rfcvalue = "dummy"; // If needed by your code, or set to NULL if not
    cmd.timeout = 0;
    bool result = executeCommands(&cmd);
    EXPECT_FALSE(result);
} */

TEST(RemoteDebuggerDocStrErrorTest, KnownErrorCodes) {
    EXPECT_STREQ(remotedebuggerdoc_strerror(OK), "No errors.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(OUT_OF_MEMORY), "Out of memory.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_FIRST_ELEMENT), "Invalid first element.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_VERSION), "Invalid 'version' value.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_OBJECT), "Invalid 'value' array.");
}

TEST(RemoteDebuggerDocStrErrorTest, UnknownErrorCode) {
    // An error code not defined in the map
    int unknownError = 9999;
    EXPECT_STREQ(remotedebuggerdoc_strerror(unknownError), "Unknown error.");
}

TEST(RemoteDebuggerDocStrErrorTest, EdgeCaseZeroButNotInMap) {
    EXPECT_STREQ(remotedebuggerdoc_strerror(0), "No errors.");
}


TEST(LookupRrdProfileListTest, NullInput) {
    EXPECT_FALSE(lookupRrdProfileList(nullptr));
}
 
TEST(LookupRrdProfileListTest, EmptyStringInput) {
    EXPECT_FALSE(lookupRrdProfileList(""));
}

TEST(LookupRrdProfileListTest, ExactMatchFirst) {
    lookupRrdProfileList("RRD_PROFILE_LIST");
} 

TEST(ExecuteCommandsTest, ReturnsTrueIfCommandIsPresentAndAllSucceed) {
    issueData cmd;
    cmd.command = strdup("echo hello");
    cmd.rfcvalue = strdup("dummy");
    cmd.timeout = 0;
    MockSecure secureApi;
    FILE *fp = fopen(RRD_DEVICE_PROP_FILE, "w");
    // Mock dependencies like mkdir, fopen, etc., as needed
    bool result = executeCommands(&cmd);
    //EXPECT_CALL(secureApi, v_secure_popen(_, _, _))
    //        .WillOnce(Return(fp));
    //EXPECT_CALL(secureApi, v_secure_pclose(_))
    //        .WillOnce(Return(0));
    //EXPECT_CALL(secureApi, v_secure_system(_, _))
          //  .WillOnce(Return(0));
    EXPECT_TRUE(result);
    //free(cmd.command);
    //free(cmd.rfcvalue);
}
/*
TEST(ExecuteCommandsTest, ReturnsTrueIfCommandIsPresentAndAllfail) {
    issueData cmd;
    cmd.command = NULL;
    cmd.rfcvalue = strdup("dummy");
    cmd.timeout = 0;
    MockSecure secureApi;
    FILE *fp = fopen(RRD_DEVICE_PROP_FILE, "w");
    // Mock dependencies like mkdir, fopen, etc., as needed
    bool result = executeCommands(&cmd);
    //EXPECT_CALL(secureApi, v_secure_popen(_, _, _))
    //        .WillOnce(Return(fp));
    //EXPECT_CALL(secureApi, v_secure_pclose(_))
    //        .WillOnce(Return(0));
    //EXPECT_CALL(secureApi, v_secure_system(_, _))
          //  .WillOnce(Return(0));
    EXPECT_FALSE(result);
    //free(cmd.command);
    //free(cmd.rfcvalue);
} */
extern bool checkAppendRequest(char *issueRequest);
/*
TEST(CheckAppendRequestTest, ReturnsTrueAndRemovesSuffixWhenSuffixPresent) {
    char input[64] = "issue_append";
    bool result = checkAppendRequest(input);
    EXPECT_TRUE(result);
    EXPECT_STREQ(input, "issue");
} */

TEST(CheckAppendRequestTest, ReturnsTrueAndRemovesSuffixWhenSuffixPresent) {
    char input[64] = "issue_apnd";
    bool result = checkAppendRequest(input);
    EXPECT_TRUE(result);
    EXPECT_STREQ(input, "issue");
}
TEST(CheckAppendRequestTest, ReturnsTrueWhenSuffixIsOnlyContent) {
    char input[64] = "_apnd";
    bool result = checkAppendRequest(input);
    EXPECT_TRUE(result);
    EXPECT_STREQ(input, "");
}

TEST(CheckAppendRequestTest, ReturnsFalseWhenSuffixMissing) {
    char input[64] = "issue";
    bool result = checkAppendRequest(input);
    EXPECT_FALSE(result);
    EXPECT_STREQ(input, "issue");  // Should remain unchanged
}

TEST(CheckAppendRequestTest, ReturnsFalseForShortString) {
    char input[64] = "";
    bool result = checkAppendRequest(input);
    EXPECT_FALSE(result);
    EXPECT_STREQ(input, "");  // Should remain unchanged
}
/*
TEST(CheckAppendRequestTest, ReturnsTrueWhenSuffixIsOnlyContent) {
    char input[64] = "_append";
    bool result = checkAppendRequest(input);
    EXPECT_TRUE(result);
    EXPECT_STREQ(input, "");
} */

TEST(CheckAppendRequestTest, ReturnsFalseIfSuffixAtStartOnly) {
    char input[64] = "_appendissue";
    bool result = checkAppendRequest(input);
    EXPECT_FALSE(result);
    EXPECT_STREQ(input, "_appendissue");
}

class GetIssueCommandInfoTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Cleanup if needed
    }
    void FreeIssueData(issueData* d) {
        if (!d) return;
        if (d->command) free(d->command);
        if (d->rfcvalue) free(d->rfcvalue);
        free(d);
    }
};

TEST_F(GetIssueCommandInfoTest, ReturnsValidStruct) {
    const char* jsonstr = R"({
        "categoryA": {
            "type1": [ 42, "kill" ]
        },
        "Sanity": {
            "Check": {
                "Commands": [ "kill", "ls" ]
            }
        }
    })";
    cJSON* root = cJSON_Parse(jsonstr);
    ASSERT_NE(root, nullptr);

    issueNodeData node;
    node.Node = (char*)"categoryA";
    node.subNode = (char*)"type1";

    char buf[] = "rfcvalue123";
    issueData* result = getIssueCommandInfo(&node, root, buf);
    ASSERT_NE(result, nullptr);
    
}

TEST_F(GetIssueCommandInfoTest, UsesDefaultTimeoutIfNotSet) {
    const char* jsonstr = R"({
        "categoryB": {
            "typeX": [ "echo only" ]
        },
        "Sanity": {
            "Check": {
                "Commands": [ "kill" ]
            }
        }
    })";
    cJSON* root = cJSON_Parse(jsonstr);
    ASSERT_NE(root, nullptr);

    issueNodeData node;
    node.Node = (char*)"categoryB";
    node.subNode = (char*)"typeX";

    char buf[] = "rfctest";
    issueData* result = getIssueCommandInfo(&node, root, buf);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->timeout, DEFAULT_TIMEOUT);
    ASSERT_NE(result->command, nullptr);
    EXPECT_TRUE(strstr(result->command, "echo only") != nullptr);
    ASSERT_NE(result->rfcvalue, nullptr);
    EXPECT_STREQ(result->rfcvalue, "rfctest");

    FreeIssueData(result);
    cJSON_Delete(root);
}



TEST(GetParamCountTest, GetParamCount)
{
    char str[] = "abc.def.ghi";
    ASSERT_EQ(getParamcount(str), 2);
}

TEST(GetParamCountTest, GetParamCountEmptyString)
{
    char str[] = "";
    ASSERT_EQ(getParamcount(str), 0);
}

TEST(GetParamCountTest, GetParamCountNoDot)
{
    char str[] = "hello";
    ASSERT_EQ(getParamcount(str), 0);
}

TEST(GetParamCountTest, GetParamCountMultipleConsecutiveDots)
{
    char str[] = "x..y";
    ASSERT_EQ(getParamcount(str), 2);
}

/* --------------- Test removeSpecialChar() from rrdJsonParser --------------- */
TEST(RemoveSpecialCharTest, RemoveSpecialCharEmptyString)
{
    char str[] = "";
    removeSpecialChar(str);
    ASSERT_STREQ(str, "");
}

TEST(RemoveSpecialCharTest, RemoveSpecialCharWithCarriageReturn)
{
    char str[] = "hello\rworld";
    removeSpecialChar(str);
    ASSERT_STREQ(str, "hello");
}

TEST(RemoveSpecialCharTest, RemoveSpecialCharWithNewLine)
{
    char str[] = "line1\nline2";
    removeSpecialChar(str);
    ASSERT_STREQ(str, "line1");
}

/* --------------- Test readJsonFile() from rrdJsonParser --------------- */
TEST(ReadJsonFileTest, ReadValidJsonFile)
{
    char *jsonfile = "UTJson/validJson.json";
    char *jsonfile_content = readJsonFile(jsonfile);
    ASSERT_NE(jsonfile_content, nullptr);
    free(jsonfile_content);
}

TEST(ReadJsonFileTest, ReadEmptyFile)
{
    char *jsonfile = "UTJson/emptyJson.json";
    char *jsonfile_content = readJsonFile(jsonfile);
    ASSERT_EQ(jsonfile_content, nullptr);
}

TEST(ReadJsonFileTest, HandleFileReadError)
{
    char *jsonfile = "non_existent_file.json";
    char *jsonfile_content = readJsonFile(jsonfile);
    ASSERT_EQ(jsonfile_content, nullptr);
}

/* --------------- Test readAndParseJSON() from rrdJsonParser --------------- */
TEST(ReadAndParseJSONTest, ParsesJsonFileCorrectly)
{
    cJSON *result = readAndParseJSON("UTJson/validJson.json");
    cJSON *expected = cJSON_CreateObject();
    cJSON_AddStringToObject(expected, "key", "value");
    ASSERT_TRUE(cJSON_Compare(result, expected, true));

    cJSON_Delete(result);
    cJSON_Delete(expected);
}

TEST(ReadAndParseJSONTest, ReturnsNullForInvalidJsonFile)
{
    cJSON *result = readAndParseJSON("UTJson/invalidJson.json");
    ASSERT_EQ(result, nullptr);
}

TEST(ReadAndParseJSONTest, ReturnsNullForNonexistentFile)
{
    cJSON *result = readAndParseJSON("UTJson/non_existent.json");
    ASSERT_EQ(result, nullptr);
}

/* --------------- Test getIssueInfo() from rrdJsonParser --------------- */
class GetIssueInfoTest : public ::testing::Test
{
protected:
    char input[50];
    issueNodeData issue;

    void SetUp() override
    {
        memset(&issue, 0, sizeof(issue));
    }

    void TearDown() override
    {
        if (issue.Node != nullptr)
        {
            free(issue.Node);
            issue.Node = nullptr;
        }
        if (issue.subNode != nullptr)
        {
            free(issue.subNode);
            issue.subNode = nullptr;
        }
    }
};

TEST_F(GetIssueInfoTest, HandlesNormalInput)
{
    strcpy(input, "MainNode.SubNode");
    getIssueInfo(input, &issue);

    ASSERT_STREQ(issue.Node, "MainNode");
    ASSERT_STREQ(issue.subNode, "SubNode");
}

// Need Improvements in getIssueInfo
TEST_F(GetIssueInfoTest, HandlesInputWithoutSubnode)
{
    strcpy(input, "MainNode");
    getIssueInfo(input, &issue);

    ASSERT_STREQ(issue.Node, "MainNode");
    ASSERT_EQ(issue.subNode, nullptr);
}

/* ----------------- findIssueInParsedJSON() in rrdJsonParser ------------------*/
class FindIssueInParsedJSONTest : public ::testing::Test
{
protected:
    issueNodeData issue;
    cJSON *json;
    void SetUp() override
    {
        memset(&issue, 0, sizeof(issue));
        json = cJSON_CreateObject();
        cJSON *category = cJSON_CreateObject();
        cJSON_AddItemToObject(json, "MainNode", category);
        cJSON_AddStringToObject(category, "SubNode", "value");
    }

    void TearDown() override
    {
        free(issue.Node);
        free(issue.subNode);
        cJSON_Delete(json);
    }
};
/*
TEST_F(FindIssueInParsedJSONTest, checkIssueNodeInfo_)
{
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    data_buf buff;
    issueData cmd;
    cmd.command = strdup("echo hello");
    cmd.rfcvalue = strdup("dummy");
    cmd.timeout = 0;
    checkIssueNodeInfo(&issue, json, &buff, false, &cmd);
} */
TEST_F(FindIssueInParsedJSONTest, HandlesNormalInput)
{
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    bool result = findIssueInParsedJSON(&issue, json);

    ASSERT_TRUE(result);
}

TEST_F(FindIssueInParsedJSONTest, HandlesNullNode)
{
    issue.Node = NULL;
    issue.subNode = strdup("SubNode");
    bool result = findIssueInParsedJSON(&issue, json);

    ASSERT_FALSE(result);
}

TEST_F(FindIssueInParsedJSONTest, HandlesNullSubNode)
{
    issue.Node = strdup("MainNode");
    issue.subNode = NULL;
    bool result = findIssueInParsedJSON(&issue, json);

    ASSERT_TRUE(result);
}

TEST_F(FindIssueInParsedJSONTest, HandlesNonexistentNode)
{
    issue.Node = strdup("NonexistentNode");
    issue.subNode = strdup("SubNode");
    bool result = findIssueInParsedJSON(&issue, json);

    ASSERT_FALSE(result);
}

TEST_F(FindIssueInParsedJSONTest, HandlesNonexistentSubNode)
{
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("NonexistentSubNode");

    bool result = findIssueInParsedJSON(&issue, json);

    ASSERT_FALSE(result);
}

/* --------------- Test invokeSanityandCommandExec() from rrdJsonParser --------------- */
class InvokeSanityandCommandExecTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    cJSON *jsoncfg;
    char *buf;

    void SetUp() override
    {
        issuestructNode.Node = strdup("testNode");
        issuestructNode.subNode = strdup("testSubNode");

        jsoncfg = cJSON_CreateObject();
        cJSON *root = cJSON_CreateObject();
        cJSON *category = cJSON_CreateObject();
        cJSON *type = cJSON_CreateArray();
        cJSON_AddItemToArray(type, cJSON_CreateNumber(123));
        cJSON_AddItemToObject(category, "testSubNode", type);
        cJSON_AddItemToObject(root, "testNode", category);
        cJSON_AddItemToObject(jsoncfg, "DEEP_SLEEP_STR", root);
        buf = strdup("testBuffer");
    }

    void TearDown() override
    {
        cJSON_Delete(jsoncfg);
        free(buf);
        system("rm -r testNode-DebugReport*");
    }
};

TEST_F(InvokeSanityandCommandExecTest, DeepSleepAwkEvntTrue_TypeIsNumber_CommandIsNull)
{
    EXPECT_FALSE(invokeSanityandCommandExec(&issuestructNode, jsoncfg, buf, true));
}

TEST_F(InvokeSanityandCommandExecTest, DeepSleepAwkEvntFalse_TypeIsNumber_CommandIsNull)
{
    EXPECT_FALSE(invokeSanityandCommandExec(&issuestructNode, jsoncfg, buf, false));
}

TEST_F(InvokeSanityandCommandExecTest, DeepSleepAwkEvntTrue_TypeIsString_CommandIsNull)
{
    cJSON_DeleteItemFromObject(jsoncfg, "DEEP_SLEEP_STR");
    cJSON *root = cJSON_CreateObject();
    cJSON *category = cJSON_CreateObject();
    cJSON *type = cJSON_CreateArray();
    cJSON_AddItemToArray(type, cJSON_CreateString("dummyCommand"));
    cJSON_AddItemToObject(category, "testSubNode", type);
    cJSON_AddItemToObject(root, "testNode", category);
    cJSON_AddItemToObject(jsoncfg, "DEEP_SLEEP_STR", root);

    EXPECT_FALSE(invokeSanityandCommandExec(&issuestructNode, jsoncfg, buf, true));
}

TEST_F(InvokeSanityandCommandExecTest, DeepSleepAwkEvntTrue_TypeIsNumber_CommandIsNotNull_IsCommandsValidReturnsZero)
{
    cJSON_DeleteItemFromObject(jsoncfg, "DEEP_SLEEP_STR");
    cJSON *root = cJSON_CreateObject();
    cJSON *category = cJSON_CreateObject();
    cJSON *type = cJSON_CreateArray();
    cJSON_AddItemToArray(type, cJSON_CreateNumber(123));
    cJSON_AddItemToArray(type, cJSON_CreateString("commandThatIsNotInSanityList"));
    cJSON_AddItemToObject(category, "testSubNode", type);
    cJSON_AddItemToObject(root, "testNode", category);
    cJSON_AddItemToObject(jsoncfg, "DEEP_SLEEP_STR", root);

    EXPECT_FALSE(invokeSanityandCommandExec(&issuestructNode, jsoncfg, buf, true));
}

TEST_F(InvokeSanityandCommandExecTest, DeepSleepAwkEvntTrue_TypeIsString_CommandIsNotNull_IsCommandsValidReturnsNonZero)
{
    cJSON_DeleteItemFromObject(jsoncfg, "DEEP_SLEEP_STR");
    cJSON *root = cJSON_CreateObject();
    cJSON *category = cJSON_CreateObject();
    cJSON *type = cJSON_CreateArray();
    cJSON_AddItemToArray(type, cJSON_CreateString("command&"));
    cJSON_AddItemToObject(category, "testSubNode", type);
    cJSON_AddItemToObject(root, "testNode", category);
    cJSON_AddItemToObject(jsoncfg, "DEEP_SLEEP_STR", root);

    EXPECT_FALSE(invokeSanityandCommandExec(&issuestructNode, jsoncfg, buf, true));
}

/* --------------- Test processAllDebugCommand() from rrdJsonParser --------------- */
class ProcessAllDebugCommandTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    cJSON *jsoncfg;
    char *buf;

    void SetUp() override
    {
        issuestructNode.Node = strdup("testNode");
        issuestructNode.subNode = strdup("testSubNode");
        jsoncfg = cJSON_CreateObject();
    }

    void TearDown() override
    {
        free(issuestructNode.Node);
        free(issuestructNode.subNode);
        if (jsoncfg)
        {
            cJSON_Delete(jsoncfg);
        }
        system("rm -r testNode-DebugReport*");
    }
};

TEST_F(ProcessAllDebugCommandTest, MainnodeIsNull)
{
    buf = strdup("testBuffer");
    EXPECT_FALSE(processAllDebugCommand(jsoncfg, &issuestructNode, buf));
}

TEST_F(ProcessAllDebugCommandTest, MainnodenameIsNull)
{
    buf = strdup("testBuffer");
    cJSON_AddNullToObject(jsoncfg, "testNode");

    EXPECT_FALSE(processAllDebugCommand(jsoncfg, &issuestructNode, buf));
}

TEST_F(ProcessAllDebugCommandTest, SubitemsIsZero)
{
    buf = strdup("testBuffer");
    cJSON_AddObjectToObject(jsoncfg, "testNode");

    EXPECT_FALSE(processAllDebugCommand(jsoncfg, &issuestructNode, buf));
}

TEST_F(ProcessAllDebugCommandTest, InvokeSanityandCommandExecReturnsFalse)
{
    buf = strdup("testBuffer");
    cJSON *root = cJSON_CreateObject();
    cJSON *category = cJSON_CreateObject();
    cJSON *type = cJSON_CreateArray();
    cJSON_AddItemToArray(type, cJSON_CreateNumber(1));
    cJSON_AddItemToArray(type, cJSON_CreateString("invalidCommand"));
    cJSON_AddItemToObject(category, issuestructNode.subNode, type);
    cJSON_AddItemToObject(root, issuestructNode.Node, category);
    cJSON_AddItemToObject(jsoncfg, "DEEP_SLEEP_STR", root);

    EXPECT_FALSE(processAllDebugCommand(jsoncfg, &issuestructNode, buf));
}

/* --------------- Test processAllDeepSleepAwkMetricsCommands() from rrdJsonParser --------------- */
class ProcessAllDeepSleepAwkMetricsCommandsTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    cJSON *jsoncfg;
    char *buf;

    void SetUp() override
    {
        issuestructNode.Node = strdup("testNode");
        issuestructNode.subNode = strdup("testSubNode");
        buf = strdup("testBuffer");
    }

    void TearDown() override
    {
        if (issuestructNode.subNode)
        {
            free(issuestructNode.subNode);
        }
        if (jsoncfg)
        {
            cJSON_Delete(jsoncfg);
        }
        free(buf);
        system("rm -r testNode-DebugReport*");
    }
};

TEST_F(ProcessAllDeepSleepAwkMetricsCommandsTest, IssueCategoryCountIsZero)
{
    jsoncfg = cJSON_CreateObject();
    cJSON *rootNode = cJSON_AddObjectToObject(jsoncfg, "testNode");
    cJSON_AddArrayToObject(rootNode, "subNode");

    EXPECT_FALSE(processAllDeepSleepAwkMetricsCommands(jsoncfg, &issuestructNode, buf));
}

TEST_F(ProcessAllDeepSleepAwkMetricsCommandsTest, IssueTypeCountIsZero_InvokeSanityandCommandExecReturnsFalse)
{
    jsoncfg = cJSON_CreateObject();
    cJSON *rootNode = cJSON_AddObjectToObject(jsoncfg, "testNode");
    cJSON *category = cJSON_AddArrayToObject(rootNode, "subNode");

    EXPECT_FALSE(processAllDeepSleepAwkMetricsCommands(jsoncfg, &issuestructNode, buf));
}

TEST_F(ProcessAllDeepSleepAwkMetricsCommandsTest, IssueTypeCountIsZero)
{
    jsoncfg = cJSON_CreateObject();
    cJSON *rootNode = cJSON_AddObjectToObject(jsoncfg, "testNode");
    cJSON *category = cJSON_AddObjectToObject(rootNode, "subNode");

    EXPECT_FALSE(processAllDeepSleepAwkMetricsCommands(jsoncfg, &issuestructNode, buf));
}

/* --------------- Test RRDStoreDeviceInfo() from rrdJsonParser --------------- */
class StoreDeviceInfo : public testing::Test
{
protected:
    devicePropertiesData devPropData;
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }

    void SetUp() override
    {
        string test_name = getCurrentTestName();
        const char *deviceName = "";
        FILE *fp = fopen(RRD_DEVICE_PROP_FILE, "w");
        if (test_name == "ReadDevicePropFilePlatco")
        {
            deviceName = "PLATCO";
            fprintf(fp, "DEVICE_NAME=%s", deviceName);
        }
        else if (test_name == "ReadDevicePropFileLlama")
        {
            deviceName = "LLAMA";
            fprintf(fp, "DEVICE_NAME=%s", deviceName);
        }
        else if (test_name == "ReadDevicePropFileEDev")
        {
            deviceName = "XI6";
            fprintf(fp, "DEVICE_NAME=%s", deviceName);
        }
        else if (test_name == "ReadDevicePropFileDefault")
        {
            deviceName = "UnknownDevice";
            fprintf(fp, "DEVICE_NAME=%s", deviceName);
        }
        fclose(fp);
    }

    void TearDown() override
    {
        remove(RRD_DEVICE_PROP_FILE);
    }
};
/* ================ rrdCommandSanity =====================*/
/* --------------- Test updateBackgroundCmd() from rrdCommandSanity --------------- */
TEST(UpdateBackgroundCmdTest, ReturnsOneForNullInput)
{
    int result = updateBackgroundCmd(NULL);
    ASSERT_EQ(result, 1);
}

TEST(UpdateBackgroundCmdTest, UpdatesBackgroundCmdCorrectly)
{
    char test_str[] = "command&;";
    int result = updateBackgroundCmd(test_str);

    ASSERT_EQ(result, 0);
    ASSERT_STREQ(test_str, "command& ");
}

TEST(UpdateBackgroundCmdTest, DoesNothingForStringWithoutAmpersandSemicolon)
{
    char test_str[] = "command";
    int result = updateBackgroundCmd(test_str);

    ASSERT_EQ(result, 0);
    ASSERT_STREQ(test_str, "command");
}

/* --------------- Test isCommandsValid() from rrdCommandSanity --------------- */
class IsCommandsValidTest : public ::testing::Test
{
protected:
    cJSON *sanitylist;

    void SetUp() override
    {
        sanitylist = cJSON_CreateArray();
        cJSON *harmful_cmd = cJSON_CreateString("kill");
        cJSON_AddItemToArray(sanitylist, harmful_cmd);
    }

    void TearDown() override
    {
        cJSON_Delete(sanitylist);
    }
};

TEST_F(IsCommandsValidTest, ReturnsZeroForValidCommand)
{
    int result = isCommandsValid("ls -l", sanitylist);
    ASSERT_EQ(result, 0);
}

TEST_F(IsCommandsValidTest, ReturnsZeroForBackgroundCommand)
{
    char command[] = "ls -lh &;";
    int result = isCommandsValid(command, sanitylist);

    ASSERT_EQ(result, 0);
    ASSERT_STREQ(command, "ls -lh & ");
}

/* --------------- Test replaceRRDLocation() from rrdCommandSanity --------------- */

TEST(ReplaceRRDLocationTest, TestReplaceRRDLocation_SubstringExists)
{
    char *new_dir = "/new/dir";
    char *command = "ls RRD_LOCATION";
    const char *expected_command = "ls /new/dir";
    char *command_copy = strdup(command);
    char *result = replaceRRDLocation(command_copy, new_dir);

    EXPECT_STREQ(result, expected_command);

    free(result);
}

TEST(ReplaceRRDLocationTest, TestReplaceRRDLocation_SubstringNotExists)
{
    char *new_dir = "/new/dir";
    char *command = "ls /default/dir";
    const char *expected_command = "ls /default/dir";
    char *command_copy = strdup(command);
    char *result = replaceRRDLocation(command_copy, new_dir);

    EXPECT_STREQ(result, expected_command);

    free(result);
}

/* ------------- Mock for setParam() from tr181api.h ---------------*/

// sample function to call setParam Mock API
tr181ErrorCode_t sampleSetParam(MockSetParam &mock_set_param, char *arg1, const char *arg2, const char *arg3)
{
    RDK_LOG(RDK_LOG_ERROR, LOG_REMDEBUG, "[%s:%d]:  Called SampleSetParam!!! \n ", __FUNCTION__, __LINE__);
    return mock_set_param.setParam(arg1, arg2, arg3);
}

class SetParamByRFC : public ::testing::Test
{
protected:
    MockSetParam mock_set_param;

    void SetUp() override
    {
        SetParamWrapper::setImpl(&mock_set_param);
        EXPECT_CALL(mock_set_param, setParam(_, _, _)).WillOnce(Return(tr181Success)).WillOnce(Return(tr181Failure));
    }

    void TearDown() override
    {
        SetParamWrapper::setImpl(nullptr);
    }
};

TEST_F(SetParamByRFC, TestSetParam)
{
    tr181ErrorCode_t result = sampleSetParam(mock_set_param, "arg1", "arg2", "arg3");
    EXPECT_EQ(result, tr181Success);

    result = sampleSetParam(mock_set_param, "arg1", "arg2", "arg3");
    EXPECT_EQ(result, tr181Failure);
}

#ifdef IARMBUS_SUPPORT
/* ----------------IARM --------------- */
class IARMBusTest : public ::testing::Test
{
protected:
    ClientIARMMock mock;

    void SetUp() override
    {
        setMock(&mock);
    }

    void TearDown() override
    {
        setMock(nullptr);
    }
};

TEST_F(IARMBusTest, TestIARM_Bus_Disconnect)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect())
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    IARM_Result_t result = IARM_Bus_Disconnect();
    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}

TEST_F(IARMBusTest, TestIARM_Bus_Term)
{
    EXPECT_CALL(mock, IARM_Bus_Term())
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    IARM_Result_t result = IARM_Bus_Term();
    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}

TEST_F(IARMBusTest, TestIARM_Bus_UnRegisterEventHandler)
{
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    IARM_Result_t result = IARM_Bus_UnRegisterEventHandler("owner", IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS);
    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}
#endif

/* ------------- RBUS ------------- */
class RBusApiTest : public ::testing::Test
{
protected:
    MockRBusApi mock_rbus_api;

    void SetUp() override
    {
        RBusApiWrapper::setImpl(&mock_rbus_api);
        EXPECT_CALL(mock_rbus_api, rbus_open(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mock_rbus_api, rbus_close(_))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mock_rbus_api, rbusValue_Init(_))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mock_rbus_api, rbus_get(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    }

    void TearDown() override
    {
        RBusApiWrapper::clearImpl();
    }
};

TEST_F(RBusApiTest, TestRBusApi)
{
    rbusHandle_t handle;
    rbusValue_t value;
    rbusError_t result;

    result = RBusApiWrapper::rbus_open(&handle, "component");
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);

    result = RBusApiWrapper::rbusValue_Init(&value);
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);

    result = RBusApiWrapper::rbusValue_SetString(value, "string");
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);

    result = RBusApiWrapper::rbus_set(handle, "objectName", value, nullptr);
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);

    result = RBusApiWrapper::rbus_get(handle, "objectName", value, nullptr);
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);

    result = RBusApiWrapper::rbus_close(handle);
    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
}

/* ---------- WebConfig ------------- */
// sample function to call register_sub_docs Mock API
void sampleWebconfigFrameworkInit(ClientWebConfigMock &mock_webconfig)
{
    char *sub_doc = "remotedebugger";

    blobRegInfo *blobData;
    blobData = (blobRegInfo *)malloc(sizeof(blobRegInfo));
    memset(blobData, 0, sizeof(blobRegInfo));
    strncpy(blobData->subdoc_name, sub_doc, strlen(sub_doc) + 1);

    mock_webconfig.register_sub_docs_mock(blobData, 1 /*SubDoc Count*/, nullptr, nullptr);
    free(blobData);
}

class WebConfigTest : public ::testing::Test
{
protected:
    ClientWebConfigMock mock_webconfig;

    void SetUp() override
    {
        setWebConfigMock(&mock_webconfig);
    }

    void TearDown() override
    {
        setWebConfigMock(nullptr);
    }
};

TEST_F(WebConfigTest, TestRegisterSubDocMock)
{
    EXPECT_CALL(mock_webconfig, register_sub_docs_mock(_, _, _, _))
        .Times(1)
        .WillOnce([](blobRegInfo *bInfo, int numOfSubdocs, getVersion getv, setVersion setv)
                  {
            ASSERT_NE(bInfo, nullptr);
            ASSERT_EQ(numOfSubdocs, 1);
            ASSERT_EQ(getv, nullptr);
            ASSERT_EQ(setv, nullptr); });

    sampleWebconfigFrameworkInit(mock_webconfig);
    ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mock_webconfig));
}

/* --------------- Test RRDCheckIssueInDynamicProfile() from rrdDeepSleep --------------- */
class RRDCheckIssueInDynamicProfileTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    data_buf buff;
};

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsFalse)
{
    issueNodeData issuestructNode;
    data_buf buff;
    issuestructNode.Node = NULL;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = false;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathDoesNotExist)
{
    issueNodeData issuestructNode;
    data_buf buff;
    issuestructNode.Node = NULL;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseJSONReturnsNull)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = NULL;
    data_buf buff;
    buff.inDynamic = true;
    buff.jsonPath = strdup("UTJson/emptyJson.json");
    buff.mdata = NULL;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);

    free(buff.jsonPath);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseNonNull_FindIssueFalse)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);

    free(buff.jsonPath);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseNonNull_FindIssueTrue)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("key");
    issuestructNode.subNode = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result, "{\n\t\"key\":\t\"value\"\n}");

    free(result);
    free(issuestructNode.Node);
    free(buff.jsonPath);
}



/* --------------- Test RRDRdmManagerDownloadRequest() from rrdDeepSleep --------------- */
class RRDRdmManagerDownloadRequestTest : public ::testing::Test
{
protected:
    devicePropertiesData originalDevPropData;
    MockRBusApi mock_rbus_api;
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    void SetUp() override
    {
        originalDevPropData = devPropData;
        string test_name = getCurrentTestName();
        if (test_name == "DeepSleepAwakeEventIsFalse_SetParamReturnsFailure" || test_name == "DeepSleepAwakeEventIsTrue_SetParamReturnsFailure")
        {
            RBusApiWrapper::setImpl(&mock_rbus_api);
        }
    }

    void TearDown() override
    {
        devPropData = originalDevPropData;
        SetParamWrapper::clearImpl();
        string test_name = getCurrentTestName();
        if (test_name == "DeepSleepAwakeEventIsFalse_SetParamReturnsFailure")
        {
            RBusApiWrapper::clearImpl();
        }
    }
};

TEST_F(RRDRdmManagerDownloadRequestTest, IssueStructNodeIsNull)
{
    issueNodeData *issuestructNode = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = false;
    RRDRdmManagerDownloadRequest(issuestructNode, buff.jsonPath, &buff, false);

    EXPECT_EQ(issuestructNode, nullptr);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsFalse_SetParamReturnsFailure)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;

    //MockSetParam mock_set_param;
    //SetParamWrapper::setImpl(&mock_set_param);
    //EXPECT_CALL(mock_set_param, setParam(_, _, _)).WillOnce(Return(tr181Failure));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, false);

    free(buff.jsonPath);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsFailure)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsFalse_SetParamReturnsSuccess)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, false);

    free(buff.jsonPath);
    free(buff.mdata);
}
/*
TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_X1)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_PLATCO)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_DEF)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}
*/

/* --------------- Test RRDProcessDeepSleepAwakeEvents() from rrdDeepSleep --------------- */
class RRDProcessDeepSleepAwakeEventsTest : public ::testing::Test
{
protected:
    devicePropertiesData originalDevPropData;

    void SetUp() override
    {
        originalDevPropData = devPropData;
    }

    void TearDown() override
    {
        devPropData = originalDevPropData;
        SetParamWrapper::clearImpl();
    }
};

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDataNull)
{
    data_buf buff;
    buff.mdata = nullptr;
    RRDProcessDeepSleepAwakeEvents(&buff);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsInvalidDefault)
{
    data_buf rbuf;
    rbuf.mdata = "Sample data";
    rbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmDownloadPkgInitiateSetParamSuccess)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmDownloadPkgInitiateSetParamFail)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmPkgInstallCompleteInDynamicFalse)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE;
    rbuf.inDynamic = false;
    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmPkgInstallCompleteInDynamicTrue)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE;
    rbuf.inDynamic = true;
    rbuf.jsonPath = NULL;
    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

#ifdef IARMBUS_SUPPORT
/* ====================== rrdDeepSleep ===================*/
/* --------------- Test RRDGetDeepSleepdynJSONPathLen() from rrdDeepSleep --------------- */
devicePropertiesData devPropData;
class RRDGetDeepSleepdynJSONPathLenTest : public ::testing::Test
{
protected:
    devicePropertiesData originalDevPropData;
    devicePropertiesData testDevPropData;

    void SetUp() override
    {
        originalDevPropData = devPropData;
    }

    void TearDown() override
    {
        devPropData = originalDevPropData;
    }
};

TEST_F(RRDGetDeepSleepdynJSONPathLenTest, TestRRDGetDeepSleepdynJSONPathLen)
{
    //testDevPropData.deviceType = RRD_DEFAULT_PLTFMS;
    devPropData = testDevPropData;
    EXPECT_EQ(RRDGetDeepSleepdynJSONPathLen(), strlen(RRD_MEDIA_APPS) + strlen(RDM_PKG_PREFIX) + strlen(DEEP_SLEEP_STR) + strlen(deviceProfileMap[RRD_DEFAULT_PLTFMS]) + strlen(RRD_JSON_FILE) + 1);

    testDevPropData.deviceType = RRD_REG_X1_PLTFMS;
    devPropData = testDevPropData;
    EXPECT_EQ(RRDGetDeepSleepdynJSONPathLen(), strlen(RRD_MEDIA_APPS) + strlen(RDM_PKG_PREFIX) + strlen(DEEP_SLEEP_STR) + strlen(deviceProfileMap[RRD_REG_X1_PLTFMS]) + strlen(RRD_JSON_FILE) + 1);

    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;
    EXPECT_EQ(RRDGetDeepSleepdynJSONPathLen(), strlen(RRD_MEDIA_APPS) + strlen(RDM_PKG_PREFIX) + strlen(DEEP_SLEEP_STR) + strlen(deviceProfileMap[RRD_LLAMA_PLTFMS]) + strlen(RRD_JSON_FILE) + 1);

    testDevPropData.deviceType = RRD_PLATCO_PLTFMS;
    devPropData = testDevPropData;
    EXPECT_EQ(RRDGetDeepSleepdynJSONPathLen(), strlen(RRD_MEDIA_APPS) + strlen(RDM_PKG_PREFIX) + strlen(DEEP_SLEEP_STR) + strlen(deviceProfileMap[RRD_PLATCO_PLTFMS]) + strlen(RRD_JSON_FILE) + 1);
}

/* --------------- Test RRDGetProfileStringLength() from rrdDeepSleep --------------- */

TEST(RRDGetProfileStringLengthTest, HandlesIsDeepSleepAwakeEventFalse)
{
    issueNodeData issue;
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    int length = RRDGetProfileStringLength(&issue, false);

    ASSERT_EQ(length, strlen(RDM_PKG_PREFIX) + strlen(issue.Node) + strlen(RDM_PKG_SUFFIX) + 1);

    free(issue.Node);
    free(issue.subNode);
}

TEST(RRDGetProfileStringLengthTest, HandlesIsDeepSleepAwakeEventTrueRRD_REG_X1_PLTFMS)
{
    issueNodeData issue;
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    devPropData.deviceType = RRD_REG_X1_PLTFMS;
    int length = RRDGetProfileStringLength(&issue, true);

    ASSERT_EQ(length, strlen(RDM_PKG_PREFIX) + strlen(RDM_PKG_SUFFIX) + strlen(DEEP_SLEEP_STR) + 1);

    free(issue.Node);
    free(issue.subNode);
}

TEST(RRDGetProfileStringLengthTest, HandlesIsDeepSleepAwakeEventTrueRRD_LLAMA_PLTFMS)
{
    issueNodeData issue;
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    devPropData.deviceType = RRD_LLAMA_PLTFMS;
    int length = RRDGetProfileStringLength(&issue, true);

    ASSERT_EQ(length, strlen(RDM_PKG_PREFIX) + strlen(LLAMMA_PROFILE_STR) + strlen(RDM_PKG_SUFFIX) + strlen(DEEP_SLEEP_STR) + 1);

    free(issue.Node);
    free(issue.subNode);
}

TEST(RRDGetProfileStringLengthTest, HandlesIsDeepSleepAwakeEventTrueRRD_PLATCO_PLTFMS)
{
    issueNodeData issue;
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    devPropData.deviceType = RRD_PLATCO_PLTFMS;
    int length = RRDGetProfileStringLength(&issue, true);

    ASSERT_EQ(length, strlen(RDM_PKG_PREFIX) + strlen(PLATCO_PROFILE_STR) + strlen(RDM_PKG_SUFFIX) + strlen(DEEP_SLEEP_STR) + 1);

    free(issue.Node);
    free(issue.subNode);
}

TEST(RRDGetProfileStringLengthTest, HandlesIsDeepSleepAwakeEventTrueRRD_DEFAULT_PLTFMS)
{
    issueNodeData issue;
    issue.Node = strdup("MainNode");
    issue.subNode = strdup("SubNode");
    devPropData.deviceType = RRD_DEFAULT_PLTFMS;
    int length = RRDGetProfileStringLength(&issue, true);

    ASSERT_EQ(length, 0);

    free(issue.Node);
    free(issue.subNode);
}

/* --------------- Test RRDCheckIssueInDynamicProfile() from rrdDeepSleep --------------- */
class RRDCheckIssueInDynamicProfileTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    data_buf buff;
};

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsFalse)
{
    issueNodeData issuestructNode;
    data_buf buff;
    issuestructNode.Node = NULL;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = false;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathDoesNotExist)
{
    issueNodeData issuestructNode;
    data_buf buff;
    issuestructNode.Node = NULL;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseJSONReturnsNull)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = NULL;
    data_buf buff;
    buff.inDynamic = true;
    buff.jsonPath = strdup("UTJson/emptyJson.json");
    buff.mdata = NULL;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);

    free(buff.jsonPath);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseNonNull_FindIssueFalse)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    EXPECT_EQ(result, nullptr);

    free(buff.jsonPath);
}

TEST_F(RRDCheckIssueInDynamicProfileTest, InDynamicIsTrue_PathExists_ReadAndParseNonNull_FindIssueTrue)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("key");
    issuestructNode.subNode = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = true;
    char *result = RRDCheckIssueInDynamicProfile(&buff, &issuestructNode);

    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result, "{\n\t\"key\":\t\"value\"\n}");

    free(result);
    free(issuestructNode.Node);
    free(buff.jsonPath);
}

/* --------------- Test RRDRdmManagerDownloadRequest() from rrdDeepSleep --------------- */
class RRDRdmManagerDownloadRequestTest : public ::testing::Test
{
protected:
    devicePropertiesData originalDevPropData;
    devicePropertiesData testDevPropData;

    void SetUp() override
    {
        originalDevPropData = devPropData;
    }

    void TearDown() override
    {
        devPropData = originalDevPropData;
        SetParamWrapper::clearImpl();
    }
};

TEST_F(RRDRdmManagerDownloadRequestTest, IssueStructNodeIsNull)
{
    issueNodeData *issuestructNode = NULL;
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = NULL;
    buff.inDynamic = false;
    RRDRdmManagerDownloadRequest(issuestructNode, buff.jsonPath, &buff, false);

    EXPECT_EQ(issuestructNode, NULL);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsFalse_SetParamReturnsFailure)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _)).WillOnce(Return(tr181Failure));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, false);

    free(buff.jsonPath);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsFailure)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = NULL;
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _)).WillOnce(Return(tr181Failure));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsFalse_SetParamReturnsSuccess)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, false);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_X1)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    testDevPropData.deviceType = RRD_REG_X1_PLTFMS;
    devPropData = testDevPropData;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_PLATCO)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    testDevPropData.deviceType = RRD_PLATCO_PLTFMS;
    devPropData = testDevPropData;

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));
    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

TEST_F(RRDRdmManagerDownloadRequestTest, DeepSleepAwakeEventIsTrue_SetParamReturnsSuccess_DEF)
{
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("MainNode");
    issuestructNode.subNode = strdup("SubNode");
    data_buf buff;
    buff.mdata = strdup("ValidIssueTypeData");
    buff.jsonPath = strdup("UTJson/validJson.json");
    buff.inDynamic = false;
    testDevPropData.deviceType = RRD_DEFAULT_PLTFMS;
    devPropData = testDevPropData;

    RRDRdmManagerDownloadRequest(&issuestructNode, buff.jsonPath, &buff, true);

    free(buff.jsonPath);
    free(buff.mdata);
}

/* --------------- Test RRDProcessDeepSleepAwakeEvents() from rrdDeepSleep --------------- */
class RRDProcessDeepSleepAwakeEventsTest : public ::testing::Test
{
protected:
    devicePropertiesData originalDevPropData;
    devicePropertiesData testDevPropData;

    void SetUp() override
    {
        originalDevPropData = devPropData;
    }

    void TearDown() override
    {
        devPropData = originalDevPropData;
        SetParamWrapper::clearImpl();
    }
};

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDataNull)
{
    data_buf buff;
    buff.mdata = nullptr;
    RRDProcessDeepSleepAwakeEvents(&buff);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsInvalidDefault)
{
    data_buf rbuf;
    rbuf.mdata = "Sample data";
    rbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmDownloadPkgInitiateSetParamSuccess)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;
    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Success));

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmDownloadPkgInitiateSetParamFail)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;
    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Failure));

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmPkgInstallCompleteInDynamicFalse)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE;
    rbuf.inDynamic = false;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}

TEST_F(RRDProcessDeepSleepAwakeEventsTest, RbufDsEventIsRdmPkgInstallCompleteInDynamicTrue)
{
    data_buf rbuf;
    rbuf.mdata = strdup("IssueNode");
    rbuf.dsEvent = RRD_DEEPSLEEP_RDM_PKG_INSTALL_COMPLETE;
    rbuf.inDynamic = true;
    rbuf.jsonPath = NULL;
    testDevPropData.deviceType = RRD_LLAMA_PLTFMS;
    devPropData = testDevPropData;

    RRDProcessDeepSleepAwakeEvents(&rbuf);
}
#endif

/* ========================== rrdExecuteScript ======================= */
/* --------------- Test normalizeIssueName() from rrdExecuteScript --------------- */
TEST(NormalizeIssueNameTest, HandlesEmptyString)
{
    char str[] = "";
    normalizeIssueName(str);

    ASSERT_STREQ(str, "");
}

TEST(NormalizeIssueNameTest, HandlesStringWithNoDot)
{
    char str[] = "issuedata";
    normalizeIssueName(str);

    ASSERT_STREQ(str, "issuedata");
}

TEST(NormalizeIssueNameTest, HandlesStringWithOnlyDot)
{
    char str[] = ".";
    normalizeIssueName(str);

    ASSERT_STREQ(str, "_");
}

TEST(NormalizeIssueNameTest, HandlesStringWithOneDot)
{
    char str[] = "issuedata.issuetype";
    normalizeIssueName(str);

    ASSERT_STREQ(str, "issuedata_issuetype");
}

TEST(NormalizeIssueNameTest, HandlesStringWithConsecutiveDots)
{
    char str[] = "issuedata...issuetype";
    normalizeIssueName(str);

    ASSERT_STREQ(str, "issuedata___issuetype");
}

/* --------------- Test uploadDebugoutput() from rrdExecuteScript --------------- */
class UploadDebugoutputTest : public ::testing::Test
{
protected:
    int result;

    void SetUp() override
    {
        char command[256];
        sprintf(command, "chmod +x %s", RRD_SCRIPT);
        system(command);
    }

    void TearDown() override
    {
        char command[256];
        sprintf(command, "chmod -x %s", RRD_SCRIPT);
        system(command);
    }
};

TEST_F(UploadDebugoutputTest, HandlesBadPath)
{
    result = uploadDebugoutput("/sample/bad_path", "issuename");
    ASSERT_EQ(result, 1);
}

TEST_F(UploadDebugoutputTest, HandlesNullParameters)
{
    result = uploadDebugoutput(NULL, NULL);
    ASSERT_EQ(result, 0);
}

TEST_F(UploadDebugoutputTest, HandlesGoodPath)
{
    result = uploadDebugoutput("/sample/good_path", "issuename");
    ASSERT_EQ(result, 0);
}

/* ========================== rrdRunCmdThread ======================= */
/* --------------- Test initCache() from rrdRunCmdThread --------------- */
class InitCacheTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        pthread_mutex_destroy(&rrdCacheMut);
    }
};

TEST_F(InitCacheTest, InitializesMutexAndSetsPointerToNull)
{
    initCache();

    int ret = pthread_mutex_trylock(&rrdCacheMut);
    ASSERT_EQ(ret, 0) << "Expected mutex to be unlocked after initCache, but it was locked.";

    if (ret == 0)
    {
        pthread_mutex_unlock(&rrdCacheMut);
    }

    ASSERT_EQ(cacheDataNode, nullptr);
}

/* --------------- Test print_items() from rrdRunCmdThread --------------- */
TEST(PrintItemsTest, HandlesNullNode)
{
    print_items(NULL);
}

TEST(PrintItemsTest, HandlesNonNullNode)
{
    cacheData node;
    node.mdata = strdup("mdata");
    node.issueString = strdup("issueString");
    node.next = NULL;
    node.prev = NULL;
    print_items(&node);

    free(node.mdata);
    free(node.issueString);
}

/* --------------- Test createCache() from rrdRunCmdThread --------------- */
TEST(CreateCacheTest, HandlesNullPkgDataAndValidIssueTypeData)
{
    char *pkgData = NULL;
    char *issueTypeData = strdup("ValidIssueTypeData");
    cacheData *result = createCache(pkgData, issueTypeData);

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->mdata, nullptr);
    ASSERT_STREQ(result->issueString, "ValidIssueTypeData");
    ASSERT_EQ(result->next, nullptr);
    ASSERT_EQ(result->prev, nullptr);

    free(issueTypeData);
    free(result);
}

TEST(CreateCacheTest, HandlesValidPkgDataAndIssueTypeData)
{
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = strdup("ValidIssueTypeData");
    cacheData *result = createCache(pkgData, issueTypeData);

    ASSERT_NE(result, nullptr);
    ASSERT_STREQ(result->mdata, "ValidPkgData");
    ASSERT_STREQ(result->issueString, "ValidIssueTypeData");
    ASSERT_EQ(result->next, nullptr);
    ASSERT_EQ(result->prev, nullptr);

    free(pkgData);
    free(issueTypeData);
    free(result);
}

TEST(CreateCacheTest, HandlesValidPkgDataAndNullIssueTypeData)
{
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = NULL;
    cacheData *result = createCache(pkgData, issueTypeData);

    ASSERT_NE(result, nullptr);
    ASSERT_STREQ(result->mdata, "ValidPkgData");
    ASSERT_EQ(result->issueString, nullptr);
    ASSERT_EQ(result->next, nullptr);
    ASSERT_EQ(result->prev, nullptr);

    free(pkgData);
    free(result);
}

TEST(CreateCacheTest, HandlesNullPkgDataAndIssueTypeData)
{
    char *pkgData = NULL;
    char *issueTypeData = NULL;
    cacheData *result = createCache(pkgData, issueTypeData);

    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->mdata, nullptr);
    ASSERT_EQ(result->issueString, nullptr);
    ASSERT_EQ(result->next, nullptr);
    ASSERT_EQ(result->prev, nullptr);

    free(result);
}

/* --------------- Test append_item() from rrdRunCmdThread --------------- */
class AppendItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pthread_mutex_init(&rrdCacheMut, NULL);
        cacheDataNode = NULL;
    }

    void TearDown() override
    {
        pthread_mutex_destroy(&rrdCacheMut);
        while (cacheDataNode != NULL)
        {
            cacheData *next = cacheDataNode->next;
            if (cacheDataNode->mdata != NULL)
            {
                free(cacheDataNode->mdata);
                cacheDataNode->mdata = NULL;
            }
            if (cacheDataNode->mdata != NULL)
            { // NOT_COVERED
                free(cacheDataNode->issueString);
                cacheDataNode->issueString = NULL;
            }
            free(cacheDataNode);
            cacheDataNode = next;
        }
    }
};

TEST_F(AppendItemTest, HandlesRrdCachecnodeNullAndPkgDataNullAndIssueTypeDataNull)
{
    char *pkgData = NULL;
    char *issueTypeData = NULL;
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_EQ(cacheDataNode->mdata, nullptr);
    ASSERT_EQ(cacheDataNode->issueString, nullptr);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNullAndPkgDataNullAndIssueTypeDataNotNull)
{
    char *pkgData = NULL;
    char *issueTypeData = strdup("ValidIssueTypeData");
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_EQ(cacheDataNode->mdata, nullptr);
    ASSERT_STREQ(cacheDataNode->issueString, issueTypeData);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNullAndPkgDataNotNullAndIssueTypeDataNull)
{
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = NULL;
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_STREQ(cacheDataNode->mdata, pkgData);
    ASSERT_EQ(cacheDataNode->issueString, nullptr);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNullAndPkgDataAndIssueTypeDataNotNull)
{
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = strdup("ValidIssueTypeData");
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_STREQ(cacheDataNode->mdata, pkgData);
    ASSERT_STREQ(cacheDataNode->issueString, issueTypeData);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNotNullAndPkgDataNullAndIssueTypeDataNull)
{
    cacheDataNode = (cacheData *)malloc(sizeof(cacheData));
    cacheDataNode->mdata = strdup("ExistingPkgData");
    cacheDataNode->issueString = strdup("ExistingIssueTypeData");
    cacheDataNode->next = NULL;
    cacheDataNode->prev = NULL;
    char *pkgData = NULL;
    char *issueTypeData = NULL;
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_EQ(cacheDataNode->mdata, nullptr);
    ASSERT_EQ(cacheDataNode->issueString, nullptr);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNotNullAndPkgDataNullAndIssueTypeDataNotNull)
{
    cacheDataNode = (cacheData *)malloc(sizeof(cacheData));
    cacheDataNode->mdata = strdup("ExistingPkgData");
    cacheDataNode->issueString = strdup("ExistingIssueTypeData");
    cacheDataNode->next = NULL;
    cacheDataNode->prev = NULL;
    char *pkgData = NULL;
    char *issueTypeData = strdup("ValidIssueTypeData");
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_EQ(cacheDataNode->mdata, nullptr);
    ASSERT_STREQ(cacheDataNode->issueString, issueTypeData);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNotNullAndPkgDataNotNullAndIssueTypeDataNull)
{
    cacheDataNode = (cacheData *)malloc(sizeof(cacheData));
    cacheDataNode->mdata = strdup("ExistingPkgData");
    cacheDataNode->issueString = strdup("ExistingIssueTypeData");
    cacheDataNode->next = NULL;
    cacheDataNode->prev = NULL;
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = NULL;
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_STREQ(cacheDataNode->mdata, pkgData);
    ASSERT_EQ(cacheDataNode->issueString, nullptr);
}

TEST_F(AppendItemTest, HandlesRrdCachecnodeNotNullAndPkgDataAndIssueTypeDataNotNull)
{
    cacheDataNode = (cacheData *)malloc(sizeof(cacheData));
    cacheDataNode->mdata = strdup("ExistingPkgData");
    cacheDataNode->issueString = strdup("ExistingIssueTypeData");
    cacheDataNode->next = NULL;
    cacheDataNode->prev = NULL;
    char *pkgData = strdup("ValidPkgData");
    char *issueTypeData = strdup("ValidIssueTypeData");
    append_item(pkgData, issueTypeData);

    ASSERT_NE(cacheDataNode, nullptr);
    ASSERT_STREQ(cacheDataNode->mdata, pkgData);
    ASSERT_STREQ(cacheDataNode->issueString, issueTypeData);
}

/* --------------- Test freecacheDataCacheNode() from rrdRunCmdThread --------------- */
TEST(FreecacheDataCacheNodeTest, HandlesNodeNotNullAndMdataNotNullAndIssueStringNotNull)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("ValidMdata");
    node->issueString = strdup("ValidIssueString");
    freecacheDataCacheNode(&node);

    EXPECT_EQ(node, nullptr);
}

TEST(FreecacheDataCacheNodeTest, HandlesNodeNull)
{
    cacheData *node = NULL;
    freecacheDataCacheNode(&node);

    EXPECT_EQ(node, nullptr);
}

TEST(FreecacheDataCacheNodeTest, HandlesNodeNotNullAndMdataNullAndIssueStringNotNull)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = NULL;
    node->issueString = strdup("ValidIssueString");
    freecacheDataCacheNode(&node);

    EXPECT_EQ(node, nullptr);
}

/*---------------- Test removeQuotes() from rrdRunCmdThread ---------------*/
TEST(RemoveQuotesTest, null_string)
{
    removeQuotes(nullptr);
}

TEST(RemoveQuotesTest, HandlesQuotesAndEscapedQuotes)
{
    char str[] = "\"Hello \\\"World\\\"\"";
    removeQuotes(str);
    EXPECT_STREQ(str, "Hello \"World\"");
}

TEST(RemoveQuotesTest, HandlesSurroundingQuotes)
{
    char str[] = "\"Hello World\"";
    removeQuotes(str);
    EXPECT_STREQ(str, "Hello World");
}

TEST(RemoveQuotesTest, HandlesEscapedQuotesWithoutSurroundingQuotes)
{
    char str[] = "Hello \\\"World\\\"";
    removeQuotes(str);
    EXPECT_STREQ(str, "Hello \"World\"");
}

TEST(RemoveQuotesTest, HandlesStringWithoutQuotes)
{
    char str[] = "Hello World";
    removeQuotes(str);
    EXPECT_STREQ(str, "Hello World");
}

TEST(RemoveQuotesTest, HandlesEmptyString)
{
    char str[] = "";
    removeQuotes(str);
    EXPECT_STREQ(str, "");
}

/*----------------- Test copyDebugLogDestFile() from rrdRunCmdThread ------------*/
TEST(CopyDebugLogDestFileTest, HandlesEmptySource)
{
    FILE *source = std::tmpfile();
    FILE *destination = std::tmpfile();

    ASSERT_NE(source, nullptr);
    ASSERT_NE(destination, nullptr);

    fclose(source);
    source = std::tmpfile();
    ASSERT_NE(source, nullptr);

    copyDebugLogDestFile(source, destination);

    fseek(destination, 0, SEEK_SET);
    char buffer[1024] = {0};
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, destination);

    EXPECT_EQ(bytesRead, 0);

    fclose(source);
    fclose(destination);
}

TEST(CopyDebugLogDestFileTest, HandlesNullPointers)
{
    FILE *destination = std::tmpfile();
    ASSERT_NE(destination, nullptr);

    copyDebugLogDestFile(nullptr, destination);

    fseek(destination, 0, SEEK_SET);
    char buffer[1024] = {0};
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, destination);

    EXPECT_EQ(bytesRead, 0);

    fclose(destination);

    FILE *source = std::tmpfile();
    ASSERT_NE(source, nullptr);
    copyDebugLogDestFile(source, nullptr);

    fclose(source);
}

/* --------------- Test findPresentInCache() from rrdRunCmdThread --------------- */
class FindPresentInCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pthread_mutex_init(&rrdCacheMut, NULL);
        cacheDataNode = NULL;
    }

    void TearDown() override
    {
        pthread_mutex_destroy(&rrdCacheMut);
        while (cacheDataNode != NULL)
        {
            cacheData *next = cacheDataNode->next;
            if (cacheDataNode->mdata != NULL)
            {
                free(cacheDataNode->mdata);
                cacheDataNode->mdata = NULL;
            }
            if (cacheDataNode->mdata != NULL)
            { // NOT_COVERED
                free(cacheDataNode->issueString);
                cacheDataNode->issueString = NULL;
            }
            free(cacheDataNode);
            cacheDataNode = next;
        }
    }
};

TEST_F(FindPresentInCacheTest, HandlesPkgDataFoundInSecondElement)
{
    cacheData *firstNode = (cacheData *)malloc(sizeof(cacheData));
    firstNode->mdata = strdup("FirstPkgData");
    firstNode->issueString = strdup("FirstIssueString");
    firstNode->next = NULL;
    cacheData *secondNode = (cacheData *)malloc(sizeof(cacheData));
    secondNode->mdata = strdup("SecondPkgData");
    secondNode->issueString = strdup("SecondIssueString");
    secondNode->next = NULL;
    firstNode->next = secondNode;
    cacheDataNode = firstNode;
    cacheData *result = findPresentInCache("SecondPkgData");

    EXPECT_EQ(result, secondNode);
}

TEST_F(FindPresentInCacheTest, HandlesPkgDataFoundInFirstElement)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("IssueString");
    node->next = NULL;
    cacheDataNode = node;
    cacheData *result = findPresentInCache("PkgData");

    EXPECT_EQ(result, node);
}

TEST_F(FindPresentInCacheTest, HandlesRrdCachecnodeNull)
{
    cacheData *result = findPresentInCache("PkgData");
    EXPECT_EQ(result, nullptr);
}

TEST_F(FindPresentInCacheTest, HandlesPkgDataNotFoundInRrdCachecnode)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("IssueString");
    node->next = NULL;
    cacheDataNode = node;
    cacheData *result = findPresentInCache("NonExistentPkgData");

    EXPECT_EQ(result, nullptr);
}

/* --------------- Test remove_item() from rrdRunCmdThread --------------- */
class RemoveItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pthread_mutex_init(&rrdCacheMut, NULL);
        cacheDataNode = NULL;
    }

    void TearDown() override
    {
        pthread_mutex_destroy(&rrdCacheMut);
        while (cacheDataNode != NULL)
        { // NOT_COVERED
            cacheData *next = cacheDataNode->next;
            if (cacheDataNode->mdata != NULL)
            {
                free(cacheDataNode->mdata);
                cacheDataNode->mdata = NULL;
            }
            if (cacheDataNode->issueString != NULL)
            {
                free(cacheDataNode->issueString);
                cacheDataNode->issueString = NULL;
            }
            free(cacheDataNode);
            cacheDataNode = next;
        }
    }
};

TEST_F(RemoveItemTest, HandlesCacheNull)
{
    remove_item(NULL);
    EXPECT_EQ(cacheDataNode, nullptr);
}

TEST_F(RemoveItemTest, HandlesCacheNotNullAndCacheEqualsRrdCachecnode)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("IssueString");
    node->next = NULL;
    cacheDataNode = node;
    remove_item(node);

    EXPECT_EQ(cacheDataNode, nullptr);
}

TEST_F(RemoveItemTest, HandlesCacheNotNullAndCacheNotEqualsRrdCachecnode)
{
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("IssueString");
    node->next = NULL;
    cacheDataNode = node;
    cacheData *node_dummy = (cacheData *)malloc(sizeof(cacheData));
    node_dummy->mdata = strdup("PkgData");
    node_dummy->issueString = strdup("IssueString");
    node_dummy->next = NULL;
    remove_item(node_dummy);

    EXPECT_NE(cacheDataNode, nullptr);
}

/* ======================== rrdEventProcess ==============*/
/* --------------- Test freeParsedJson() from rrdEventProcess --------------- */
TEST(FreeParsedJsonTest, HandlesValidJson)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "key", "value");
    freeParsedJson(json);
}

TEST(FreeParsedJsonTest, HandlesNullJson)
{
    freeParsedJson(NULL);
}

/* --------------- Test removeSpecialCharacterfromIssueTypeList() from rrdEventProcess --------------- */

TEST(RemoveSpecialCharacterfromIssueTypeListTest, HandlesEmptyString)
{
    char str[] = "";
    removeSpecialCharacterfromIssueTypeList(str);

    EXPECT_STREQ(str, "");
}

TEST(RemoveSpecialCharacterfromIssueTypeListTest, HandlesStringWithCommaAndDot)
{
    char str[] = "a,b.c";
    removeSpecialCharacterfromIssueTypeList(str);

    EXPECT_STREQ(str, "a,b.c");
}

TEST(RemoveSpecialCharacterfromIssueTypeListTest, HandlesStringWithoutCommaAndDot)
{
    char str[] = "abc";
    removeSpecialCharacterfromIssueTypeList(str);

    EXPECT_STREQ(str, "abc");
}

TEST(RemoveSpecialCharacterfromIssueTypeListTest, HandlesStringWithConsecutiveSpecialCharacters)
{
    char str[] = "a^&b";
    char str1[] = "^x";
    removeSpecialCharacterfromIssueTypeList(str);
    removeSpecialCharacterfromIssueTypeList(str1);

    EXPECT_STREQ(str, "ab");
    EXPECT_STREQ(str1, "x");
}

/* --------------- Test issueTypeSplitter() from rrdEventProcess --------------- */
TEST(IssueTypeSplitterTest, HandlesStringWithSpecialCharacters)
{
    char str[] = "a@,b,&,cd,ef";
    char **args = NULL;
    int count = issueTypeSplitter(str, ',', &args);

    ASSERT_EQ(count, 4);
    ASSERT_STREQ(args[0], "a");
    ASSERT_STREQ(args[1], "b");
    ASSERT_STREQ(args[2], "cd");
    ASSERT_STREQ(args[3], "ef");

    for (int i = 0; i < count; i++)
    {
        free(args[i]);
    }
    free(args);
}

TEST(IssueTypeSplitterTest, HandlesStringWithNoSpecialCharacters)
{
    char str[] = "abcd";
    char **args = NULL;
    int count = issueTypeSplitter(str, ',', &args);

    ASSERT_EQ(count, 1);
    ASSERT_STREQ(args[0], "abcd");

    for (int i = 0; i < count; i++)
    {
        free(args[i]);
    }
    free(args);
}

TEST(IssueTypeSplitterTest, HandlesEmptyString)
{
    char str[] = "";
    char **args = NULL;
    int count = issueTypeSplitter(str, ',', &args);

    ASSERT_EQ(count, 1);

    free(args);
}

/* --------------- Test processIssueTypeInDynamicProfile() from rrdEventProcess --------------- */
class ProcessIssueTypeInDynamicProfileTest : public ::testing::Test
{
protected:
    issueNodeData issuestructNode;
    data_buf buff;

    void SetUp() override
    {
        issuestructNode.Node = strdup("testNode");
        issuestructNode.subNode = strdup("testSubNode");
        buff.mdata = strdup("testData");
    }

    void TearDown() override
    {
        free(issuestructNode.Node);
        free(issuestructNode.subNode);
    }
};

TEST_F(ProcessIssueTypeInDynamicProfileTest, JsonPathIsNull)
{
    buff.jsonPath = NULL;
    char *mdata_before = strdup(buff.mdata);
    processIssueTypeInDynamicProfile(&buff, &issuestructNode);

    ASSERT_EQ(buff.mdata, nullptr);

    free(mdata_before);
}

TEST_F(ProcessIssueTypeInDynamicProfileTest, JsonPathIsNotNull_ReadAndParseJSONReturnsNull)
{
    buff.jsonPath = strdup("UTJson/invalidJson.json");
    char *mdata_before = strdup(buff.mdata);
    char *jsonPath_before = strdup(buff.jsonPath);
    processIssueTypeInDynamicProfile(&buff, &issuestructNode);

    ASSERT_EQ(buff.mdata, nullptr);
    ASSERT_EQ(buff.jsonPath, nullptr);

    free(mdata_before);
    free(jsonPath_before);
}

TEST_F(ProcessIssueTypeInDynamicProfileTest, JsonPathIsNotNull_ReadAndParseJSONReturnsNotNull_FindIssueInParsedJSONReturnsFalse)
{
    buff.jsonPath = strdup("UTJson/validJson.json");
    char *mdata_before = strdup(buff.mdata);
    char *jsonPath_before = strdup(buff.jsonPath);
    processIssueTypeInDynamicProfile(&buff, &issuestructNode);

    ASSERT_EQ(buff.mdata, nullptr);
    ASSERT_EQ(buff.jsonPath, nullptr);

    free(mdata_before);
    free(jsonPath_before);
}

/* --------------- Test processWebCfgTypeEvent() from rrdEventProcess --------------- */
TEST(ProcessWebCfgTypeEvntTest, RBufIsNull){
    data_buf *rbuf = nullptr;
    processWebCfgTypeEvent(rbuf);
}

TEST(ProcessWebCfgTypeEvntTest, RBufDataIsNull){
    data_buf rbuf;
    rbuf.mdata = nullptr;
    rbuf.jsonPath = strdup("sample-path");
    processWebCfgTypeEvent(&rbuf);

    free(rbuf.jsonPath);
}

/* --------------- Test processIssueTypeEvent() from rrdEventProcess --------------- */
TEST(ProcessIssueTypeEvntTest, RBufIsNull){
    data_buf *rbuf = nullptr;
    processIssueTypeEvent(rbuf);
}

TEST(ProcessIssueTypeEvntTest, inDynamic_NoJson){
    data_buf rbuf;
    rbuf.mdata = strdup("a");
    rbuf.inDynamic = true;
    rbuf.jsonPath = nullptr;
    processIssueTypeEvent(&rbuf);
}

/* ======================== rrdExecuteScript ==============*/

/* --------------- Test processIssueTypeInInstalledPackage() from rrdExecuteScript --------------- */
TEST(ProcessIssueTypeInInstalledPackageTest, WhenReadAndParseJSONReturnsNull)
{
    data_buf rbuf;
    issueNodeData *issuestructNode = NULL;
    rbuf.jsonPath = strdup("UTJson/emptyJson.json");
    processIssueTypeInInstalledPackage(&rbuf, issuestructNode);

    free(rbuf.jsonPath);
}

#ifdef IARMBUS_SUPPORT
TEST(ProcessIssueTypeInInstalledPackageTest, WhenReadAndParseJSONReturnsNonNull)
{
    data_buf rbuf;
    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("sample");
    issuestructNode.subNode = NULL;
    rbuf.jsonPath = strdup("UTJson/validJson.json");

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Failure));
    processIssueTypeInInstalledPackage(&rbuf, &issuestructNode);

    free(rbuf.jsonPath);
}

/* --------------- Test processIssueTypeInStaticProfile() from rrdExecuteScript --------------- */
class ProcessIssueTypeInStaticProfileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SetParamWrapper::clearImpl();
    }
    void TearDown() override
    {
        SetParamWrapper::clearImpl();
    }
};

TEST_F(ProcessIssueTypeInStaticProfileTest, StatusProcessWhenReadAndParseJSONReturnsNull)
{

    data_buf rbuf;

    issueNodeData issuestructNode;
    issuestructNode.Node = strdup("key");
    issuestructNode.subNode = strdup("notValue");
    rbuf.jsonPath = strdup("UTJson/validJson2.json");

    MockSetParam mock_set_param;
    SetParamWrapper::setImpl(&mock_set_param);
    EXPECT_CALL(mock_set_param, setParam(_, _, _))
        .WillOnce(Return(tr181Failure));
    processIssueTypeInStaticProfile(&rbuf, &issuestructNode);

    free(rbuf.jsonPath);
}
#endif

/* --------------- Test processIssueType() from rrdExecuteScript --------------- */
TEST(processIssueTypeTest, mdataIsNull)
{
    data_buf rbuf;
    rbuf.mdata = NULL;
    processIssueType(&rbuf);
}

TEST(processIssueTypeTest, dynamicPath)
{
    data_buf rbuf;
    issueNodeData issuestructNode;
    rbuf.mdata = strdup("IssueNode");
    rbuf.inDynamic = true;
    rbuf.jsonPath = NULL;
    processIssueType(&rbuf);
}


/* --------------- Test RRDMsgDeliver() from rrdIarm --------------- */
extern int msqid;
extern key_t key;

class RRDMsgDeliverTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        msqid_cpy = msqid;
        key_cpy = key;
        msqid = msgget(key, IPC_CREAT | 0666);

        ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    }

    void TearDown() override
    {
        int ret = msgctl(msqid, IPC_RMID, nullptr);
        ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

        msqid = msqid_cpy;
        key = key_cpy;
    }
};

TEST_F(RRDMsgDeliverTest, TestMessageDelivery)
{
    data_buf sbuf;
    sbuf.mtype = EVENT_MSG;
    sbuf.mdata = "mdata";
    sbuf.inDynamic = true;
    sbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;
    RRDMsgDeliver(msqid, &sbuf);
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), DEFAULT, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
    ASSERT_EQ(sbuf.mtype, receivedBuf.mtype);
}

TEST_F(RRDMsgDeliverTest, TestMessageDeliveryFailure)
{
    data_buf sbuf;
    sbuf.mtype = EVENT_MSG;
    sbuf.mdata = "mdata";
    sbuf.inDynamic = true;
    sbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;

    EXPECT_EXIT(RRDMsgDeliver(-1, &sbuf), ::testing::ExitedWithCode(1), ".*");
}

/* --------------- Test pushIssueTypesToMsgQueue() from rrdIarm --------------- */
class PushIssueTypesToMsgQueueTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        msqid_cpy = msqid;
        key_cpy = key;
        msqid = msgget(key, IPC_CREAT | 0666);

        ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    }

    void TearDown() override
    {
        int ret = msgctl(msqid, IPC_RMID, nullptr);
        ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

        msqid = msqid_cpy;
        key = key_cpy;
    }
};

TEST_F(PushIssueTypesToMsgQueueTest, TestPushIssueTypesToMsgQueueSuccess)
{
    char issueTypeList[] = "mdata";
    pushIssueTypesToMsgQueue(issueTypeList, EVENT_MSG);
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

#ifdef IARMBUS_SUPPORT
/* ====================== rrdIarm ================*/
/* --------------- Test getBlobVersion() from rrdIarm --------------- */
extern uint32_t gWebCfgBloBVersion;
namespace
{
    TEST(SetBlobVersionTest, SetsGlobalVariable)
    {
        char subdoc[] = "test_subdoc";
        uint32_t version = 5;
        int result = setBlobVersion(subdoc, version);

        EXPECT_EQ(result, 0);
        EXPECT_EQ(gWebCfgBloBVersion, version);
    }
    TEST(GetBlobVersionTest, ReturnsGlobalVariable)
    {
        char subdoc[] = "test_subdoc";
        uint32_t result = getBlobVersion(subdoc);

        EXPECT_EQ(result, gWebCfgBloBVersion);
    }
}

/* --------------- Test RRD_data_buff_init() from rrdIarm --------------- */
TEST(RRDDataBuffInitTest, InitializeDataBuff)
{
    data_buf sbuf;
    message_type_et sndtype = IARM_EVENT_MSG;
    deepsleep_event_et deepSleepEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    RRD_data_buff_init(&sbuf, sndtype, deepSleepEvent);

    EXPECT_EQ(sbuf.mtype, sndtype);
    EXPECT_EQ(sbuf.mdata, nullptr);
    EXPECT_EQ(sbuf.jsonPath, nullptr);
    EXPECT_FALSE(sbuf.inDynamic);
    EXPECT_EQ(sbuf.dsEvent, deepSleepEvent);
}

/* --------------- Test RRD_data_buff_deAlloc() from rrdIarm --------------- */
TEST(RRDDataBuffDeAllocTest, DeallocateDataBuff)
{
    data_buf *sbuf = (data_buf *)malloc(sizeof(data_buf));
    sbuf->mdata = (char *)malloc(10 * sizeof(char));
    sbuf->jsonPath = (char *)malloc(10 * sizeof(char));

    ASSERT_NO_FATAL_FAILURE(RRD_data_buff_deAlloc(sbuf));
}

TEST(RRDDataBuffDeAllocTest, NullPointer)
{
    data_buf *sbuf = nullptr;

    ASSERT_NO_FATAL_FAILURE(RRD_data_buff_deAlloc(sbuf));
}

/* --------------- Test RRD_unsubscribe() from rrdIarm --------------- */

class RRDUnsubscribeTest : public ::testing::Test
{
protected:
    ClientIARMMock mock;

    void SetUp() override
    {
        setMock(&mock);
    }

    void TearDown() override
    {
        setMock(nullptr);
    }
};

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_Success)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_DisconnectFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_TermFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterEventHandlerFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterRDMMgrEventHandlerRRDFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterPwrMgrEventHandlerFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

/* --------------- Test webconfigFrameworkInit() from rrdIarm --------------- */
class WebConfigIntegrationTest : public ::testing::Test
{
protected:
    ClientWebConfigMock mock_webconfig;

    void SetUp() override
    {
        setWebConfigMock(&mock_webconfig);
    }

    void TearDown() override
    {
        setWebConfigMock(nullptr);
    }
};

TEST_F(WebConfigIntegrationTest, TestWebconfigFrameworkInit)
{
    EXPECT_CALL(mock_webconfig, register_sub_docs_mock(_, _, _, _)).Times(1);
    webconfigFrameworkInit();
}

/* --------------- Test RRDMsgDeliver() from rrdIarm --------------- */
extern int msqid;
extern key_t key;

class RRDMsgDeliverTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        msqid_cpy = msqid;
        key_cpy = key;
        msqid = msgget(key, IPC_CREAT | 0666);

        ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    }

    void TearDown() override
    {
        int ret = msgctl(msqid, IPC_RMID, nullptr);
        ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

        msqid = msqid_cpy;
        key = key_cpy;
    }
};

TEST_F(RRDMsgDeliverTest, TestMessageDelivery)
{
    data_buf sbuf;
    sbuf.mtype = IARM_EVENT_MSG;
    sbuf.mdata = "mdata";
    sbuf.inDynamic = true;
    sbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;
    RRDMsgDeliver(msqid, &sbuf);
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), DEFAULT, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
    ASSERT_EQ(sbuf.mtype, receivedBuf.mtype);
    ASSERT_EQ(receivedBuf.inDynamic, true);
}

TEST_F(RRDMsgDeliverTest, TestMessageDeliveryFailure)
{
    data_buf sbuf;
    sbuf.mtype = IARM_EVENT_MSG;
    sbuf.mdata = "mdata";
    sbuf.inDynamic = true;
    sbuf.dsEvent = RRD_DEEPSLEEP_INVALID_DEFAULT;

    EXPECT_EXIT(RRDMsgDeliver(-1, &sbuf), ::testing::ExitedWithCode(1), ".*");
}

/* --------------- Test pushIssueTypesToMsgQueue() from rrdIarm --------------- */
class PushIssueTypesToMsgQueueTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        msqid_cpy = msqid;
        key_cpy = key;
        msqid = msgget(key, IPC_CREAT | 0666);

        ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    }

    void TearDown() override
    {
        int ret = msgctl(msqid, IPC_RMID, nullptr);
        ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

        msqid = msqid_cpy;
        key = key_cpy;
    }
};

TEST_F(PushIssueTypesToMsgQueueTest, TestPushIssueTypesToMsgQueueSuccess)
{
    char issueTypeList[] = "mdata";
    pushIssueTypesToMsgQueue(issueTypeList, IARM_EVENT_MSG);
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), IARM_EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

/* --------------- Test _remoteDebuggerEventHandler() from rrdIarm --------------- */
class RemoteDebuggerEventHandlerTest : public ::testing::Test
{
protected:
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            msqid_cpy = msqid;
            key_cpy = key;
            msqid = msgget(key, IPC_CREAT | 0666);

            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
        }
    }

    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            int ret = msgctl(msqid, IPC_RMID, nullptr);
            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

            msqid = msqid_cpy;
            key = key_cpy;
        }
    }
};

TEST_F(RemoteDebuggerEventHandlerTest, TestPushIssueTypesToMsgQueueSuccess)
{
    const char *owner = IARM_BUS_RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "mdata";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), IARM_EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

TEST_F(RemoteDebuggerEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RemoteDebuggerEventHandlerTest, TestInvalidEventId)
{
    const char *owner = IARM_BUS_RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT; // Invalid event id
    char data[] = "Test data";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
}

/* --------------- Test _remoteDebuggerWebCfgDataEventHandler() from rrdIarm --------------- */
class RemoteDebuggerWebConfigEventHandlerTest : public ::testing::Test
{
protected:
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            msqid_cpy = msqid;
            key_cpy = key;
            msqid = msgget(key, IPC_CREAT | 0666);

            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
        }
    }
    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            int ret = msgctl(msqid, IPC_RMID, nullptr);
            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

            msqid = msqid_cpy;
            key = key_cpy;
        }
    }
};

TEST_F(RemoteDebuggerWebConfigEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _remoteDebuggerWebCfgDataEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RemoteDebuggerWebConfigEventHandlerTest, TestInvalidEventId)
{
    const char *owner = IARM_BUS_RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT;
    char data[] = "Test data";
    _remoteDebuggerWebCfgDataEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RemoteDebuggerWebConfigEventHandlerTest, TestPushIssueTypesToMsgQueueSuccess)
{
    const char *owner = IARM_BUS_RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA;
    char data[] = "mdata";
    _remoteDebuggerWebCfgDataEventHandler(owner, eventId, data, sizeof(data));
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), IARM_EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

/* --------------- Test _rdmManagerEventHandler() from rrdIarm --------------- */
class RDMMgrEventHandlerTest : public ::testing::Test
{
protected:
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestFoundInCacheDownloadIsCompleteAndDEEPSLEEPIssue" || test_name == "TestFoundInCacheDownloadIsCompleteAndNotDEEPSLEEPIssue")
        {
            msqid_cpy = msqid;
            key_cpy = key;
            msqid = msgget(key, IPC_CREAT | 0666);

            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
        }
    }
    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestFoundInCacheDownloadIsCompleteDEEPSLEEPIssue" || test_name == "TestFoundInCacheDownloadIsCompleteAndNotDEEPSLEEPIssue")
        {
            int ret = msgctl(msqid, IPC_RMID, nullptr);
            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

            msqid = msqid_cpy;
            key = key_cpy;
        }
    }
};

TEST_F(RDMMgrEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _rdmManagerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RDMMgrEventHandlerTest, TestInvalidEventId)
{
    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT; // Invalid event id
    char data[] = "Test data";
    _rdmManagerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RDMMgrEventHandlerTest, TestNotFoundInCache)
{
    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS;
    IARM_Bus_RDMMgr_EventData_t eventData;
    strncpy(eventData.rdm_pkg_info.pkg_name, "Test package", RDM_PKG_NAME_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_version, "1.0.0", RDM_PKG_VERSION_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_inst_path, "/path/to/package", RDM_PKG_INST_PATH_MAX_SIZE);
    _rdmManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(RDMMgrEventHandlerTest, TestFoundInCacheDownloadNotComplete)
{
    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS;
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("IssueString");
    node->next = NULL;
    cacheDataNode = node;
    IARM_Bus_RDMMgr_EventData_t eventData;
    strncpy(eventData.rdm_pkg_info.pkg_name, "PkgData", RDM_PKG_NAME_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_version, "1.0.0", RDM_PKG_VERSION_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_inst_path, "/path/to/package", RDM_PKG_INST_PATH_MAX_SIZE);
    eventData.rdm_pkg_info.pkg_inst_status = RDM_PKG_INSTALL_ERROR;
    _rdmManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(RDMMgrEventHandlerTest, TestFoundInCacheDownloadIsCompleteAndDEEPSLEEPIssue)
{
    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS;
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("DEEPSLEEP");
    node->next = NULL;
    cacheDataNode = node;
    IARM_Bus_RDMMgr_EventData_t eventData;
    strncpy(eventData.rdm_pkg_info.pkg_name, "PkgData", RDM_PKG_NAME_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_version, "1.0.0", RDM_PKG_VERSION_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_inst_path, "/path/to/package", RDM_PKG_INST_PATH_MAX_SIZE);
    eventData.rdm_pkg_info.pkg_inst_status = RDM_PKG_INSTALL_COMPLETE;
    _rdmManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(RDMMgrEventHandlerTest, TestFoundInCacheDownloadIsCompleteAndNotDEEPSLEEPIssue)
{
    const char *owner = IARM_BUS_RDMMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS;
    cacheData *node = (cacheData *)malloc(sizeof(cacheData));
    node->mdata = strdup("PkgData");
    node->issueString = strdup("NotDeepSleepIssue");
    node->next = NULL;
    cacheDataNode = node;
    IARM_Bus_RDMMgr_EventData_t eventData;
    strncpy(eventData.rdm_pkg_info.pkg_name, "PkgData", RDM_PKG_NAME_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_version, "1.0.0", RDM_PKG_VERSION_MAX_SIZE);
    strncpy(eventData.rdm_pkg_info.pkg_inst_path, "/path/to/package", RDM_PKG_INST_PATH_MAX_SIZE);
    eventData.rdm_pkg_info.pkg_inst_status = RDM_PKG_INSTALL_COMPLETE;
    _rdmManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(RDMMgrEventHandlerTest, TestInvalidOwnerName1)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _rdmDownloadEventHandler(owner, eventId, data, sizeof(data));
}

/* --------------- Test _pwrManagerEventHandler() from rrdIarm --------------- */
class PwrMgrEventHandlerTest : public ::testing::Test
{
protected:
    MockRBusApi mock_rbus_api;
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestCurrentStateDeepSleepRBusOpenFail" || test_name == "TestCurrentStateDeepSleepRBusOpenSuccessRbusSetSuccess")
        {
            RBusApiWrapper::setImpl(&mock_rbus_api);
        }
    }
    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestCurrentStateDeepSleepRBusOpenFail" || test_name == "TestCurrentStateDeepSleepRBusOpenSuccessRbusSetFail" || test_name == "TestCurrentStateDeepSleepRBusOpenSuccessRbusSetSuccess")
        {
            RBusApiWrapper::clearImpl();
        }
    }
};

TEST_F(PwrMgrEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _pwrManagerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateNotDeepSleep)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenFail)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenSuccessRbusSetFail)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;

    EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _)).WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenSuccessRbusSetSuccess)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;

    EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

/* --------------- Test RRD_subscribe() from rrdIarm --------------- */
class RRDSubscribeTest : public ::testing::Test
{
protected:
    ClientIARMMock mock;
    ClientWebConfigMock mock_webconfig;

    void SetUp() override
    {
        setMock(&mock);
        setWebConfigMock(&mock_webconfig);
    }

    void TearDown() override
    {
        setMock(nullptr);
        setWebConfigMock(nullptr);
    }
};

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_AllSuccess)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock_webconfig, register_sub_docs_mock(_, _, _, _)).Times(1);
    IARM_Result_t result = RRD_subscribe();

    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_InitFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_ConnectFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_RRDHandlerFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_RRDWebCfgHandlerFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_RDMMgrHandlerFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDSubscribeTest, TestRRD_Subscribe_PwrMgrHandlerFail)
{
    EXPECT_CALL(mock, IARM_Bus_Init(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Connect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_WEBCFGDATA, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    IARM_Result_t result = RRD_subscribe();

    EXPECT_NE(result, IARM_RESULT_SUCCESS);
}
#endif

/* ====================== rrdMsgPackDecoder ================*/
/* --------------- Test rollback_Debugger() from rrdMsgPackDecoder --------------- */
TEST(TestDebuggerRollback, ValidReturn)
{
    int ret = rollback_Debugger();
    EXPECT_EQ(ret, 0);
}

/* --------------- Test remotedebuggerdoc_destroy() from rrdMsgPackDecoder --------------- */
TEST(TestRRDDocDestroy, DocIsNULL)
{
    remotedebuggerdoc_t *ld = NULL;
    remotedebuggerdoc_destroy(ld);
}

TEST(TestRRDDocDestroy, DocNotNullParamNotNullCommandListNull)
{
    remotedebuggerdoc_t *ld = (remotedebuggerdoc_t *)malloc(sizeof(remotedebuggerdoc_t));
    ld->param = (remotedebuggerparam_t *)malloc(sizeof(remotedebuggerparam_t));
    ld->param->commandList = NULL;
    ld->subdoc_name = NULL;
    remotedebuggerdoc_destroy(ld);
}

TEST(TestRRDDocDestroy, DocNotNullParamNotNullCommandListNotNull)
{
    remotedebuggerdoc_t *ld = (remotedebuggerdoc_t *)malloc(sizeof(remotedebuggerdoc_t));
    ld->param = (remotedebuggerparam_t *)malloc(sizeof(remotedebuggerparam_t));
    ld->param->commandList = strdup("sampleCommandList");
    ld->subdoc_name = NULL;
    remotedebuggerdoc_destroy(ld);
}

TEST(TestRRDDocDestroy, DocIsNULLSubDocNotNULL)
{
    remotedebuggerdoc_t *ld = (remotedebuggerdoc_t *)malloc(sizeof(remotedebuggerdoc_t));
    ld->param = NULL;
    ld->subdoc_name = strdup("sampleSubDocs");
    remotedebuggerdoc_destroy(ld);
}

/* --------------- Test __finder() from rrdMsgPackDecoder --------------- */
TEST(FinderTest, MapSizeZero)
{
    msgpack_object_map map;
    map.size = 0;
    msgpack_object *result = __finder("name", MSGPACK_OBJECT_STR, &map);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(errno, MISSING_ENTRY);
}

TEST(FinderTest, KeyTypeNotStr)
{
    msgpack_object_map map;
    msgpack_object_kv kv;
    kv.key.type = MSGPACK_OBJECT_NIL; // not MSGPACK_OBJECT_STR
    map.size = 1;
    map.ptr = &kv;
    msgpack_object *result = __finder("name", MSGPACK_OBJECT_STR, &map);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(errno, MISSING_ENTRY);
}

TEST(FinderTest, KeyTypeStrExpectTypeMatch)
{
    msgpack_object_map map;
    msgpack_object_kv kv;
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str.ptr = "name";
    kv.key.via.str.size = strlen("name");
    kv.val.type = MSGPACK_OBJECT_STR;
    kv.val.via.str.ptr = "value";
    kv.val.via.str.size = strlen("value");
    map.size = 1;
    map.ptr = &kv;
    msgpack_object *result = __finder("name", MSGPACK_OBJECT_STR, &map);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type, MSGPACK_OBJECT_STR);
    EXPECT_STREQ(result->via.str.ptr, "value");
}

TEST(FinderTest, KeyTypeStrValTypeStrMatch)
{
    msgpack_object_map map;
    msgpack_object_kv kv;
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str.ptr = "name";
    kv.key.via.str.size = strlen("name");
    kv.val.type = MSGPACK_OBJECT_STR;
    kv.val.via.str.ptr = "value";
    kv.val.via.str.size = strlen("value");
    map.size = 1;
    map.ptr = &kv;
    msgpack_object *result = __finder("name", MSGPACK_OBJECT_NIL, &map);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type, MSGPACK_OBJECT_STR);
    EXPECT_STREQ(result->via.str.ptr, "value");
}

TEST(FinderTest, KeyTypeStrValTypeAnyMatch)
{
    msgpack_object_map map;
    msgpack_object_kv kv;
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str.ptr = "name";
    kv.key.via.str.size = strlen("name");
    kv.val.type = MSGPACK_OBJECT_NIL;
    map.size = 1;
    map.ptr = &kv;
    msgpack_object *result = __finder("name", MSGPACK_OBJECT_NIL, &map);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->type, MSGPACK_OBJECT_NIL);
}

/* --------------- Test get_msgpack_unpack_status() from rrdMsgPackDecoder --------------- */
TEST(GetMsgpackUnpackStatusTest, DecodedbufIsNullAndSizeIsZero)
{
    char *decodedbuf = NULL;
    int size = 0;

    EXPECT_EQ(get_msgpack_unpack_status(decodedbuf, size), MSGPACK_UNPACK_NOMEM_ERROR);
}

TEST(GetMsgpackUnpackStatusTest, MsgpackUnpackReturnsSuccess)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_int(&pk, 1);

    EXPECT_EQ(get_msgpack_unpack_status(sbuf.data, sbuf.size), MSGPACK_UNPACK_SUCCESS);

    msgpack_sbuffer_destroy(&sbuf);
}

TEST(GetMsgpackUnpackStatusTest, MsgpackUnpackReturnsExtraBytes)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    // Serialize data with extra bytes
    msgpack_pack_int(&pk, 1);
    sbuf.data[sbuf.size] = '\0'; // Add an extra byte

    // msg_unpack_next() internally handles EXTRABYTES. It should return SUCCESS.
    EXPECT_EQ(get_msgpack_unpack_status(sbuf.data, sbuf.size + 1), MSGPACK_UNPACK_SUCCESS);

    msgpack_sbuffer_destroy(&sbuf);
}

TEST(GetMsgpackUnpackStatusTest, MsgpackUnpackReturnsOtherValidMacro)
{
    char decodedbuf[] = "dummy";
    int size = sizeof(decodedbuf);
    msgpack_unpack_return unpack_ret = static_cast<msgpack_unpack_return>(999);
    // This test case is illustrative; msgpack_unpack does not have a return value of 999.

    EXPECT_NE(get_msgpack_unpack_status(decodedbuf, size), unpack_ret);
}

/* --------------- Test remotedebuggerdoc_strerror() from rrdMsgPackDecoder --------------- */
TEST(RemoteDebuggerDocStrErrorTest, KnownError)
{
    EXPECT_STREQ(remotedebuggerdoc_strerror(OK), "No errors.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(OUT_OF_MEMORY), "Out of memory.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_FIRST_ELEMENT), "Invalid first element.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_VERSION), "Invalid 'version' value.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(INVALID_OBJECT), "Invalid 'value' array.");
}

TEST(RemoteDebuggerDocStrErrorTest, UnknownError)
{
    EXPECT_STREQ(remotedebuggerdoc_strerror(MISSING_ENTRY), "Unknown error.");
    EXPECT_STREQ(remotedebuggerdoc_strerror(999), "Unknown error."); // Test with an arbitrary number not in the map
}

/* --------------- Test getReqTotalSizeOfIssueTypesStr() from rrdMsgPackDecoder --------------- */
TEST(GetReqTotalSizeOfIssueTypesStrTest, ArraySizeZero)
{
    msgpack_object obj_array[1];
    msgpack_object_array arr = {0, obj_array};

    EXPECT_EQ(getReqTotalSizeOfIssueTypesStr(&arr), 1);
}

TEST(GetReqTotalSizeOfIssueTypesStrTest, ArraySizeFiveWithNonStringTypes)
{
    msgpack_object obj_array[5];
    for (int i = 0; i < 5; i++)
    {
        obj_array[i].type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    }
    msgpack_object_array arr = {5, obj_array};

    EXPECT_EQ(getReqTotalSizeOfIssueTypesStr(&arr), 6);
}

TEST(GetReqTotalSizeOfIssueTypesStrTest, ArraySizeFiveWithStringTypes)
{
    msgpack_object obj_array[5];
    const char *test_strs[5] = {"one", "two", "three", "four", "five"};
    int total_strlen = 0;
    for (int i = 0; i < 5; i++)
    {
        obj_array[i].type = MSGPACK_OBJECT_STR;
        obj_array[i].via.str.size = strlen(test_strs[i]);
        obj_array[i].via.str.ptr = test_strs[i];
        total_strlen += strlen(test_strs[i]);
    }
    msgpack_object_array arr = {5, obj_array};
    EXPECT_EQ(getReqTotalSizeOfIssueTypesStr(&arr), 1 + 5 + total_strlen);
}

/* --------------- Test FreeResources_RemoteDebugger() from rrdMsgPackDecoder --------------- */
TEST(FreeResourcesRemoteDebuggerTest, NullPointerArgument)
{
    FreeResources_RemoteDebugger(NULL);
    // No assertion needed, just ensuring no crashes
}

/* --------------- Test PrepareDataToPush() from rrdMsgPackDecoder --------------- */
class PrepareDataToPushTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        msqid_cpy = msqid;
        key_cpy = key;
        msqid = msgget(key, IPC_CREAT | 0666);
        ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    }
    void TearDown() override
    {
        int ret = msgctl(msqid, IPC_RMID, nullptr);
        ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

        msqid = msqid_cpy;
        key = key_cpy;
    }
};

TEST_F(PrepareDataToPushTest, SuccessfulExecution)
{
    remotedebuggerparam_t param;
    param.length = 5;
    param.commandList = (char *)"12345";
    PrepareDataToPush(&param);
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

/* --------------- Test helper_convert() from rrdMsgPackDecoder --------------- */
int mock_process_success(void *p, int num, ...)
{
    // Mock implementation of process function
    return 0; // Success
}

int mock_process_failure(void *p, int num, ...)
{
    // Mock implementation of process function that fails
    return -1; // Return non-zero to indicate failure
}

void mock_destroy(void *p)
{
    // Mock implementation of destroy function
    free(p); // Free the allocated memory
}

TEST(HelperConvertTest, BufIsNull)
{
    void *result = helper_convert(NULL, 10, 100, "wrapper", MSGPACK_OBJECT_MAP, false, mock_process_success, mock_destroy);
    EXPECT_NE(result, nullptr); // malloc success
    EXPECT_EQ(errno, 3);
    mock_destroy(result);
}

TEST(HelperConvertTest, MsgPackUnpackSuccessProcessSuccess)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    // Create a map with one key-value pair: "parameters" -> "value"
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 10);
    msgpack_pack_str_body(&pk, "parameters", 10);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    void *result = helper_convert(sbuf.data, sbuf.size, 100, "parameters", MSGPACK_OBJECT_STR, false, mock_process_success, mock_destroy);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(errno, 0); // Expect errno to be 0 (OK)

    msgpack_sbuffer_destroy(&sbuf);
    mock_destroy(result);
}

TEST(HelperConvertTest, MsgPackUnpackSuccessProcessFails)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    // Create a map with one key-value pair: "parameters" -> "value"
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 10);
    msgpack_pack_str_body(&pk, "parameters", 10);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    // Call the helper_convert function with mock process that fails
    void *result = helper_convert(sbuf.data, sbuf.size, 100, "parameters", MSGPACK_OBJECT_STR, false, mock_process_failure, mock_destroy);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(errno, INVALID_FIRST_ELEMENT);

    msgpack_sbuffer_destroy(&sbuf);
}

TEST(HelperConvertTest, MsgPackWrapperConditionFalseProcessFails)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    // Create a map with one key-value pair: "key" -> "value"
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 3);
    msgpack_pack_str_body(&pk, "key", 3);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    // Call the helper_convert function with mock process that fails
    void *result = helper_convert(sbuf.data, sbuf.size, 100, "key", MSGPACK_OBJECT_STR, false, mock_process_failure, mock_destroy);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(errno, INVALID_FIRST_ELEMENT);

    msgpack_sbuffer_destroy(&sbuf);
}

TEST(HelperConvertTest, MsgPackWrapperConditionFalseProcessSuccess)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    // Create a map with one key-value pair: "key" -> "value"
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 3);
    msgpack_pack_str_body(&pk, "key", 3);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    // Call the helper_convert function with mock process that fails
    void *result = helper_convert(sbuf.data, sbuf.size, 100, "key", MSGPACK_OBJECT_STR, false, mock_process_success, mock_destroy);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(errno, 0);

    msgpack_sbuffer_destroy(&sbuf);
}

/* --------------- Test process_remotedebuggerparams() from rrdMsgPackDecoder --------------- */
TEST(ProcessRemoteDebuggerParamsTest, TestProcessRemoteDebuggerParams)
{
    remotedebuggerparam_t rrdParam;
    msgpack_object_map map;
    map.size = -1; // Set map size to -1
    int result = process_remotedebuggerparams(&rrdParam, &map);

    EXPECT_EQ(result, -1);
}

TEST(ProcessRemoteDebuggerParamsTest, TestProcessRemDebugParams_WhileTrueCondition)
{
    remotedebuggerparam_t rrdParam;
    msgpack_object_map map;
    msgpack_object_kv kv;
    msgpack_object_str key;
    msgpack_object_array arr;
    // Set up invalid conditions to fail the while loop condition
    map.size = 1; // Set map size to 1 (arbitrary)
    kv.key.type = MSGPACK_OBJECT_ARRAY;
    key.ptr = "InvalidKey"; // Using an invalid key to fail the condition
    key.size = strlen(key.ptr);
    kv.key.via.str = key;
    kv.val.type = MSGPACK_OBJECT_STR;
    kv.val.via.array = arr;
    map.ptr = &kv;
    int result = process_remotedebuggerparams(&rrdParam, &map);

    EXPECT_EQ(result, -1);
}

TEST(ProcessRemoteDebuggerParamsTest, TestProcessRemDebugParams_WhilePassesMsgPackPassMatchFail)
{
    remotedebuggerparam_t rrdParam;
    msgpack_object_map map;
    msgpack_object_kv kv;
    msgpack_object_str key;
    msgpack_object_array arr;
    // Set up conditions to fail the 'if (0 == match(p, "IssueType"))' condition
    map.size = 1;             // Set map size to 1 (arbitrary)
    key.ptr = "NotIssueType"; // Using valid key to pass the first condition
    key.size = strlen(key.ptr);
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str = key;
    kv.val.type = MSGPACK_OBJECT_ARRAY;
    kv.val.via.array = arr;
    map.ptr = &kv;
    int result = process_remotedebuggerparams(&rrdParam, &map);

    EXPECT_EQ(result, -1);
}

TEST(ProcessRemoteDebuggerParamsTest, TestProcessRemDebugParams_WhilePassForPassIfPass)
{
    remotedebuggerparam_t rrdParam;
    msgpack_object_map map;
    msgpack_object_kv kv;
    msgpack_object_str key;
    msgpack_object_array arr;
    msgpack_object str_obj;
    // Set up conditions where the loop for (index = 0; index < arr->size; index++) enters but fails
    map.size = 1;          // Set map size to 1 (arbitrary)
    key.ptr = "IssueType"; // Using valid key to pass the first condition
    key.size = strlen(key.ptr);
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str = key;
    kv.val.type = MSGPACK_OBJECT_ARRAY;
    // Set arr.size to 1 to simulate a valid array
    msgpack_object str;
    str.type = MSGPACK_OBJECT_STR;
    str.via.str.ptr = "value";
    str.via.str.size = 5;
    arr.ptr = &str;
    arr.size = 1;
    kv.val.via.array = arr;
    map.ptr = &kv;
    int result = process_remotedebuggerparams(&rrdParam, &map);

    EXPECT_EQ(result, 0);
}

TEST(ProcessRemoteDebuggerParamsTest, TestProcessRemDebugParams_LoopEnteredAndHitsCondition)
{
    remotedebuggerparam_t rrdParam;
    msgpack_object_map map;
    msgpack_object_kv kv;
    msgpack_object_str key;
    msgpack_object_array arr;
    msgpack_object str_obj;
    // Set up conditions where the loop for (index = 0; index < arr->size; index++) hits condition
    map.size = 1;          // Set map size to 1 (arbitrary)
    key.ptr = "IssueType"; // Using valid key to pass the first condition
    key.size = strlen(key.ptr);
    kv.key.type = MSGPACK_OBJECT_STR;
    kv.key.via.str = key;
    kv.val.type = MSGPACK_OBJECT_ARRAY;
    // Set arr.size to 2 to simulate a valid array with more than one element
    msgpack_object str1;
    str1.type = MSGPACK_OBJECT_STR;
    str1.via.str.ptr = "value1";
    str1.via.str.size = 6;
    msgpack_object str2;
    str2.type = MSGPACK_OBJECT_STR;
    str2.via.str.ptr = "value2";
    str2.via.str.size = 6;
    arr.ptr = &str1;
    arr.size = 2;
    kv.val.via.array = arr;
    map.ptr = &kv;
    int result = process_remotedebuggerparams(&rrdParam, &map);

    EXPECT_EQ(result, 0);

    free(rrdParam.commandList);
}

/* --------------- Test Process_RemoteDebugger_WebConfigRequest() from rrdMsgPackDecoder --------------- */
class ProcessRRDWfgReq : public ::testing::Test
{
protected:
    void *Data = NULL;
    pErr retStatus;
    remotedebuggerdoc_t *premotedebuggerInfo;
    int msqid_cpy;
    key_t key_cpy;

    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }

    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPrepAndPushToQueue")
        {
            msqid_cpy = msqid;
            key_cpy = key;
            retStatus = NULL;
            msqid = msgget(key, IPC_CREAT | 0666);
            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
            premotedebuggerInfo = (remotedebuggerdoc_t *)malloc(sizeof(remotedebuggerdoc_t));
            premotedebuggerInfo->param = (remotedebuggerparam_t *)malloc(sizeof(remotedebuggerparam_t));
            premotedebuggerInfo->param->commandList = "command";
            premotedebuggerInfo->param->length = strlen(premotedebuggerInfo->param->commandList);
        }
        else
        {
            Data = NULL;
            retStatus = NULL;
        }
    }

    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPrepAndPushToQueue")
        {
            if (retStatus != NULL)
            {
                free(retStatus);
            }
            if (premotedebuggerInfo != NULL)
            {
                if (premotedebuggerInfo->param != NULL)
                {
                    free(premotedebuggerInfo->param);
                }
                free(premotedebuggerInfo);
            }
            int ret = msgctl(msqid, IPC_RMID, nullptr);

            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";
            msqid = msqid_cpy;
            key = key_cpy;
        }
        else
        {
            if (retStatus != NULL)
            {
                free(retStatus);
            }
        }
    }
};

TEST_F(ProcessRRDWfgReq, TestNullData)
{
    retStatus = Process_RemoteDebugger_WebConfigRequest(Data);

    ASSERT_NE(retStatus, nullptr);
    EXPECT_EQ(retStatus->ErrorCode, SYSCFG_FAILURE);
    EXPECT_STREQ(retStatus->ErrorMsg, "failure while applying remote debugger subdoc");
}

TEST_F(ProcessRRDWfgReq, TestPrepAndPushToQueue)
{
    retStatus = Process_RemoteDebugger_WebConfigRequest(premotedebuggerInfo);

    ASSERT_NE(retStatus, nullptr);
    EXPECT_EQ(retStatus->ErrorCode, BLOB_EXEC_SUCCESS);
}

/* --------------- Test get_base64_decodedbuffer() from rrdMsgPackDecoder --------------- */
class Base64DecodeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        g_mockBase64 = new MockBase64();
    }

    void TearDown() override
    {
        delete g_mockBase64;
    }
};

TEST_F(Base64DecodeTest, BufferIsNull)
{
    char *pString = "test";
    char **buffer = nullptr;
    int size;
    int result = get_base64_decodedbuffer(pString, buffer, &size);

    EXPECT_EQ(result, -1);
}

TEST_F(Base64DecodeTest, SizeIsNull)
{
    char *pString = "test";
    char *buffer;
    int *size = nullptr;
    int result = get_base64_decodedbuffer(pString, &buffer, size);

    EXPECT_EQ(result, -1);
}

TEST_F(Base64DecodeTest, PStringIsNull)
{
    char *pString = nullptr;
    char *buffer;
    int size;
    int result = get_base64_decodedbuffer(pString, &buffer, &size);

    EXPECT_EQ(result, -1);
}

TEST_F(Base64DecodeTest, B64BuffZero_B64DecodeFalse)
{
    char *pString = "test";
    char *buffer;
    int size;

    EXPECT_CALL(*g_mockBase64, b64_get_decoded_buffer_size(::testing::_))
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_mockBase64, b64_decode(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));

    int result = get_base64_decodedbuffer(pString, &buffer, &size);

    EXPECT_EQ(result, 0);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(size, 0);

    free(buffer);
}

TEST_F(Base64DecodeTest, B64BuffZero_B64DecodeTrue)
{
    char *pString = "test";
    char *buffer;
    int size;

    EXPECT_CALL(*g_mockBase64, b64_get_decoded_buffer_size(::testing::_))
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_mockBase64, b64_decode(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));

    int result = get_base64_decodedbuffer(pString, &buffer, &size);

    EXPECT_EQ(result, 0);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(size, 1);

    free(buffer);
}

TEST_F(Base64DecodeTest, B64BuffNonZero_B64DecodeFalse)
{
    char *pString = "test";
    char *buffer;
    int size;

    EXPECT_CALL(*g_mockBase64, b64_get_decoded_buffer_size(::testing::_))
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*g_mockBase64, b64_decode(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));

    int result = get_base64_decodedbuffer(pString, &buffer, &size);

    EXPECT_EQ(result, 0);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(size, 0);

    free(buffer);
}

TEST_F(Base64DecodeTest, B64BuffNonZero_B64DecodeTrue)
{
    char *pString = "test";
    char *buffer;
    int size;

    EXPECT_CALL(*g_mockBase64, b64_get_decoded_buffer_size(::testing::_))
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*g_mockBase64, b64_decode(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));

    int result = get_base64_decodedbuffer(pString, &buffer, &size);

    EXPECT_EQ(result, 0);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(size, 1);

    free(buffer);
}

/* --------------- Test process_remotedebuggerdoc() from rrdMsgPackDecoder --------------- */
class RemoteDebuggerDocTest : public ::testing::Test
{
protected:
    remotedebuggerdoc_t ld;

    void SetUp() override
    {
        memset(&ld, 0, sizeof(ld));
    }

    void TearDown() override
    {
        if (ld.param)
        {
            if (ld.param->commandList)
            {
                free(ld.param->commandList);
            }
            free(ld.param);
        }
        if (ld.subdoc_name)
        {
            free(ld.subdoc_name);
        }
    }
};

TEST_F(RemoteDebuggerDocTest, ProcessRemoteDebuggerDocSuccess)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "test", 4);
    msgpack_pack_map(&pk, 1);
    msgpack_object obj;
    msgpack_object obj1;
    msgpack_object obj2;
    msgpack_object obj3;
    obj.type = MSGPACK_OBJECT_MAP;
    obj.via.map.size = 1;
    obj.via.map.ptr = (msgpack_object_kv *)malloc(sizeof(msgpack_object_kv));
    obj.via.map.ptr[0].key.type = MSGPACK_OBJECT_STR;
    obj.via.map.ptr[0].key.via.str.ptr = "IssueType";
    obj.via.map.ptr[0].key.via.str.size = strlen("IssueType");
    obj.via.map.ptr[0].val.type = MSGPACK_OBJECT_ARRAY;
    obj.via.map.ptr[0].val.via.array.size = 1;
    obj.via.map.ptr[0].val.via.array.ptr = (msgpack_object *)malloc(sizeof(msgpack_object));
    obj.via.map.ptr[0].val.via.array.ptr[0].type = MSGPACK_OBJECT_STR;
    obj.via.map.ptr[0].val.via.array.ptr[0].via.str.ptr = "value";
    obj.via.map.ptr[0].val.via.array.ptr[0].via.str.size = strlen("value");
    obj1.type = MSGPACK_OBJECT_STR;
    obj1.via.str.ptr = "subdoc_name";
    obj1.via.str.size = strlen("subdoc_name");
    obj2.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    obj2.via.u64 = 1;
    obj3.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    obj3.via.u64 = 1;
    int result = process_remotedebuggerdoc(&ld, 4, &obj, &obj1, &obj2, &obj3);

    EXPECT_EQ(result, 0);
    EXPECT_NE(ld.subdoc_name, nullptr);
    EXPECT_EQ(ld.version, 1u);
    EXPECT_EQ(ld.transaction_id, 1u);
    EXPECT_NE(ld.param, nullptr);
    EXPECT_NE(ld.param->commandList, nullptr);

    msgpack_sbuffer_destroy(&sbuf);
    free(obj.via.map.ptr[0].val.via.array.ptr);
    free(obj.via.map.ptr);
}

TEST_F(RemoteDebuggerDocTest, ProcessRemoteDebuggerDocFailure)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "test", 4);
    msgpack_pack_map(&pk, 1);
    msgpack_object obj;
    msgpack_object obj1;
    msgpack_object obj2;
    msgpack_object obj3;
    obj.type = MSGPACK_OBJECT_MAP;
    obj.via.map.size = -1; // Set size to negative to simulate the failure
    obj.via.map.ptr = nullptr;
    obj1.type = MSGPACK_OBJECT_STR;
    obj1.via.str.ptr = "subdoc_name";
    obj1.via.str.size = strlen("subdoc_name");
    obj2.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    obj2.via.u64 = 1;
    obj3.type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    obj3.via.u64 = 1;
    int result = process_remotedebuggerdoc(&ld, 4, &obj, &obj1, &obj2, &obj3);

    EXPECT_EQ(result, -1);
    EXPECT_NE(ld.subdoc_name, nullptr);
    EXPECT_NE(ld.param, nullptr);

    msgpack_sbuffer_destroy(&sbuf);
}

/* --------------- Test remotedebuggerdoc_convert() from rrdMsgPackDecoder --------------- */
TEST(RemoteDebuggerDocConvertTest, ConvertSuccess)
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_map(&pk, 4);
    msgpack_pack_str(&pk, 16);
    msgpack_pack_str_body(&pk, "remotedebugger", 16);
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, 9);
    msgpack_pack_str_body(&pk, "IssueType", 9);
    msgpack_pack_array(&pk, 1);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    msgpack_pack_str(&pk, 11);
    msgpack_pack_str_body(&pk, "subdoc_name", 11);
    msgpack_pack_str(&pk, 9);
    msgpack_pack_str_body(&pk, "test_name", 9);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "version", 7);
    msgpack_pack_uint32(&pk, 1);
    msgpack_pack_str(&pk, 14);
    msgpack_pack_str_body(&pk, "transaction_id", 14);
    msgpack_pack_uint16(&pk, 123);
    remotedebuggerdoc_t *result = remotedebuggerdoc_convert(sbuf.data, sbuf.size);

    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result->subdoc_name, "test_name");
    EXPECT_EQ(result->version, 1u);
    EXPECT_EQ(result->transaction_id, 123u);
    ASSERT_NE(result->param, nullptr);
    EXPECT_STREQ(result->param->commandList, "value");

    remotedebuggerdoc_destroy(result);
    msgpack_sbuffer_destroy(&sbuf);
}

/* --------------- Test decodeWebCfgData() from rrdMsgPackDecoder --------------- */
class DecodeWebCfgDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        g_mockBase64 = new MockBase64();
    }
    void TearDown() override
    {
        delete g_mockBase64;
        g_mockBase64 = nullptr;
    }
};

TEST_F(DecodeWebCfgDataTest, Base64DecodeFails)
{
    char *pString = nullptr;
    int result = decodeWebCfgData(pString);
    EXPECT_EQ(result, -1);
}

/* TODO */
// TEST_F(DecodeWebCfgDataTest, MsgpackUnpackFail) {
// }

// TEST_F(DecodeWebCfgDataTest, MsgpackUnpackSuccess) {
// }

/* ====================== rrdMain ================*/
/* --------------- Test RRDEventThreadFunc() aka main() from rrdMain --------------- */
class RRDEventThreadFuncTest : public ::testing::Test
{
protected:
    int msqid_cpy;
    key_t key_cpy;

    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }

    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name != "MessageReceiveFailure"){
            msqid_cpy = msqid;
            key_cpy = key;
            msqid = msgget(key, IPC_CREAT | 0666);
            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
        }
    }
    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name != "MessageReceiveFailure"){
            int ret = msgctl(msqid, IPC_RMID, nullptr);
            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";
            msqid = msqid_cpy;
            key = key_cpy;
        }
    }
};

TEST_F(RRDEventThreadFuncTest, MessageReceiveFailure) {
    void *arg = nullptr;
    EXPECT_EQ(RRDEventThreadFunc(arg), arg);
}

TEST_F(RRDEventThreadFuncTest, MessageReceiveSuccessDefaultType) {
    data_buf rbuf;
    rbuf.mtype = DEFAULT;
    rbuf.mdata = strdup("Test Message");
    msgRRDHdr msgHdr;
    msgHdr.mbody = malloc(sizeof(data_buf));
    ASSERT_NE(msgHdr.mbody, nullptr);
    memcpy(msgHdr.mbody, &rbuf, sizeof(data_buf));
    ASSERT_NE(msgsnd(msqid, &msgHdr, sizeof(void *), 0), -1);
    void *arg = nullptr;

    EXPECT_EQ(RRDEventThreadFunc(arg), arg);
}

TEST_F(RRDEventThreadFuncTest, MessageReceiveSuccessEventMsgType) {
    data_buf rbuf;
    rbuf.mtype = EVENT_MSG;
    rbuf.mdata = strdup("Test");
    rbuf.inDynamic = true;
    rbuf.jsonPath = nullptr;
    msgRRDHdr msgHdr;
    msgHdr.mbody = malloc(sizeof(data_buf));
    ASSERT_NE(msgHdr.mbody, nullptr);
    memcpy(msgHdr.mbody, &rbuf, sizeof(data_buf));
    ASSERT_NE(msgsnd(msqid, &msgHdr, sizeof(void *), 0), -1);
    void *arg = nullptr;

    EXPECT_EQ(RRDEventThreadFunc(arg), arg);
}

TEST_F(RRDEventThreadFuncTest, MessageReceiveSuccessWebCfgMsgType) {
    data_buf rbuf;
    rbuf.mtype = EVENT_WEBCFG_MSG;
    rbuf.mdata = nullptr;
    msgRRDHdr msgHdr;
    msgHdr.mbody = malloc(sizeof(data_buf));
    ASSERT_NE(msgHdr.mbody, nullptr);
    memcpy(msgHdr.mbody, &rbuf, sizeof(data_buf));
    ASSERT_NE(msgsnd(msqid, &msgHdr, sizeof(void *), 0), -1);
    void *arg = nullptr;

    EXPECT_EQ(RRDEventThreadFunc(arg), arg);
}

TEST_F(RRDEventThreadFuncTest, MessageReceiveSuccessDeepSleepEventType) {
    data_buf rbuf;
    rbuf.mtype = DEEPSLEEP_EVENT_MSG;
    rbuf.mdata = nullptr;
    msgRRDHdr msgHdr;
    msgHdr.mbody = malloc(sizeof(data_buf));
    ASSERT_NE(msgHdr.mbody, nullptr);
    memcpy(msgHdr.mbody, &rbuf, sizeof(data_buf));
    ASSERT_NE(msgsnd(msqid, &msgHdr, sizeof(void *), 0), -1);
    void *arg = nullptr;

    EXPECT_EQ(RRDEventThreadFunc(arg), arg);
}

#ifdef IARMBUS_SUPPORT
/* --------------- Test shadowMain() aka main() from rrdMain --------------- */
TEST(shadowMainTest, MaintTest) {
    int msqid_cpy;
    key_t key_cpy;
    msqid_cpy = msqid;
    key_cpy = key;
    msqid = msgget(key, IPC_CREAT | 0666);
    ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
    data_buf rbuf;
    rbuf.mtype = DEFAULT;
    rbuf.mdata = strdup("Test Message");
    msgRRDHdr msgHdr;
    msgHdr.mbody = malloc(sizeof(data_buf));
    ASSERT_NE(msgHdr.mbody, nullptr);
    memcpy(msgHdr.mbody, &rbuf, sizeof(data_buf));
    ASSERT_NE(msgsnd(msqid, &msgHdr, sizeof(void *), 0), -1);
    void *arg = nullptr;

    EXPECT_EQ(shadowMain(arg), 0);

    int ret = msgctl(msqid, IPC_RMID, nullptr);
    ASSERT_NE(ret, -1) << "Error removing message queue used for testing";
    msqid = msqid_cpy;
    key = key_cpy;
}
#endif

/* ================== Gtest Main ======================== */
GTEST_API_ main(int argc, char *argv[])
{
    char testresults_fullfilepath[GTEST_REPORT_FILEPATH_SIZE];
    char buffer[GTEST_REPORT_FILEPATH_SIZE];

    memset( testresults_fullfilepath, 0, GTEST_REPORT_FILEPATH_SIZE );
    memset( buffer, 0, GTEST_REPORT_FILEPATH_SIZE );

    snprintf( testresults_fullfilepath, GTEST_REPORT_FILEPATH_SIZE, "json:%s%s" , GTEST_DEFAULT_RESULT_FILEPATH , GTEST_DEFAULT_RESULT_FILENAME);
    ::testing::GTEST_FLAG(output) = testresults_fullfilepath;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* ====================== rrdIarm ================*/
/* --------------- Test getBlobVersion() from rrdIarm --------------- */
extern uint32_t gWebCfgBloBVersion;
namespace
{
    TEST(SetBlobVersionTest, SetsGlobalVariable)
    {
        char subdoc[] = "test_subdoc";
        uint32_t version = 5;
        int result = setBlobVersion(subdoc, version);

        EXPECT_EQ(result, 0);
        EXPECT_EQ(gWebCfgBloBVersion, version);
    }
    TEST(GetBlobVersionTest, ReturnsGlobalVariable)
    {
        char subdoc[] = "test_subdoc";
        uint32_t result = getBlobVersion(subdoc);

        EXPECT_EQ(result, gWebCfgBloBVersion);
    }
}

/* --------------- Test RRD_data_buff_init() from rrdIarm --------------- */
TEST(RRDDataBuffInitTest, InitializeDataBuff)
{
    data_buf sbuf;
    message_type_et sndtype = EVENT_MSG;
    deepsleep_event_et deepSleepEvent = RRD_DEEPSLEEP_RDM_DOWNLOAD_PKG_INITIATE;
    RRD_data_buff_init(&sbuf, sndtype, deepSleepEvent);

    EXPECT_EQ(sbuf.mtype, sndtype);
    EXPECT_EQ(sbuf.mdata, nullptr);
    EXPECT_EQ(sbuf.jsonPath, nullptr);
    EXPECT_FALSE(sbuf.inDynamic);
    EXPECT_EQ(sbuf.dsEvent, deepSleepEvent);
}

/* --------------- Test RRD_data_buff_deAlloc() from rrdIarm --------------- */
TEST(RRDDataBuffDeAllocTest, DeallocateDataBuff)
{
    data_buf *sbuf = (data_buf *)malloc(sizeof(data_buf));
    sbuf->mdata = (char *)malloc(10 * sizeof(char));
    sbuf->jsonPath = (char *)malloc(10 * sizeof(char));

    ASSERT_NO_FATAL_FAILURE(RRD_data_buff_deAlloc(sbuf));
}

TEST(RRDDataBuffDeAllocTest, NullPointer)
{
    data_buf *sbuf = nullptr;

    ASSERT_NO_FATAL_FAILURE(RRD_data_buff_deAlloc(sbuf));
}

/* --------------- Test RRD_unsubscribe() from rrdIarm --------------- */

class RRDUnsubscribeTest : public ::testing::Test
{
protected:
    ClientIARMMock mock;

    void SetUp() override
    {
        setMock(&mock);
    }

    void TearDown() override
    {
        setMock(nullptr);
    }
};

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_Success)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    //EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_SUCCESS);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_DisconnectFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_TermFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterEventHandlerFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterRDMMgrEventHandlerRRDFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    //EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

TEST_F(RRDUnsubscribeTest, TestRRD_Unsubscribe_UnRegisterPwrMgrEventHandlerFailure)
{
    EXPECT_CALL(mock, IARM_Bus_Disconnect()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_Term()).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    //EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDK_REMOTE_DEBUGGER_NAME, IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_RDMMGR_NAME, IARM_BUS_RDMMGR_EVENT_APP_INSTALLATION_STATUS)).WillOnce(::testing::Return(IARM_RESULT_SUCCESS));
    EXPECT_CALL(mock, IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED)).WillOnce(::testing::Return(IARM_RESULT_FAILURE));
    int result = RRD_unsubscribe();

    EXPECT_EQ(result, IARM_RESULT_FAILURE);
}

/* --------------- Test webconfigFrameworkInit() from rrdIarm --------------- */
class WebConfigIntegrationTest : public ::testing::Test
{
protected:
    ClientWebConfigMock mock_webconfig;

    void SetUp() override
    {
        setWebConfigMock(&mock_webconfig);
    }

    void TearDown() override
    {
        setWebConfigMock(nullptr);
    }
};

TEST_F(WebConfigIntegrationTest, TestWebconfigFrameworkInit)
{
    EXPECT_CALL(mock_webconfig, register_sub_docs_mock(_, _, _, _)).Times(1);
    webconfigFrameworkInit();
}

/* --------------- Test _pwrManagerEventHandler() from rrdIarm --------------- */
class PwrMgrEventHandlerTest : public ::testing::Test
{
protected:
    MockRBusApi mock_rbus_api;
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    void SetUp() override
    {
        RBusApiWrapper::setImpl(&mock_rbus_api);
        /*string test_name = getCurrentTestName();
        if (test_name == "TestCurrentStateDeepSleepRBusOpenFail" || test_name == "TestCurrentStateDeepSleepRBusOpenSuccessRbusSetFail")
        {
            RBusApiWrapper::setImpl(&mock_rbus_api);
        } */
    }
    void TearDown() override
    {
        RBusApiWrapper::clearImpl();
        /*
        string test_name = getCurrentTestName();
        if (test_name == "TestCurrentStateDeepSleepRBusOpenFail" || test_name == "TestCurrentStateDeepSleepRBusOpenSuccessRbusSetFail")
        {
            RBusApiWrapper::clearImpl();
        } */
    }
};

TEST_F(PwrMgrEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _pwrManagerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateNotDeepSleep)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenSuccessRbusSetSuccess)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;

    //EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

/*
TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenFail)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _))
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}

TEST_F(PwrMgrEventHandlerTest, TestCurrentStateDeepSleepRBusOpenSuccessRbusSetFail)
{
    const char *owner = IARM_BUS_PWRMGR_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;

    //EXPECT_CALL(mock_rbus_api, rbus_open(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_Init(_)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbusValue_SetString(_, _)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mock_rbus_api, rbus_set(_, _, _, _)).WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    _pwrManagerEventHandler(owner, eventId, &eventData, sizeof(eventData));
}


*/


/*
/* --------------- Test _remoteDebuggerEventHandler() from rrdIarm --------------- */

/*
class RemoteDebuggerEventHandlerTest : public ::testing::Test
{
protected:
    string getCurrentTestName()
    {
        const testing::TestInfo *const test_info = testing::UnitTest::GetInstance()->current_test_info();
        return test_info->name();
    }
    int msqid_cpy;
    key_t key_cpy;
    void SetUp() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            msqid_cpy = msqid;
            key_cpy = key;
            msqid = msgget(key, IPC_CREAT | 0666);

            ASSERT_NE(msqid, -1) << "Error creating message queue for testing";
        }
    }

    void TearDown() override
    {
        string test_name = getCurrentTestName();
        if (test_name == "TestPushIssueTypesToMsgQueueSuccess")
        {
            int ret = msgctl(msqid, IPC_RMID, nullptr);
            ASSERT_NE(ret, -1) << "Error removing message queue used for testing";

            msqid = msqid_cpy;
            key = key_cpy;
        }
    }
};

TEST_F(RemoteDebuggerEventHandlerTest, TestPushIssueTypesToMsgQueueSuccess)
{
    const char *owner = RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "mdata";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
    data_buf receivedBuf;
    int ret = msgrcv(msqid, &receivedBuf, sizeof(receivedBuf), EVENT_MSG, 0);

    ASSERT_NE(ret, -1) << "Error receiving message from queue";
}

TEST_F(RemoteDebuggerEventHandlerTest, TestInvalidOwnerName)
{
    const char *owner = "InvalidOwner";
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_ISSUETYPE;
    char data[] = "Test data";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
}

TEST_F(RemoteDebuggerEventHandlerTest, TestInvalidEventId)
{
    const char *owner = RDK_REMOTE_DEBUGGER_NAME;
    IARM_EventId_t eventId = IARM_BUS_RDK_REMOTE_DEBUGGER_MAX_EVENT; // Invalid event id
    char data[] = "Test data";
    _remoteDebuggerEventHandler(owner, eventId, data, sizeof(data));
}
*/
