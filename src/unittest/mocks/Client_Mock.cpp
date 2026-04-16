/* ====================== rrd_SetHandler and rrd_GetHandler ================*/

// Simple mock for RBUS profile handler tests
class RBusProfileMock {
public:
    std::string mockPropertyName;
    std::string mockPropertyValue;
    rbusValueType_t mockValueType = RBUS_STRING;
    std::string mockResponseValue;
};

// Global mock RBUS property for profile handler tests
struct MockRBusProperty {
    std::string name;
    std::string value;
    rbusValueType_t type;
} g_mockRbusProperty;

// Mock RBUS function implementations for profile handler tests
static char const* mock_rbusProperty_GetName(rbusProperty_t property) {
    (void)property;
    return g_mockRbusProperty.name.c_str();
}

static rbusValue_t mock_rbusProperty_GetValue(rbusProperty_t property) {
    (void)property;
    return (rbusValue_t)g_mockRbusProperty.value.c_str();
}

static rbusValueType_t mock_rbusValue_GetType(rbusValue_t value) {
    (void)value;
    return g_mockRbusProperty.type;
}

static char const* mock_rbusValue_GetString(rbusValue_t value, int* len) {
    (void)value;
    if (len) *len = g_mockRbusProperty.value.length();
    return g_mockRbusProperty.value.c_str();
}

static void mock_rbusProperty_SetValue(rbusProperty_t property, rbusValue_t value) {
    (void)property; (void)value;
}

static void mock_rbusValue_Release(rbusValue_t value) {
    (void)value;
}

// External declarations for function pointers from Client_Mock.cpp
extern char const* (*rbusProperty_GetName)(rbusProperty_t);
extern rbusValue_t (*rbusProperty_GetValue)(rbusProperty_t);
extern rbusValueType_t (*rbusValue_GetType)(rbusValue_t);
extern char const* (*rbusValue_GetString)(rbusValue_t, int*);
extern void (*rbusProperty_SetValue)(rbusProperty_t, rbusValue_t);
extern void (*rbusValue_Release)(rbusValue_t);

// Test fixture for RRD Profile Handler tests
class RRDProfileHandlerTest : public ::testing::Test {
protected:
    RBusProfileMock mockRBusApi;
    MockRBusApi mockWrapper; // Add mock for RBusApiWrapper

    // Store original function pointers
    char const* (*orig_rbusProperty_GetName)(rbusProperty_t);
    rbusValue_t (*orig_rbusProperty_GetValue)(rbusProperty_t);
    rbusValueType_t (*orig_rbusValue_GetType)(rbusValue_t);
    char const* (*orig_rbusValue_GetString)(rbusValue_t, int*);
    void (*orig_rbusProperty_SetValue)(rbusProperty_t, rbusValue_t);
    void (*orig_rbusValue_Release)(rbusValue_t);

    void SetUp() override {
        // Reset global state
        memset(RRDProfileCategory, 0, sizeof(RRDProfileCategory));
        strcpy(RRDProfileCategory, "all");

        // Reset mock RBUS data
        mockRBusApi.mockPropertyName.clear();
        mockRBusApi.mockPropertyValue.clear();
        mockRBusApi.mockValueType = RBUS_STRING;
        mockRBusApi.mockResponseValue.clear();

        // Reset global mock property
        g_mockRbusProperty.name.clear();
        g_mockRbusProperty.value.clear();
        g_mockRbusProperty.type = RBUS_STRING;

        // Clear any existing RBusApiWrapper implementation first
        RBusApiWrapper::clearImpl();

        // Set up RBusApiWrapper with mock implementation
        RBusApiWrapper::setImpl(&mockWrapper);

        // Set up expectations for common RBUS operations
        EXPECT_CALL(mockWrapper, rbusValue_Init(testing::_))
            .WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mockWrapper, rbusValue_SetString(testing::_, testing::_))
            .WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
        EXPECT_CALL(mockWrapper, rbusProperty_SetValue(testing::_, testing::_))
            .WillRepeatedly(testing::Return());
        EXPECT_CALL(mockWrapper, rbusValue_Release(testing::_))
            .WillRepeatedly(testing::Return());

        // Store original function pointers
        orig_rbusProperty_GetName = rbusProperty_GetName;
        orig_rbusProperty_GetValue = rbusProperty_GetValue;
        orig_rbusValue_GetType = rbusValue_GetType;
        orig_rbusValue_GetString = rbusValue_GetString;
        orig_rbusProperty_SetValue = rbusProperty_SetValue;
        orig_rbusValue_Release = rbusValue_Release;

        // Redirect to mock implementations
        rbusProperty_GetName = mock_rbusProperty_GetName;
        rbusProperty_GetValue = mock_rbusProperty_GetValue;
        rbusValue_GetType = mock_rbusValue_GetType;
        rbusValue_GetString = mock_rbusValue_GetString;
        rbusProperty_SetValue = mock_rbusProperty_SetValue;
        rbusValue_Release = mock_rbusValue_Release;
    }

    void TearDown() override {
        // Clean up test files
        unlink(RRD_PROFILE_CATEGORY_FILE);

        // Reset global state
        memset(RRDProfileCategory, 0, sizeof(RRDProfileCategory));
        strcpy(RRDProfileCategory, "all");

        // Reset global mock property properly (don't use memset on C++ objects)
        g_mockRbusProperty.name.clear();
        g_mockRbusProperty.value.clear();
        g_mockRbusProperty.type = RBUS_STRING;

        // Clear RBusApiWrapper implementation
        RBusApiWrapper::clearImpl();

        // Restore original function pointers
        rbusProperty_GetName = orig_rbusProperty_GetName;
        rbusProperty_GetValue = orig_rbusProperty_GetValue;
        rbusValue_GetType = orig_rbusValue_GetType;
        rbusValue_GetString = orig_rbusValue_GetString;
        rbusProperty_SetValue = orig_rbusProperty_SetValue;
        rbusValue_Release = orig_rbusValue_Release;
    }
};

/* --------------- Test rrd_SetHandler() --------------- */

TEST_F(RRDProfileHandlerTest, SetHandler_ValidStringAll)
{
    // Setup mock RBUS property
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "all";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusSetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_SetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, "all");
}

TEST_F(RRDProfileHandlerTest, SetHandler_ValidStringCategory)
{
    // Setup mock RBUS property
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "Video";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusSetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_SetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, "Video");
}

TEST_F(RRDProfileHandlerTest, SetHandler_StringTooLong)
{
    // Create a string longer than 255 characters
    std::string longString(300, 'A');

    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = longString;
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusSetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_SetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_INVALID_INPUT);
    // RRDProfileCategory should remain unchanged
    EXPECT_STREQ(RRDProfileCategory, "all");
}

TEST_F(RRDProfileHandlerTest, SetHandler_InvalidType)
{
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "Network";
    g_mockRbusProperty.type = RBUS_INT32; // Invalid type for this parameter

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusSetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_SetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_INVALID_INPUT);
    EXPECT_STREQ(RRDProfileCategory, "all");
}

TEST_F(RRDProfileHandlerTest, SetHandler_WrongPropertyName)
{
    g_mockRbusProperty.name = "wrong.property.name";
    g_mockRbusProperty.value = "Audio";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusSetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_SetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_INVALID_INPUT);
    EXPECT_STREQ(RRDProfileCategory, "all");
}

/* --------------- Test rrd_GetHandler() --------------- */

TEST_F(RRDProfileHandlerTest, GetHandler_AllCategories)
{
    // Override the filename in get handler to use our test JSON
    // We'll need to modify the function to accept a test file path

    strcpy(RRDProfileCategory, "all");

    g_mockRbusProperty.name = RRD_GET_PROFILE_EVENT;
    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusGetHandlerOptions_t* opts = nullptr;

    // Note: The actual function reads from "/etc/rrd/remote_debugger.json"
    // For testing, we would need to either:
    // 1. Create that file with test data, or
    // 2. Modify the function to accept a test file parameter
    // For now, we'll test the logic with a file that doesn't exist
    rbusError_t result = rrd_GetHandler(nullptr, mockProp, opts);

    // Expect BUS_ERROR because test file doesn't exist at expected location
    EXPECT_EQ(result, RBUS_ERROR_BUS_ERROR);
}

TEST_F(RRDProfileHandlerTest, GetHandler_SpecificCategory)
{
    strcpy(RRDProfileCategory, "Network");

    g_mockRbusProperty.name = RRD_GET_PROFILE_EVENT;
    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusGetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_GetHandler(nullptr, mockProp, opts);

    // Expect BUS_ERROR because test file doesn't exist at expected location
    EXPECT_EQ(result, RBUS_ERROR_BUS_ERROR);
}

TEST_F(RRDProfileHandlerTest, GetHandler_WrongPropertyName)
{
    g_mockRbusProperty.name = "wrong.property.name";
    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusGetHandlerOptions_t* opts = nullptr;

    rbusError_t result = rrd_GetHandler(nullptr, mockProp, opts);

    EXPECT_EQ(result, RBUS_ERROR_INVALID_INPUT);
}

/* --------------- Test helper functions --------------- */

TEST_F(RRDProfileHandlerTest, ReadProfileJsonFile_ValidFile)
{
    long file_size = 0;
    const char* filepath = find_test_file("profileTestValid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestValid.json in any search path";

    char* result = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(file_size, 0);
    EXPECT_NE(strstr(result, "Video"), nullptr);
    EXPECT_NE(strstr(result, "Audio"), nullptr);
    EXPECT_NE(strstr(result, "Network"), nullptr);
    EXPECT_NE(strstr(result, "System"), nullptr);

    free(result);
}

TEST_F(RRDProfileHandlerTest, ReadProfileJsonFile_NonExistentFile)
{
    long file_size = 0;
    char* result = read_profile_json_file("/nonexistent/file.json", &file_size);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(file_size, 0);
}
/*
TEST_F(RRDProfileHandlerTest, HasDirectCommands_ValidStructure)
{
    // Parse our test JSON
    long file_size = 0;
    const char* filepath = find_test_file("profileTestValid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestValid.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    ASSERT_NE(json, nullptr);

    cJSON* videoCategory = cJSON_GetObjectItem(json, "Video");
    ASSERT_NE(videoCategory, nullptr);

    bool result = has_direct_commands(videoCategory);
    EXPECT_TRUE(result);

    cJSON_Delete(json);
    free(jsonBuffer);
}

TEST_F(RRDProfileHandlerTest, HasDirectCommands_EmptyCategory)
{
    // Test with empty JSON
    long file_size = 0;
    const char* filepath = find_test_file("profileTestEmpty.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestEmpty.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    ASSERT_NE(json, nullptr);

    bool result = has_direct_commands(json);
    EXPECT_FALSE(result);

    cJSON_Delete(json);
    free(jsonBuffer);
}

TEST_F(RRDProfileHandlerTest, GetAllCategoriesJson_ValidInput)
{
    // Parse our test JSON
    long file_size = 0;
    const char* filepath = find_test_file("profileTestValid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestValid.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    ASSERT_NE(json, nullptr);

    char* result = get_all_categories_json(json);
    ASSERT_NE(result, nullptr);

    // Check that the result contains the expected categories
    EXPECT_NE(strstr(result, "Video"), nullptr);
    EXPECT_NE(strstr(result, "Audio"), nullptr);
    EXPECT_NE(strstr(result, "Network"), nullptr);
    EXPECT_NE(strstr(result, "System"), nullptr);

    cJSON_Delete(json);
    free(jsonBuffer);
    free(result);
}

TEST_F(RRDProfileHandlerTest, GetSpecificCategoryJson_ValidCategory)
{
    // Parse our test JSON
    long file_size = 0;
    const char* filepath = find_test_file("profileTestValid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestValid.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    ASSERT_NE(json, nullptr);

    char* result = get_specific_category_json(json, "Video");
    ASSERT_NE(result, nullptr);

    // Check that the result contains Video issue types
    EXPECT_NE(strstr(result, "VideoDecodeFailure"), nullptr);
    EXPECT_NE(strstr(result, "VideoFreeze"), nullptr);
    EXPECT_NE(strstr(result, "VideoArtifacts"), nullptr);
    // Should not contain other categories
    EXPECT_EQ(strstr(result, "AudioLoss"), nullptr);

    cJSON_Delete(json);
    free(jsonBuffer);
    free(result);
}
*/

TEST_F(RRDProfileHandlerTest, GetSpecificCategoryJson_InvalidCategory)
{
    // Parse our test JSON
    long file_size = 0;
    const char* filepath = find_test_file("profileTestValid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestValid.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    ASSERT_NE(json, nullptr);

    char* result = get_specific_category_json(json, "NonExistentCategory");
    ASSERT_NE(result, nullptr);

    // Should return empty array
    EXPECT_NE(strstr(result, "[]"), nullptr);

    cJSON_Delete(json);
    free(jsonBuffer);
    free(result);
}

/* --------------- Test JSON parsing error handling --------------- */

TEST_F(RRDProfileHandlerTest, ParseInvalidJson)
{
    // Test with invalid JSON file
    long file_size = 0;
    const char* filepath = find_test_file("profileTestInvalid.json");
    ASSERT_NE(filepath, nullptr) << "Could not find profileTestInvalid.json in any search path";

    char* jsonBuffer = read_profile_json_file(filepath, &file_size);

    ASSERT_NE(jsonBuffer, nullptr);

    cJSON* json = cJSON_Parse(jsonBuffer);
    EXPECT_EQ(json, nullptr); // Should fail to parse

    // Clean up
    free(jsonBuffer);
}

TEST_F(RRDProfileHandlerTest, SetRbusResponse_ValidInput)
{
    g_mockRbusProperty.name = RRD_GET_PROFILE_EVENT;
    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;

    const char* testJson = "{\"test\": \"value\"}";

    rbusError_t result = set_rbus_response(mockProp, testJson);

    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
    // Note: Response verification depends on implementation
}

TEST_F(RRDProfileHandlerTest, SetRbusResponse_NullInput)
{
    g_mockRbusProperty.name = RRD_GET_PROFILE_EVENT;
    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;

    rbusError_t result = set_rbus_response(mockProp, nullptr);

    EXPECT_EQ(result, RBUS_ERROR_BUS_ERROR);
}

/* --------------- Test profile category file operations --------------- */

TEST_F(RRDProfileHandlerTest, SaveAndLoadProfileCategory)
{
    // Test saving a category
    strcpy(RRDProfileCategory, "Network");
    int saveResult = save_profile_category();
    EXPECT_EQ(saveResult, 0);

    // Clear the global variable
    memset(RRDProfileCategory, 0, sizeof(RRDProfileCategory));
    strcpy(RRDProfileCategory, "default");

    // Test loading the category
    int loadResult = load_profile_category();
    EXPECT_EQ(loadResult, 0);
    EXPECT_STREQ(RRDProfileCategory, "Network");
}

TEST_F(RRDProfileHandlerTest, LoadProfileCategory_NoFile)
{
    // Ensure file doesn't exist
    unlink(RRD_PROFILE_CATEGORY_FILE);

    int result = load_profile_category();
    EXPECT_NE(result, 0);
    EXPECT_STREQ(RRDProfileCategory, "all");
}

/* --------------- Integration tests for complete workflow --------------- */

TEST_F(RRDProfileHandlerTest, SetAndGetWorkflow_AllCategories)
{
    // Test complete workflow: set "all" -> get should return all categories

    // Step 1: Set profile category to "all"
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "all";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockSetProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusError_t setResult = rrd_SetHandler(nullptr, mockSetProp, nullptr);

    EXPECT_EQ(setResult, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, "all");

    // Step 2: Get profile data (will fail because file doesn't exist at expected path)
    g_mockRbusProperty.name = RRD_GET_PROFILE_EVENT;
    rbusProperty_t mockGetProp = (rbusProperty_t)&g_mockRbusProperty;

    rbusError_t getResult = rrd_GetHandler(nullptr, mockGetProp, nullptr);
    EXPECT_EQ(getResult, RBUS_ERROR_BUS_ERROR); // Expected since file doesn't exist
}

TEST_F(RRDProfileHandlerTest, SetAndGetWorkflow_SpecificCategory)
{
    // Test complete workflow: set "System" -> get should return System category only

    // Step 1: Set profile category to specific category
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "System";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockSetProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusError_t setResult = rrd_SetHandler(nullptr, mockSetProp, nullptr);

    EXPECT_EQ(setResult, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, "System");

    // Step 2: Verify the category was persisted
    // Clear global and reload from file
    strcpy(RRDProfileCategory, "default");
    load_profile_category();
    EXPECT_STREQ(RRDProfileCategory, "System");
}

/* --------------- Boundary and stress tests --------------- */

TEST_F(RRDProfileHandlerTest, SetHandler_EmptyString)
{
    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = "";
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&mockRBusApi;
    rbusError_t result = rrd_SetHandler(nullptr, mockProp, nullptr);

    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, "");
}

TEST_F(RRDProfileHandlerTest, SetHandler_MaxLengthString)
{
    // Create a string of exactly 255 characters (max allowed)
    std::string maxString(255, 'A');

    g_mockRbusProperty.name = RRD_SET_PROFILE_EVENT;
    g_mockRbusProperty.value = maxString;
    g_mockRbusProperty.type = RBUS_STRING;

    rbusProperty_t mockProp = (rbusProperty_t)&g_mockRbusProperty;
    rbusError_t result = rrd_SetHandler(nullptr, mockProp, nullptr);

    EXPECT_EQ(result, RBUS_ERROR_SUCCESS);
    EXPECT_STREQ(RRDProfileCategory, maxString.c_str());
}
