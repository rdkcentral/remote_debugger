---
name: memory-safety-analyzer
description: Analyze C/C++ code for memory safety issues including leaks, use-after-free, buffer overflows, and provide fixes. Use when reviewing memory management, debugging crashes, or improving code safety.
---

# Memory Safety Analysis for Embedded C

## Purpose

Systematically analyze C/C++ code for memory safety issues that can cause crashes, security vulnerabilities, or resource exhaustion in embedded systems.

## Usage

Invoke this skill when:
- Reviewing new code with dynamic memory allocation
- Debugging memory-related crashes
- Analyzing legacy code for safety issues
- Preparing code for production deployment
- Investigating memory leaks or fragmentation

## Analysis Process

### Step 1: Identify All Allocations

Search the code for:
- `malloc`, `calloc`, `realloc`
- `strdup`, `strndup`
- `fopen`, `open`
- `pthread_create`, `pthread_mutex_init`
- Custom allocation functions

For each allocation, verify:
1. Return value is checked
2. Corresponding free/close exists
3. Error paths also free resources
4. No double-free possible

### Step 2: Check Pointer Lifetimes

For each pointer variable:
- When is it assigned?
- When is it freed?
- Can it be used after free?
- Can it outlive the data it points to?
- Is it NULL-initialized?
- Is it NULL-checked before use?

### Step 3: Analyze Error Paths

For each error return:
- Are all resources freed?
- Is cleanup done in correct order?
- Are error codes accurate?
- Is logging appropriate?

### Step 4: Review Buffer Operations

For string and memory operations:
- `strcpy` → should be `strncpy` with size check
- `sprintf` → should be `snprintf` with size
- `gets` → never use (remove immediately)
- `strcat` → verify buffer size
- `memcpy` → verify no overlap, validate size

### Step 5: Static Analysis

Run tools:
```bash
# Cppcheck
cppcheck --enable=all --inconclusive file.c

# Clang static analyzer
scan-build make

# Compiler warnings
gcc -Wall -Wextra -Werror file.c
```

### Step 6: Dynamic Analysis

Run valgrind:
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./program
```

## Common Issues and Fixes

### Issue: Unchecked malloc

```c
// PROBLEM
char* buffer = malloc(size);
strcpy(buffer, input);  // Crash if malloc failed

// FIX
char* buffer = malloc(size);
if (!buffer) {
    log_error("Failed to allocate %zu bytes", size);
    return ERR_NO_MEMORY;
}
strncpy(buffer, input, size - 1);
buffer[size - 1] = '\0';
```

### Issue: Memory leak on error

```c
// PROBLEM
int process() {
    char* buf = malloc(1024);
    FILE* f = fopen("file.txt", "r");
    
    if (!f) return -1;  // Leaked buf
    
    // ... process ...
    
    free(buf);
    fclose(f);
    return 0;
}

// FIX: Single exit with cleanup
int process() {
    int ret = 0;
    char* buf = NULL;
    FILE* f = NULL;
    
    buf = malloc(1024);
    if (!buf) {
        ret = ERR_NO_MEMORY;
        goto cleanup;
    }
    
    f = fopen("file.txt", "r");
    if (!f) {
        ret = ERR_FILE_OPEN;
        goto cleanup;
    }
    
    // ... process ...
    
cleanup:
    free(buf);
    if (f) fclose(f);
    return ret;
}
```

### Issue: Use after free

```c
// PROBLEM
free(ptr);
if (ptr->field > 0) { ... }  // Use after free!

// FIX
int value = ptr->field;
free(ptr);
ptr = NULL;
if (value > 0) { ... }
```

### Issue: Double free

```c
// PROBLEM
free(ptr);
// ... later ...
free(ptr);  // Double free!

// FIX: NULL after free
free(ptr);
ptr = NULL;
// ... later ...
free(ptr);  // Safe: free(NULL) is a no-op
```

### Issue: Buffer overflow

```c
// PROBLEM
char buffer[100];
strcpy(buffer, user_input);  // Overflow if input > 99 chars

// FIX
char buffer[100];
strncpy(buffer, user_input, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

## Output Format

Provide findings as:

```
## Memory Safety Analysis

### Critical Issues (must fix)
1. [file.c:123] Unchecked malloc - potential NULL dereference
2. [file.c:456] Memory leak on error path - buffer not freed
3. [file.c:789] Use after free - ptr used after free()

### Warnings (should fix)
1. [file.c:234] strcpy used - prefer strncpy
2. [file.c:567] Missing NULL check before pointer use

### Recommendations
1. Add cleanup label for resource management
2. Use RAII wrapper in tests
3. Run valgrind in CI pipeline

### Suggested Fixes
[Provide specific code changes for each issue]
```

## Verification

After fixes:
1. All static analysis warnings resolved
2. Valgrind shows no leaks
3. All tests pass
4. Code review by human
5. Memory footprint measured and acceptable
