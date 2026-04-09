---
applyTo: "unittest/**/*.cpp,unittest/**/*.h,uploadstblogs/unittest/**/*.cpp,uploadstblogs/unittest/**/*.h"
---

# C++ Testing Standards (Google Test)

## Test Framework

Use Google Test (gtest) and Google Mock (gmock) for all C++ test code.

## Test Organization

### File Structure
- One test file per source file: `foo.c` → `test/FooTest.cpp`
- Test fixtures for complex setups
- Mocks in separate files when reusable

```cpp
// GOOD: Test file structure
// filepath: unittest/dcm_utils_gtest.cpp

extern "C" {
#include "dcm_utils.h"
#include "dcm_types.h"
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class DcmUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test resources
    }
    
    void TearDown() override {
        // Clean up test resources
    }
};

TEST_F(DcmUtilsTest, ConfigFileReadWriteRoundTrip) {
    // Test configuration file parsing
    const char* config = "/tmp/test.conf";
    // verify read back value matches written value
    ASSERT_EQ(readConfigValue(config, "key"), "value");
}
```

## Testing Patterns

### Test C Code from C++
- Wrap C headers in `extern "C"` blocks
- Use RAII in tests for automatic cleanup
- Mock C functions using gmock when needed

```cpp
extern "C" {
#include "dcm_parseconf.h"
#include "dcm_rbus.h"
}

#include <gtest/gtest.h>

class DcmParseConfTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize handler stubs
    }
    
    void TearDown() override {
        // Clean up
    }
};

TEST_F(DcmParseConfTest, ParseConfigReturnsExpected) {
    DCMDHandle handle = {};
    // Test configuration parsing
    int result = dcmParseConfig(&handle, "/etc/dcmresponse.txt");
    // verify handler returns success and populates configuration
    ASSERT_EQ(result, 0);
}
```

### Memory Leak Testing
- All tests must pass valgrind
- Use RAII wrappers for C resources
- Verify cleanup in TearDown

```cpp
// GOOD: RAII wrapper for C resource
class FileHandle {
    FILE* file_;
public:
    explicit FileHandle(const char* path, const char* mode)
        : file_(fopen(path, mode)) {}
    
    ~FileHandle() {
        if (file_) fclose(file_);
    }
    
    FILE* get() const { return file_; }
    bool valid() const { return file_ != nullptr; }
};

TEST(FileTest, ReadConfig) {
    FileHandle file("/tmp/config.json", "r");
    ASSERT_TRUE(file.valid());
    // file automatically closed when test exits
}
```

### Mocking External Dependencies

```cpp
// GOOD: Mock for handler dependencies
class MockIniFile {
public:
    MOCK_METHOD(std::string, get, (const std::string& key));
    MOCK_METHOD(bool, set, (const std::string& key, const std::string& value));
};

TEST(HandlerTest, GetParamUsesIniFile) {
    MockIniFile mock;
    
    EXPECT_CALL(mock, get("Device.DeviceInfo.Manufacturer"))
        .WillOnce(testing::Return("TestVendor"));
    
    std::string result = mock.get("Device.DeviceInfo.Manufacturer");
    EXPECT_EQ("TestVendor", result);
}
```

## Test Quality Standards

### Coverage Requirements
- All public functions must have tests
- Test both success and failure paths
- Test boundary conditions
- Test error handling

### Test Naming
```cpp
// Pattern: TEST(ComponentName, BehaviorBeingTested)

TEST(Vector, CreateReturnsNonNull) { ... }
TEST(Vector, DestroyHandlesNull) { ... }
TEST(Vector, PushIncrementsSize) { ... }
TEST(Utils, ParseConfigInvalidJson) { ... }
```

### Assertions
- Use `ASSERT_*` when test can't continue after failure
- Use `EXPECT_*` when subsequent checks are still valuable
- Provide helpful failure messages

```cpp
// GOOD: Informative assertions
ASSERT_NE(nullptr, ptr) << "Failed to allocate " << size << " bytes";
EXPECT_EQ(expected, actual) << "Mismatch at index " << i;
EXPECT_TRUE(condition) << "Context: " << debug_info;
```

## Running Tests

### Build Tests
```bash
./configure --enable-gtest
make check
```

### Memory Checking
```bash
valgrind --leak-check=full --show-leak-kinds=all \
         ./unittest/dcm_gtest

valgrind --leak-check=full --show-leak-kinds=all \
         ./uploadstblogs/unittest/uploadstblogs_gtest
```

### Test Output
- Use `GTEST_OUTPUT=xml:results.xml` for CI integration
- Check return code: 0 = all passed
