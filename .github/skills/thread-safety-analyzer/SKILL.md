---
name: thread-safety-analyzer
description: Analyze C/C++ code for thread safety issues including race conditions, deadlocks, and improper synchronization. Use when reviewing concurrent code or debugging threading issues.
---

# Thread Safety Analysis for Embedded C

## Purpose

Systematically analyze C/C++ code for thread safety issues that can cause race conditions, deadlocks, or performance degradation in embedded systems.

## Usage

Invoke this skill when:
- Reviewing multi-threaded code
- Debugging race conditions or deadlocks
- Optimizing synchronization overhead
- Validating thread creation and cleanup
- Investigating lock contention issues

## Analysis Process

### Step 1: Identify Shared Data

Search for global and static variables:
- Global variables (especially non-const)
- Static variables in functions
- Shared heap allocations
- Reference-counted objects

For each shared variable, verify:
1. How is it protected (mutex, atomic, etc.)?
2. Is the protection consistent across all accesses?
3. Are reads and writes both protected?
4. Is initialization thread-safe?

### Step 2: Review Thread Creation

Check all pthread_create calls:
- Are thread attributes used?
- Is stack size specified?
- Are threads detached or joinable?
- Is cleanup properly handled?

```c
// CHECK FOR:
pthread_t thread;
pthread_create(&thread, NULL, func, arg);  // BAD: No attributes

// SHOULD BE:
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 64 * 1024);  // Explicit size
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thread, &attr, func, arg);
pthread_attr_destroy(&attr);
```

### Step 3: Analyze Lock Usage

For each mutex/rwlock:
- Is it initialized before use?
- Is it destroyed when done?
- Are lock/unlock pairs balanced?
- What is the lock ordering?
- Are locks held during expensive operations?

Common patterns to check:
```c
// Pattern 1: Missing unlock on error path
pthread_mutex_lock(&lock);
if (error) return -1;  // LEAK!
pthread_mutex_unlock(&lock);

// Pattern 2: Lock ordering violation
// Thread 1:
pthread_mutex_lock(&a);
pthread_mutex_lock(&b);

// Thread 2:
pthread_mutex_lock(&b);  // Different order!
pthread_mutex_lock(&a);  // DEADLOCK RISK!

// Pattern 3: Heavy lock for simple operation
pthread_rwlock_wrlock(&lock);  // Too heavy
counter++;
pthread_rwlock_unlock(&lock);
// Should use atomic_int instead
```

### Step 4: Check for Race Conditions

Look for unprotected accesses to shared data:

```c
// RACE: Read-modify-write without protection
if (shared_flag == 0) {  // Thread 1 reads
    shared_flag = 1;     // Thread 2 also reads before Thread 1 writes
}

// FIX: Use atomic or lock
pthread_mutex_lock(&lock);
if (shared_flag == 0) {
    shared_flag = 1;
}
pthread_mutex_unlock(&lock);

// OR: Use atomic compare-and-swap
int expected = 0;
atomic_compare_exchange_strong(&shared_flag, &expected, 1);
```

### Step 5: Verify Atomic Usage

For atomic variables:
- Are they declared with proper type (atomic_int, atomic_bool)?
- Is memory ordering appropriate?
- Are non-atomic operations mixed with atomic ones?

```c
// CHECK:
atomic_int counter;

// GOOD: Atomic operations
atomic_fetch_add(&counter, 1);
int value = atomic_load(&counter);

// BAD: Mixing atomic and non-atomic
counter++;  // Non-atomic! Use atomic_fetch_add
```

### Step 6: Deadlock Detection

Check for common deadlock patterns:

1. **Circular wait**: Lock A → Lock B, Lock B → Lock A
2. **Lock held while waiting**: Mutex held during sleep/wait
3. **Missing timeout**: Indefinite blocking without timeout
4. **Signal under lock**: Condition signal while holding mutex

```c
// Deadlock Pattern 1: Circular dependency
// Function 1:
lock(mutex_a);
lock(mutex_b);  // Order: A, B

// Function 2:
lock(mutex_b);
lock(mutex_a);  // Order: B, A - DEADLOCK!

// Deadlock Pattern 2: Lock held during expensive operation
lock(mutex);
expensive_network_call();  // Blocks other threads!
unlock(mutex);

// Deadlock Pattern 3: No timeout
pthread_mutex_lock(&lock);  // Waits forever if deadlock
```

### Step 7: Check Condition Variables

For condition variables:
- Is wait always in a loop?
- Is predicate checked before and after wait?
- Is signal/broadcast done correctly?
- Is spurious wakeup handled?

```c
// GOOD: Proper condition variable usage
pthread_mutex_lock(&mutex);
while (!condition) {  // Loop for spurious wakeups
    pthread_cond_wait(&cond, &mutex);
}
// ... use protected data ...
pthread_mutex_unlock(&mutex);

// Signal:
pthread_mutex_lock(&mutex);
condition = true;
pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);

// BAD: Missing loop
pthread_mutex_lock(&mutex);
if (!condition) {  // Should be 'while'!
    pthread_cond_wait(&cond, &mutex);
}
pthread_mutex_unlock(&mutex);
```

## Common Issues and Fixes

### Issue: Default Thread Stack Size

```c
// PROBLEM: Wastes memory (8MB per thread)
pthread_t thread;
pthread_create(&thread, NULL, worker, arg);

// FIX: Specify minimal stack size
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 64 * 1024);  // 64KB
pthread_create(&thread, &attr, worker, arg);
pthread_attr_destroy(&attr);
```

### Issue: Heavy Synchronization

```c
// PROBLEM: Reader-writer lock overkill
pthread_rwlock_t lock;
int counter;

void increment() {
    pthread_rwlock_wrlock(&lock);
    counter++;
    pthread_rwlock_unlock(&lock);
}

// FIX: Use atomic operations
atomic_int counter;

void increment() {
    atomic_fetch_add(&counter, 1);  // No lock needed
}
```

### Issue: Lock Ordering Violation

```c
// PROBLEM: Different lock orders cause deadlock
// Thread 1:
void process_a_then_b() {
    lock(&resource_a.lock);
    lock(&resource_b.lock);
    // ...
}

// Thread 2:
void process_b_then_a() {
    lock(&resource_b.lock);
    lock(&resource_a.lock);  // DEADLOCK!
    // ...
}

// FIX: Consistent ordering everywhere
void process_a_then_b() {
    lock(&resource_a.lock);  // Always A first
    lock(&resource_b.lock);  // Then B
    // ...
}

void process_b_then_a() {
    lock(&resource_a.lock);  // Always A first
    lock(&resource_b.lock);  // Then B
    // ...
}
```

### Issue: Race in Lazy Initialization

```c
// PROBLEM: Non-thread-safe initialization
static config_t* config = NULL;

config_t* get_config() {
    if (!config) {  // Race here!
        config = malloc(sizeof(config_t));
        init_config(config);
    }
    return config;
}

// FIX: Use pthread_once
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static config_t* config = NULL;

static void init_config_once() {
    config = malloc(sizeof(config_t));
    init_config(config);
}

config_t* get_config() {
    pthread_once(&init_once, init_config_once);
    return config;
}
```

### Issue: Missing Lock on Error Path

```c
// PROBLEM: Lock not released on error
int process_data(data_t* shared) {
    pthread_mutex_lock(&shared->lock);
    
    if (validate(shared) != 0) {
        return -1;  // BUG: Lock not released!
    }
    
    update(shared);
    pthread_mutex_unlock(&shared->lock);
    return 0;
}

// FIX: Unlock on all paths
int process_data(data_t* shared) {
    int ret = 0;
    
    pthread_mutex_lock(&shared->lock);
    
    if (validate(shared) != 0) {
        ret = -1;
        goto cleanup;
    }
    
    update(shared);
    
cleanup:
    pthread_mutex_unlock(&shared->lock);
    return ret;
}
```

### Issue: Long Critical Section

```c
// PROBLEM: Expensive operation under lock
pthread_mutex_lock(&lock);
for (int i = 0; i < 1000000; i++) {
    compute();  // Blocks other threads!
}
shared_result = final_value;
pthread_mutex_unlock(&lock);

// FIX: Minimize critical section
int result = 0;
for (int i = 0; i < 1000000; i++) {
    result += compute();  // No lock
}

pthread_mutex_lock(&lock);
shared_result = result;  // Lock only for update
pthread_mutex_unlock(&lock);
```

## Testing for Thread Safety

### Compile with Thread Sanitizer

```bash
# Build with thread sanitizer
gcc -g -fsanitize=thread -O1 source.c -o program -lpthread

# Run
./program

# Will report:
# - Data races
# - Lock ordering issues
# - Potential deadlocks
```

### Run Helgrind

```bash
# Check for thread safety issues
valgrind --tool=helgrind \
         --track-lockorders=yes \
         ./program

# Reports:
# - Race conditions
# - Lock order violations
# - Possible deadlocks
```

### Stress Testing

```c
// Test under high concurrency
#define NUM_THREADS 100
#define ITERATIONS 10000

void stress_test() {
    pthread_t threads[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify invariants
    assert(shared_counter == NUM_THREADS * ITERATIONS);
}
```

## Output Format

Provide findings as:

```
## Thread Safety Analysis

### Critical Issues (must fix)
1. [file.c:123] Race condition - unprotected access to shared_flag
2. [file.c:456] Deadlock potential - lock order violation (A→B vs B→A)
3. [file.c:789] Lock leak - mutex not released on error path

### Warnings (should fix)
1. [file.c:234] Default thread stack - wastes 8MB per thread
2. [file.c:567] Heavy lock - use atomic_int instead of mutex
3. [file.c:890] Long critical section - holds lock during I/O

### Recommendations
1. Establish lock ordering convention (document in header)
2. Use pthread_once for singleton initialization
3. Replace reader-writer locks with atomics for counters
4. Add thread sanitizer to CI pipeline

### Suggested Fixes
[Provide specific code changes for each issue]
```

## Verification

After fixes:
1. Thread sanitizer shows no errors
2. Helgrind reports clean
3. Stress tests pass consistently
4. Lock contention metrics acceptable
5. No deadlocks under load testing
6. Code review confirms thread safety
