# High Level Design: RDK Remote Debugger Profile Data RFC Parameters

## 1. Executive Summary

This document outlines the High Level Design for implementing RFC parameters to read remote debugger static profile data from RDK devices. The solution provides a standardized interface for retrieving supported RRD categories and issue types through RFC parameter management using RBUS framework.

## 2. Requirements

### 2.1 Functional Requirements
- **FR-1**: Create RFC parameters for profile data management:
  - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData`
  - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData`
- **FR-2**: Implement Set/Get handlers for data retrieval from static JSON file using RBUS framework
- **FR-3**: Support "all" categories query returning complete profile data
- **FR-4**: Support specific category-level queries returning filtered issue types
- **FR-5**: Provide persistent category selection across device reboots


## 3. Architecture Overview

### 3.1 System Context
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   RBUS CLI      │    │  RBUS Framework  │    │ RRD Application │
│   (rbuscli)     │◄──►│                  │◄──►│                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │
                                ▼
                       ┌──────────────────┐
                       │ Static JSON File │
                       │ (/etc/rrd/       │
                       │remote_debugger.  │
                       │     json)        │
                       └──────────────────┘
```

### 3.2 Component Architecture
```
┌─────────────────────────────────────────────────────────────────┐
│                    RRD Interface Layer                          │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │ rrd_SetHandler  │  │ rrd_GetHandler  │  │ Category Store  │ │
│  │                 │  │                 │  │ (File-based)    │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                    RBUS Data Elements                           │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐           ┌─────────────────────────────┐   │
│  │ SET Profile     │           │ GET Profile Data            │   │
│  │ Event Handler   │           │ Event Handler               │   │
│  └─────────────────┘           └─────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                      RBUS Framework                            │
└─────────────────────────────────────────────────────────────────┘
```

## 4. Detailed Design

### 4.1 Data Structures

#### 4.1.1 Global Variables
```c
// Profile category storage (persistent across sessions)
char RRDProfileCategory[256] = "all";

// RBUS handle for communication
rbusHandle_t rrdRbusHandle;

// RBUS data elements configuration
rbusDataElement_t profileDataElements[2];
```

#### 4.1.2 RBUS Data Elements
```c
profileDataElements[0] = {
    RRD_SET_PROFILE_EVENT,          // Set parameter name
    RBUS_ELEMENT_TYPE_PROPERTY,     // Property type
    DATA_HANDLER_SET_MACRO          // Set handler
};

profileDataElements[1] = {
    RRD_GET_PROFILE_EVENT,          // Get parameter name  
    RBUS_ELEMENT_TYPE_PROPERTY,     // Property type
    DATA_HANDLER_GET_MACRO          // Get handler
};
```

### 4.2 Core Functions

#### 4.2.1 Profile Category Management
```c
// Load stored category from persistent storage
int load_profile_category(void);

// Save current category to persistent storage  
int save_profile_category(void);
```

**Implementation Details:**
- **Storage Location**: `RRD_PROFILE_CATEGORY_FILE` (implementation-specific path)
- **Default Value**: "all" when no stored preference exists
- **Persistence**: Survives device reboots and service restarts
- **Error Handling**: Graceful fallback to default on read failures

#### 4.2.2 RBUS Registration
```c
// Register RBUS data elements during initialization
int RRD_subscribe() {
    // ... existing code ...
    
    // Load persistent category selection
    load_profile_category();
    
    // Register profile data elements
    ret = rbus_regDataElements(rrdRbusHandle, 2, profileDataElements);
    
    // ... error handling ...
}
```

### 4.3 RFC Parameter Handlers

#### 4.3.1 Set Handler (`rrd_SetHandler`)

**Purpose**: Handle category selection for profile data filtering

**Input Parameters**:
- `propertyName`: RFC parameter name
- `value`: Category name string (e.g., "all", "Video", "Audio")

**Processing Logic**:
1. **Validation**: Check string length (max 255 characters)
2. **Storage**: Update global `RRDProfileCategory` variable
3. **Persistence**: Save category to file using `save_profile_category()`
4. **Logging**: Record successful category change

**Return Values**:
- `RBUS_ERROR_SUCCESS`: Successful category update
- `RBUS_ERROR_INVALID_INPUT`: Invalid input parameters
- `RBUS_ERROR_BUS_ERROR`: File storage failure

#### 4.3.2 Get Handler (`rrd_GetHandler`)

**Purpose**: Retrieve profile data based on selected category

**Input Parameters**:
- `propertyName`: RFC parameter name
- `prop`: RBUS property for result assignment

**Processing Logic**:
1. **File Access**: Read static JSON file `/etc/rrd/remote_debugger.json`
2. **JSON Parsing**: Parse JSON content using cJSON library
3. **Category Processing**: 
   - **"all" Category**: Return all categories and their issue types
   - **Specific Category**: Return issue types for selected category only
4. **Response Generation**: Generate JSON response string
5. **RBUS Response**: Set property value with JSON result

**JSON Processing Algorithm**:
```
IF category == "all" OR category == "" THEN
    FOR each category in JSON:
        IF category has direct Commands (not nested):
            Extract all issue types
            Add to response object
        END IF
    END FOR
    Return complete categorized response
ELSE
    Find specific category in JSON
    IF category exists AND has direct Commands:
        Extract issue types for category
        Return array of issue types
    ELSE
        Return empty array
    END IF
END IF
```

### 4.4 JSON Data Structure

#### 4.4.1 Expected JSON Format
```json
{
  "Video": {
    "issueType1": {
      "Commands": "command_string",
      "Timeout": "timeout_value"
    },
    "issueType2": {
      "Commands": "command_string", 
      "Timeout": "timeout_value"
    }
  },
  "Audio": {
    "issueType3": {
      "Commands": "command_string",
      "Timeout": "timeout_value"
    }
  },
  "DeepSleep": {
    "SubCategory1": {
      "issueType4": {
        "Commands": "command_string",
        "Timeout": "timeout_value"  
      }
    }
  }
}
```

#### 4.4.2 Response Formats

**All Categories Response**:
```json
{
  "Video": ["issueType1", "issueType2"],
  "Audio": ["issueType3"]
}
```

**Specific Category Response**:
```json
["issueType1", "issueType2"]
```

## 5. API Specification

### 5.1 RFC Parameters

#### 5.1.1 Set Profile Data
- **Parameter**: `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData`
- **Type**: RBUS String Property (Read/Write)
- **Purpose**: Set category filter for profile data retrieval
- **Valid Values**: 
  - `"all"`: Return all supported categories
  - `"<CategoryName>"`: Return specific category data
- **Persistence**: Yes (survives reboots)

#### 5.1.2 Get Profile Data
- **Parameter**: `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData`
- **Type**: RBUS String Property (Read-Only)
- **Purpose**: Retrieve profile data based on current category selection
- **Response Format**: JSON string containing category/issue type mappings

### 5.2 Usage Examples

#### 5.2.1 RBUS CLI Usage
```bash
# Set category to "all" for complete profile data
rbuscli set  Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData string "all"

# Get complete profile data
rbuscli get Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData

# Set specific category
rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.setProfileData string "Video" 

# Get Video category data
rbuscli get Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.getProfileData
```

## 6. Error Handling

### 6.1 Set Handler Error Cases
| Error Condition | Return Code | Description |
|----------------|-------------|-------------|
| Invalid property name | `RBUS_ERROR_INVALID_INPUT` | Unknown parameter name |
| String too long | `RBUS_ERROR_INVALID_INPUT` | Category name exceeds 255 chars |
| Non-string value | `RBUS_ERROR_INVALID_INPUT` | Invalid data type |
| File write failure | `RBUS_ERROR_BUS_ERROR` | Persistent storage failure |

### 6.2 Get Handler Error Cases
| Error Condition | Return Code | Description |
|----------------|-------------|-------------|
| File not found | `RBUS_ERROR_BUS_ERROR` | JSON profile file missing |
| File read failure | `RBUS_ERROR_BUS_ERROR` | File system error |
| Invalid JSON | `RBUS_ERROR_SUCCESS` | Returns empty array |
| Memory allocation failure | `RBUS_ERROR_BUS_ERROR` | Insufficient memory |

## 7. Integration Points

### 7.1 RBUS Framework Integration
- **Registration**: Data elements registered during `RRD_subscribe()`
- **Deregistration**: Elements unregistered during `RRD_unsubscribe()`
- **Handle Management**: Uses global `rrdRbusHandle`

### 7.2 File System Dependencies
- **JSON File**: `/etc/rrd/remote_debugger.json` (read-only access required)
- **Category File**: `RRD_PROFILE_CATEGORY_FILE` (read/write access required)
- **Size Limits**: JSON file limited to 32KB for performance

### 7.3 Memory Management
- **Dynamic Allocation**: JSON buffer allocated based on file size
- **Cleanup**: All allocated memory properly freed
- **Error Recovery**: Memory freed on error conditions

## 8. Security Considerations

### 8.1 Input Validation
- **String Length**: Category names limited to 255 characters
- **Data Type**: Only string values accepted for set operations
- **JSON Size**: File size limited to prevent memory exhaustion

### 8.2 File Access Security
- **Read Permissions**: JSON file requires read access
- **Write Permissions**: Category file requires write access  
- **Path Validation**: File paths are compile-time constants

## 9. Performance Considerations

### 9.1 File I/O Optimization
- **Single Read**: JSON file read once per get operation
- **Size Limits**: 32KB maximum file size for reasonable performance
- **Memory Buffering**: Entire file loaded to memory for parsing

### 9.2 JSON Processing
- **Library**: Uses cJSON for efficient parsing
- **Memory Usage**: Temporary JSON objects created and cleaned up
- **Response Generation**: JSON response strings generated on-demand

## 10. Testing Strategy

### 10.1 Unit Testing
- **Set Handler**: Test category validation and persistence
- **Get Handler**: Test JSON parsing and response generation
- **File Operations**: Test read/write operations with various conditions

### 10.2 Integration Testing  
- **RBUS Communication**: Test RFC parameter access through RBUS using rbuscli
- **End-to-End**: Test complete workflows using rbuscli commands
- **Error Scenarios**: Test error handling and recovery

### 10.3 Test Cases
| Test Case | Input | Expected Output |
|-----------|-------|----------------|
| Set "all" category | `setProfileData="all"` | Success, category stored |
| Get all profiles | `getProfileData` | Complete JSON with all categories |
| Set specific category | `setProfileData="Video"` | Success, category stored |
| Get specific category | `getProfileData` | JSON array with Video issue types |
| Invalid category | `setProfileData="Invalid"` | Success, empty response on get |
| Missing JSON file | `getProfileData` | Error response |





---

**Document Version**: 1.0  
**Date**: April 8, 2026  
**Author**: RDK Development Team  
**Reviewed By**: Architecture Review Board
