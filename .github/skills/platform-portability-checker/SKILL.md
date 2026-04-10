---
name: platform-portability-checker
description: Verify C/C++ code is platform-independent and portable across embedded platforms. Use when reviewing code for cross-platform deployment or preparing for new hardware targets.
---

# Platform Portability Checker

## Purpose

Ensure C/C++ code is portable across different embedded platforms, architectures, and operating systems without modification.

## When to Use

- Reviewing new code before merge
- Porting to new hardware platform
- Preparing release for multiple architectures
- Investigating platform-specific bugs
- Refactoring legacy platform-specific code

## Portability Checklist

### 1. Integer Types

**Check for**: Use of `int`, `long`, `short` without fixed sizes

```c
// PROBLEM: Size varies by platform
int counter;       // 16, 32, or 64 bits?
long timestamp;    // 32 or 64 bits?
short flag;        // 16 bits on most, but not guaranteed

// FIX: Use stdint.h types
#include <stdint.h>

uint32_t counter;     // Always 32 bits
uint64_t timestamp;   // Always 64 bits  
uint16_t flag;        // Always 16 bits

// For size_t operations
size_t length;        // Pointer-sized unsigned
ssize_t result;       // Pointer-sized signed
```

### 2. Pointer Assumptions

**Check for**: Pointer arithmetic, casting, size assumptions

```c
// PROBLEM: Assumes pointer == long
long ptr_value = (long)ptr;  // Fails on 64-bit with 32-bit long

// FIX: Use uintptr_t
#include <stdint.h>
uintptr_t ptr_value = (uintptr_t)ptr;

// PROBLEM: Pointer used as integer
if (ptr & 0x1) { ... }  // What size is ptr?

// FIX: Be explicit
if ((uintptr_t)ptr & 0x1) { ... }
```

### 3. Endianness

**Check for**: Multi-byte values sent over network or stored to disk

```c
// PROBLEM: Host byte order assumed
uint32_t value = 0x12345678;
fwrite(&value, 4, 1, file);  // Different on LE vs BE

// FIX: Explicit byte order
#include <arpa/inet.h>  // For htonl, ntohl

uint32_t host_value = 0x12345678;
uint32_t network_value = htonl(host_value);
fwrite(&network_value, 4, 1, file);

// For reading
uint32_t network_value;
fread(&network_value, 4, 1, file);
uint32_t host_value = ntohl(network_value);
```

### 4. Structure Packing

**Check for**: Structures sent over network or saved to disk

```c
// PROBLEM: Padding varies by platform
struct {
    uint8_t type;
    uint32_t value;  // Padding before this?
    uint16_t flags;  // Padding before this?
} data;

// FIX: Explicit packing
struct __attribute__((packed)) {
    uint8_t type;
    uint32_t value;
    uint16_t flags;
} data;

// Or control padding explicitly
struct {
    uint8_t type;
    uint8_t padding[3];  // Explicit padding
    uint32_t value;
    uint16_t flags;
    uint16_t padding2;
} data;
```

### 5. Boolean Type

**Check for**: Using int/char for boolean

```c
// PROBLEM: Non-standard boolean
int flag;  // Really 3 states: 0, 1, other
char enabled;  // Also used for booleans

// FIX: Use stdbool.h
#include <stdbool.h>

bool flag;
bool enabled;

if (flag) { ... }  // Clear intent
```

### 6. Character Sets

**Check for**: Assumptions about ASCII or character encoding

```c
// PROBLEM: Assumes ASCII
if (ch >= 'A' && ch <= 'Z') {
    ch = ch + 32;  // Convert to lowercase?
}

// FIX: Use standard functions
#include <ctype.h>

if (isupper(ch)) {
    ch = tolower(ch);
}
```

### 7. File Paths

**Check for**: Hard-coded path separators

```c
// PROBLEM: Unix-specific
const char* path = "/tmp/telemetry/data.log";

// FIX: Use platform-agnostic approach
#ifdef _WIN32
    #define PATH_SEP "\\"
    const char* tmp_dir = getenv("TEMP");
#else
    #define PATH_SEP "/"
    const char* tmp_dir = "/tmp";
#endif

char path[256];
snprintf(path, sizeof(path), "%s%stelemetry%sdata.log",
         tmp_dir, PATH_SEP, PATH_SEP);
```

### 8. System Calls

**Check for**: Platform-specific syscalls

```c
// PROBLEM: Linux-specific
#include <sys/epoll.h>
int fd = epoll_create(10);

// FIX: Abstraction layer
// In platform.h
#if defined(__linux__)
    #include "platform_linux.h"
#elif defined(__APPLE__)
    #include "platform_darwin.h"
#else
    #error "Unsupported platform"
#endif

// Each platform provides same interface
event_loop_t* create_event_loop(void);
```

### 9. Compiler Extensions

**Check for**: GCC/Clang specific features

```c
// PROBLEM: GCC-specific
typeof(x) y = x;
int array[0];  // Zero-length array

// FIX: Avoid compiler-specific typeof/__auto_type; use standard types
int y = x;  // declare with explicit type

// Or avoid non-standard features
// Define proper types instead
```

### 10. Include Paths

**Check for**: Platform-specific headers

```c
// PROBLEM: Assumes Linux headers
#include <linux/limits.h>

// FIX: Use standard headers or configure check
#ifdef HAVE_LINUX_LIMITS_H
    #include <linux/limits.h>
#else
    #include <limits.h>
#endif

// Or use autoconf to detect
// In configure.ac:
// AC_CHECK_HEADERS([linux/limits.h limits.h])
```

## Build System Integration

### configure.ac checks

```autoconf
# Check for required features
AC_C_BIGENDIAN
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([void *])

# Check for headers
AC_CHECK_HEADERS([stdint.h stdbool.h endian.h])

# Check for functions
AC_CHECK_FUNCS([htonl ntohl])

# Platform-specific code
case "$host" in
    *-linux*)
        AC_DEFINE([PLATFORM_LINUX], [1])
        ;;
    arm*|*-arm*)
        AC_DEFINE([PLATFORM_ARM], [1])
        ;;
esac
```

## Testing

### Cross-Compilation Test

```bash
# Test building for different architectures
./configure --host=arm-linux-gnueabihf
make clean && make

./configure --host=x86_64-linux-gnu
make clean && make

./configure --host=mips-linux-gnu
make clean && make
```

### Endianness Test

```c
// Test endianness handling
uint32_t value = 0x12345678;
uint32_t network = htonl(value);
uint32_t restored = ntohl(network);
assert(value == restored);

// Verify structure packing
assert(sizeof(packed_struct_t) == EXPECTED_SIZE);
```

## Output Format

```
## Platform Portability Analysis

### Critical Issues
1. [file.c:123] Using `long` for timestamp - not fixed width
2. [file.c:456] Writing struct directly to network - endianness issue
3. [file.c:789] Assuming 32-bit pointers

### Warnings
1. [file.c:234] Using int for boolean - prefer stdbool.h
2. [file.c:567] Hard-coded Unix path separator

### Recommendations
1. Add configure checks for required headers
2. Create platform abstraction layer
3. Test build on multiple architectures

### Suggested Fixes
[Specific code changes for each issue]
```

## Verification

- Code compiles on target platforms
- Tests pass on all platforms
- Static analysis clean
- No endianness issues
- No alignment issues
- Structure sizes verified
