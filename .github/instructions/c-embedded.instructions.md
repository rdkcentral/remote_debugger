---
applyTo: "**/*.c,**/*.h"
---

# C Programming Standards for Embedded Systems

## Memory Management

### Allocation Rules
- **Prefer stack allocation** for fixed-size, short-lived data
- **Use malloc/free** only when necessary; always pair them
- **Check all allocations**: Never assume malloc succeeds
- **Free in reverse order** of allocation to reduce fragmentation
- **Use memory pools** for frequent same-size allocations
- **Zero memory after free** to catch use-after-free bugs in debug builds

```c
// GOOD: Stack allocation for fixed-size data
char buffer[256];

// GOOD: Checked heap allocation with cleanup
char* data = malloc(size);
if (!data) {
    log_error("Failed to allocate %zu bytes", size);
    return ERR_NO_MEMORY;
}
// ... use data ...
free(data);
data = NULL;  // Prevent double-free

// BAD: Unchecked allocation
char* data = malloc(size);
strcpy(data, input);  // Crash if malloc failed
```

### Memory Leak Prevention
- Every function that allocates must document ownership transfer
- Use goto for single exit point in complex error handling
- Implement cleanup functions for complex structures
- Use valgrind regularly during development

```c
// GOOD: Single exit point with cleanup
int process_data(const char* input) {
    int ret = 0;
    char* buffer = NULL;
    FILE* file = NULL;
    
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        ret = ERR_NO_MEMORY;
        goto cleanup;
    }
    
    file = fopen(input, "r");
    if (!file) {
        ret = ERR_FILE_OPEN;
        goto cleanup;
    }
    
    // ... processing ...
    
cleanup:
    free(buffer);
    if (file) fclose(file);
    return ret;
}
```

## Resource Constraints

### Code Size Optimization
- Avoid inline functions unless proven beneficial
- Share common code paths
- Use function pointers for conditional logic in tables
- Strip debug symbols in release builds

### CPU Optimization
- Minimize system calls
- Cache frequently accessed data
- Use efficient algorithms (prefer O(n) over O(n²))
- Avoid floating point on devices without FPU
- Profile before optimizing (don't guess)

### Memory Optimization
- Use bitfields for boolean flags
- Pack structures to minimize padding
- Use const for read-only data (goes in .rodata)
- Prefer static buffers with maximum sizes when bounds are known
- Implement object pools for frequently created/destroyed objects

```c
// GOOD: Packed structure
typedef struct __attribute__((packed)) {
    uint8_t flags;
    uint16_t id;
    uint32_t timestamp;
    char name[32];
} telemetry_event_t;

// GOOD: Const data in .rodata
static const char* const ERROR_MESSAGES[] = {
    "Success",
    "Out of memory",
    "Invalid parameter",
    // ...
};
```

## Platform Independence

### Never Assume
- Pointer size (use uintptr_t for pointer arithmetic)
- Byte order (use htonl/ntohl for network data)
- Structure packing (use __attribute__((packed)) or #pragma pack)
- Integer sizes (use int32_t, uint64_t from stdint.h)
- Boolean type (use stdbool.h)

```c
// GOOD: Platform-independent types
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;           // Always 32 bits
    uint64_t timestamp;    // Always 64 bits
    bool enabled;          // Standard boolean
} config_t;

// GOOD: Endianness handling
uint32_t network_value = htonl(host_value);

// BAD: Assumptions
int id;  // Size varies by platform
long timestamp;  // 32 or 64 bits depending on platform
```

### Abstraction Layers
- Use platform abstraction for OS-specific code
- Isolate hardware dependencies
- Use configure.ac to detect platform capabilities

## Error Handling

### Return Value Convention
- Return 0 for success, negative for errors
- Use errno for system call failures
- Define error codes in header files
- Never ignore return values

```c
// GOOD: Consistent error handling
typedef enum {
    T2ERROR_SUCCESS = 0,
    T2ERROR_FAILURE = -1,
    T2ERROR_INVALID_PARAM = -2,
    T2ERROR_NO_MEMORY = -3,
    T2ERROR_TIMEOUT = -4
} T2ERROR;

T2ERROR init_telemetry() {
    if (!validate_config()) {
        return T2ERROR_INVALID_PARAM;
    }
    
    if (allocate_resources() != 0) {
        return T2ERROR_NO_MEMORY;
    }
    
    return T2ERROR_SUCCESS;
}
```

### Logging
- Use severity levels appropriately
- Log errors with context (function, line, errno)
- Avoid logging in hot paths
- Make logging configurable at runtime
- Never log sensitive data

```c
// GOOD: Contextual error logging
if (ret != 0) {
    T2Error("%s:%d Failed to initialize: %s (errno=%d)", 
            __FUNCTION__, __LINE__, strerror(errno), errno);
    return T2ERROR_FAILURE;
}
```

## Thread Safety and Concurrency

### Critical Principles

- **Minimize synchronization overhead**: Use lightweight primitives
- **Prevent deadlocks**: Establish lock ordering, use timeouts
- **Avoid memory fragmentation**: Configure thread stack sizes appropriately
- **Reduce contention**: Design for lock-free patterns where possible
- **Document thread safety**: Mark functions as thread-safe or not

### Thread Creation with Minimal Memory

Always create threads with attributes that specify required memory:

```c
// GOOD: Thread with minimal stack size
#include <pthread.h>

#define THREAD_STACK_SIZE (64 * 1024)  // 64KB instead of default (often 8MB)

pthread_t thread;
pthread_attr_t attr;

// Initialize attributes
pthread_attr_init(&attr);

// Set minimal stack size (reduces memory fragmentation)
pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

// Detached threads free resources immediately when done
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

// Create thread
int ret = pthread_create(&thread, &attr, thread_function, arg);
if (ret != 0) {
    T2Error("Failed to create thread: %s", strerror(ret));
    pthread_attr_destroy(&attr);
    return T2ERROR_FAILURE;
}

// Clean up attributes
pthread_attr_destroy(&attr);

// BAD: Default thread (wastes memory)
pthread_create(&thread, NULL, thread_function, arg);  // Uses 8MB stack!
```

### Lightweight Synchronization

Prefer lightweight synchronization primitives to avoid deadlocks and overhead:

```c
// GOOD: Simple mutex with minimal overhead
typedef struct {
    pthread_mutex_t lock;
    int counter;
} thread_safe_counter_t;

int init_counter(thread_safe_counter_t* c) {
    // Use default attributes (lightest weight)
    pthread_mutex_init(&c->lock, NULL);
    c->counter = 0;
    return 0;
}

void increment_counter(thread_safe_counter_t* c) {
    pthread_mutex_lock(&c->lock);
    c->counter++;
    pthread_mutex_unlock(&c->lock);
}

void cleanup_counter(thread_safe_counter_t* c) {
    pthread_mutex_destroy(&c->lock);
}

// GOOD: Use atomic operations when possible (no locks needed)
#include <stdatomic.h>

typedef struct {
    atomic_int counter;  // Lock-free!
} lockfree_counter_t;

void increment_lockfree(lockfree_counter_t* c) {
    atomic_fetch_add(&c->counter, 1);  // No mutex overhead
}
```

### Deadlock Prevention

Follow strict rules to prevent deadlocks:

```c
// GOOD: Consistent lock ordering
typedef struct {
    pthread_mutex_t lock_a;
    pthread_mutex_t lock_b;
    // ... data ...
} resource_t;

// RULE: Always acquire locks in alphabetical order (a, then b)
void multi_lock_operation(resource_t* r) {
    pthread_mutex_lock(&r->lock_a);   // First: lock_a
    pthread_mutex_lock(&r->lock_b);   // Second: lock_b
    
    // ... critical section ...
    
    pthread_mutex_unlock(&r->lock_b);  // Release in reverse order
    pthread_mutex_unlock(&r->lock_a);
}

// GOOD: Use trylock with timeout to avoid indefinite blocking
#include <time.h>

int safe_lock_with_timeout(pthread_mutex_t* lock, int timeout_ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    
    int ret = pthread_mutex_timedlock(lock, &ts);
    if (ret == ETIMEDOUT) {
        T2Error("Lock timeout - potential deadlock detected");
        return -1;
    }
    return ret;
}

// BAD: Different lock order in different functions (DEADLOCK RISK!)
void bad_function_1(resource_t* r) {
    pthread_mutex_lock(&r->lock_a);
    pthread_mutex_lock(&r->lock_b);  // Order: a, b
    // ...
}

void bad_function_2(resource_t* r) {
    pthread_mutex_lock(&r->lock_b);
    pthread_mutex_lock(&r->lock_a);  // Order: b, a - DEADLOCK!
    // ...
}
```

### Avoid Heavy Synchronization

Heavy synchronization causes performance issues and fragmentation:

```c
// BAD: Reader-writer lock for simple counter (overkill)
pthread_rwlock_t heavy_lock;
int counter;

void heavy_increment() {
    pthread_rwlock_wrlock(&heavy_lock);  // Too heavy!
    counter++;
    pthread_rwlock_unlock(&heavy_lock);
}

// GOOD: Use appropriate synchronization level
atomic_int light_counter;  // Lock-free for simple operations

void light_increment() {
    atomic_fetch_add(&light_counter, 1);  // No lock overhead
}

// BAD: Fine-grained locking everywhere (lock thrashing)
typedef struct {
    pthread_mutex_t lock;
    int value;
} each_field_locked_t;  // Don't do this!

// GOOD: Coarse-grained locking for related data
typedef struct {
    pthread_mutex_t lock;
    int value_a;
    int value_b;
    int value_c;  // All protected by one lock
} properly_locked_t;
```

### Lock-Free Patterns

Use lock-free patterns to avoid synchronization overhead:

```c
// GOOD: Lock-free flag
#include <stdatomic.h>

typedef struct {
    atomic_bool shutdown_requested;
} thread_control_t;

void request_shutdown(thread_control_t* ctrl) {
    atomic_store(&ctrl->shutdown_requested, true);
}

bool should_shutdown(thread_control_t* ctrl) {
    return atomic_load(&ctrl->shutdown_requested);
}

// GOOD: Lock-free queue for single producer, single consumer
typedef struct {
    atomic_int read_index;
    atomic_int write_index;
    void* buffer[256];
} spsc_queue_t;

bool spsc_enqueue(spsc_queue_t* q, void* item) {
    int write = atomic_load(&q->write_index);
    int next_write = (write + 1) % 256;
    
    if (next_write == atomic_load(&q->read_index)) {
        return false;  // Queue full
    }
    
    q->buffer[write] = item;
    atomic_store(&q->write_index, next_write);
    return true;
}
```

### Minimize Critical Sections

Keep locked sections as short as possible:

```c
// BAD: Long critical section
void bad_process(data_t* shared) {
    pthread_mutex_lock(&shared->lock);
    
    // Heavy computation while holding lock (BAD!)
    for (int i = 0; i < 1000000; i++) {
        compute_something();
    }
    
    shared->value = result;
    pthread_mutex_unlock(&shared->lock);
}

// GOOD: Minimal critical section
void good_process(data_t* shared) {
    // Do heavy computation WITHOUT lock
    int result = 0;
    for (int i = 0; i < 1000000; i++) {
        result += compute_something();
    }
    
    // Lock only for the update
    pthread_mutex_lock(&shared->lock);
    shared->value = result;
    pthread_mutex_unlock(&shared->lock);
}
```

### Thread-Safe Initialization

Use pthread_once for thread-safe initialization:

```c
// GOOD: Thread-safe singleton initialization
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static config_t* global_config = NULL;

static void init_config_once(void) {
    global_config = malloc(sizeof(config_t));
    // ... initialize config ...
}

config_t* get_config(void) {
    pthread_once(&init_once, init_config_once);
    return global_config;
}

// BAD: Double-checked locking (broken in C without memory barriers)
static pthread_mutex_t init_lock;
static config_t* config = NULL;

config_t* bad_get_config(void) {
    if (config == NULL) {  // First check (no lock)
        pthread_mutex_lock(&init_lock);
        if (config == NULL) {  // Second check
            config = malloc(sizeof(config_t));  // Race condition!
        }
        pthread_mutex_unlock(&init_lock);
    }
    return config;
}
```

### Thread Safety Documentation

Always document thread safety expectations:

```c
// GOOD: Clear thread safety documentation

/**
 * Process telemetry event
 * @param event Event to process
 * @return 0 on success, negative on error
 * 
 * Thread Safety: This function is thread-safe and may be called
 *                from multiple threads concurrently.
 */
int process_event(const event_t* event) {
    // Uses internal locking
}

/**
 * Initialize event processor
 * @return 0 on success, negative on error
 * 
 * Thread Safety: NOT thread-safe. Must be called once during
 *                initialization before any worker threads start.
 */
int init_event_processor(void) {
    // No locking - initialization only
}

/**
 * Get current statistics
 * @param stats Output buffer for statistics
 * 
 * Thread Safety: Caller must hold stats_lock before calling.
 *                Use get_stats_safe() for automatic locking.
 */
void get_stats_unlocked(stats_t* stats) {
    // Assumes caller holds lock
}
```

### Memory Fragmentation Prevention

Configure thread pools to prevent fragmentation:

```c
// GOOD: Thread pool with pre-allocated threads
#define THREAD_POOL_SIZE 4
#define WORK_QUEUE_SIZE 256

typedef struct {
    pthread_t threads[THREAD_POOL_SIZE];
    pthread_attr_t thread_attr;
    // ... work queue ...
} thread_pool_t;

int init_thread_pool(thread_pool_t* pool) {
    // Configure thread attributes once
    pthread_attr_init(&pool->thread_attr);
    pthread_attr_setstacksize(&pool->thread_attr, THREAD_STACK_SIZE);
    pthread_attr_setdetachstate(&pool->thread_attr, PTHREAD_CREATE_JOINABLE);
    
    // Create fixed number of threads (no dynamic allocation)
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        int ret = pthread_create(&pool->threads[i], &pool->thread_attr,
                                 worker_thread, pool);
        if (ret != 0) {
            // Cleanup already created threads
            cleanup_partial_pool(pool, i);
            return -1;
        }
    }
    
    return 0;
}

// BAD: Creating threads dynamically (causes fragmentation)
void bad_handle_request(request_t* req) {
    pthread_t thread;
    pthread_create(&thread, NULL, handle_one_request, req);
    pthread_detach(thread);  // New thread for each request!
}
```

### Testing Thread Safety

```c
// GOOD: Test for race conditions
#include <gtest/gtest.h>

TEST(ThreadSafety, ConcurrentIncrement) {
    thread_safe_counter_t counter = {0};
    init_counter(&counter);
    
    const int NUM_THREADS = 10;
    const int INCREMENTS_PER_THREAD = 1000;
    pthread_t threads[NUM_THREADS];
    
    // Create multiple threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, 
                      increment_n_times, &counter);
    }
    
    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify no race conditions
    EXPECT_EQ(counter.counter, NUM_THREADS * INCREMENTS_PER_THREAD);
    
    cleanup_counter(&counter);
}
```

### Static Analysis for Concurrency

```bash
# Use thread sanitizer to detect race conditions
gcc -g -fsanitize=thread source.c -o program
./program

# Use helgrind (valgrind) to detect synchronization issues
valgrind --tool=helgrind ./program

# Check for deadlocks
valgrind --tool=helgrind --track-lockorders=yes ./program
```

## Code Style

### Naming Conventions
- Functions: `snake_case` (e.g., `init_telemetry`)
- Types: `snake_case_t` (e.g., `telemetry_event_t`)
- Macros/Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)
- Global variables: `g_` prefix (avoid when possible)
- Static variables: `s_` prefix

### File Organization
- One .c file per module
- Corresponding .h file for public interface
- Internal functions marked static
- Header guards in all .h files

```c
// GOOD: header guard
#ifndef TELEMETRY_INTERNAL_H
#define TELEMETRY_INTERNAL_H

// ... declarations ...

#endif /* TELEMETRY_INTERNAL_H */
```

## Testing Requirements

### Unit Tests
- Test all public functions
- Test error paths and edge cases
- Use mocks for external dependencies
- Verify resource cleanup (no leaks)
- Run tests under valgrind

### Memory Testing
```bash
# Run with memory checking
valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes ./test_binary

# Static analysis
cppcheck --enable=all --inconclusive source/
```

## Anti-Patterns to Avoid

```c
// BAD: Magic numbers
if (size > 1024) { ... }

// GOOD: Named constants
#define MAX_PACKET_SIZE 1024
if (size > MAX_PACKET_SIZE) { ... }

// BAD: Unchecked allocation
char* buf = malloc(size);
strcpy(buf, input);

// GOOD: Checked with cleanup
char* buf = malloc(size);
if (!buf) return ERR_NO_MEMORY;
strncpy(buf, input, size - 1);
buf[size - 1] = '\0';

// BAD: Memory leak in error path
FILE* f = fopen(path, "r");
if (condition) return -1;  // Leaked f
fclose(f);

// GOOD: Cleanup on all paths
FILE* f = fopen(path, "r");
if (!f) return -1;
if (condition) {
    fclose(f);
    return -1;
}
fclose(f);
return 0;
```

## References

- Project follows RDK coding standards
- See `uploadstblogs/include/` for uploadSTBLogs API header documentation
- Review existing code in `uploadstblogs/src/` for patterns
- Check `src/unittest/` directory for testing examples
