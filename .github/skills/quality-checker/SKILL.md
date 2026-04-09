---
name: quality-checker
description: Run comprehensive quality checks (static analysis, memory safety, thread safety, build verification) in the standard test container. Use when validating code changes or debugging before committing.
---

# Container-Based Quality Checker

## Purpose

Execute comprehensive quality checks on the remote debugger codebase using the same containerized environment as CI pipelines. Ensures consistency between local development and automated testing.

## Usage

Invoke this skill when:
- Validating changes before committing
- Debugging build or test failures
- Running quality checks locally
- Verifying memory safety of new code
- Checking for thread safety issues
- Performing static analysis

You can run all checks or select specific ones based on your needs.

## What It Does

This skill runs quality checks inside the official test container used by this repository (`ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest`), which includes:
- Build tools (gcc, g++, autotools, make)
- Static analysis tools (cppcheck, shellcheck)
- Memory analysis tools (valgrind)
- Thread analysis tools (helgrind)
- Google Test/Mock frameworks

## Available Checks

### 1. Static Analysis
- **cppcheck**: Comprehensive C/C++ static code analyzer
- **shellcheck**: Shell script linter
- **Output**: XML report with findings

### 2. Memory Safety (Valgrind)
- **Memory leak detection**: Finds unreleased allocations
- **Use-after-free detection**: Catches dangling pointer usage
- **Invalid memory access**: Buffer overflows, uninitialized reads
- **Output**: XML and log files per test binary

### 3. Thread Safety (Helgrind)
- **Race condition detection**: Finds unsynchronized shared memory access
- **Deadlock detection**: Identifies lock ordering issues
- **Lock usage verification**: Validates proper synchronization
- **Output**: XML and log files per test binary

### 4. Build Verification
- **Strict compilation**: Builds with `-Wall -Wextra -Werror`
- **Test build**: Verifies tests compile
- **Binary analysis**: Reports size and dependencies
- **Output**: Build artifacts and size report

## Execution Process

### Step 1: Setup Container Environment

Pull the latest test container:
```bash
docker pull ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
```

Start container with workspace mounted:
```bash
docker run -d --name native-platform \
  -v /path/to/workspace:/mnt/workspace \
  ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
```

### Step 2: Run Selected Checks

Execute the requested quality checks inside the container:

**Static Analysis:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  cppcheck --enable=all \
           --inconclusive \
           --suppress=missingIncludeSystem \
           --suppress=unmatchedSuppression \
           --error-exitcode=0 \
           --xml \
           --xml-version=2 \
           . 2> cppcheck-report.xml
"
```

**Shell Script Checks:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  find . -name '*.sh' -type f -exec shellcheck {} +
"
```

**Memory Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
   cd /mnt/workspace/src/unittest && \
   automake --add-missing && \
   autoreconf --install && \
   ./configure && \
   make -j\$(nproc) && \
   find . -type f -executable -name '*gtest*' 2>/dev/null | while read test_bin; do
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --xml=yes \
             --xml-file=\"valgrind-\$(basename \$test_bin).xml\" \
             \"\$test_bin\" 2>&1 | tee \"valgrind-\$(basename \$test_bin).log\"
  done
"
```

**Thread Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
   cd /mnt/workspace/src/unittest && \
   find . -type f -executable -name '*gtest*' 2>/dev/null | while read test_bin; do
    valgrind --tool=helgrind \
             --track-lockorders=yes \
             --xml=yes \
             --xml-file=\"helgrind-\$(basename \$test_bin).xml\" \
             \"\$test_bin\" 2>&1 | tee \"helgrind-\$(basename \$test_bin).log\"
  done
"
```

**Build Verification:**
```bash
docker exec -i native-platform /bin/bash -c "
   cd /mnt/workspace && \
   sh -e cov_build.sh && \
   if [ -f 'src/remotedebugger' ]; then
      ls -lh src/remotedebugger
      file src/remotedebugger
      size src/remotedebugger
   fi
   if [ -f 'src/unittest/remotedebugger_gtest' ]; then
      ls -lh src/unittest/remotedebugger_gtest
      file src/unittest/remotedebugger_gtest
      size src/unittest/remotedebugger_gtest
   fi
"
```

### Step 3: Report Results

Parse and summarize results for the user:
- Number of issues found by category
- Critical issues requiring immediate attention
- Warnings that should be addressed
- Memory leaks with stack traces
- Race conditions or deadlock risks
- Build errors or warnings

### Step 4: Cleanup

Stop and remove the container:
```bash
docker stop native-platform
docker rm native-platform
```

## Interpreting Results

### Static Analysis (cppcheck)
- **error**: Critical issues that must be fixed
- **warning**: Potential problems to review
- **style**: Code style improvements
- **performance**: Optimization opportunities

### Memory Safety (Valgrind)
- **definitely lost**: Memory leaks requiring fixes
- **indirectly lost**: Leaks from lost parent structures
- **possibly lost**: Potential leaks to investigate
- **still reachable**: Memory held at exit (usually OK)
- **Invalid read/write**: Buffer overflow (CRITICAL)
- **Use of uninitialized value**: Must initialize before use

### Thread Safety (Helgrind)
- **Possible data race**: Unsynchronized access to shared data
- **Lock order violation**: Potential deadlock scenario
- **Unlocking unlocked lock**: Synchronization bug
- **Thread still holds locks**: Resource leak

### Build Verification
- **Compilation errors**: Must fix before proceeding
- **Warnings**: Review and fix (builds with -Werror)
- **Binary size**: Monitor for embedded constraints

## User Interaction

When invoked, ask the user:

1. **Which checks to run?**
   - All checks (comprehensive)
   - Static analysis only (fast)
   - Memory safety only
   - Thread safety only
   - Build verification only
   - Custom combination

2. **Scope:**
   - Full codebase
   - Specific directories
   - Recently changed files

3. **Report detail:**
   - Summary only (counts and critical issues)
   - Detailed (all findings)
   - Full raw output

## Example Invocations

**User**: "Run quality checks"
- Default: Run all checks on full codebase, provide summary

**User**: "Check memory safety"
- Run only valgrind checks, detailed report

**User**: "Quick static analysis"
- Run cppcheck and shellcheck, summary only

**User**: "Verify my changes build"
- Run build verification with strict warnings

**User**: "Full analysis on src"
- Run all checks scoped to the remote debugger sources

## Best Practices

1. **Run before committing**: Catch issues early
2. **Start with static analysis**: Fastest feedback
3. **Run memory checks on test binaries**: Most effective
4. **Review thread safety for concurrent code**: Essential for multi-threaded components
5. **Monitor binary size**: Important for embedded targets

## Integration with Development Workflow

1. **Pre-commit**: Quick static analysis
2. **Pre-push**: Full quality check suite
3. **Debugging**: Targeted memory/thread analysis
4. **Code review**: Validate reviewer feedback
5. **Refactoring**: Ensure no regressions

## Advantages Over Manual Testing

- **Consistency**: Same environment as CI/CD
- **Completeness**: All tools in one command
- **Reproducibility**: Container ensures identical results
- **Efficiency**: No local tool installation needed
- **Confidence**: Pass locally = pass in CI

## Output Files Generated

- `cppcheck-report.xml`: Static analysis findings
- `valgrind-<testname>.xml`: Memory issues per test
- `valgrind-<testname>.log`: Detailed memory logs
- `helgrind-<testname>.xml`: Thread safety issues per test
- `helgrind-<testname>.log`: Detailed concurrency logs

These files can be uploaded as artifacts or reviewed locally.

## Limitations

- Requires Docker with GitHub Container Registry access
- Container pulls can be slow on first run (cached afterward)
- Full suite can take several minutes depending on codebase size
- Valgrind slows execution significantly (expected)

## Tips for Faster Execution

1. Use cached container images (don't pull every time)
2. Run static analysis first (fastest)
3. Scope checks to changed directories
4. Run memory/thread checks only on affected tests
5. Use parallel execution where possible

## Skill Execution Logic

When user invokes this skill:

1. **Check local image first**
   - Reuse the existing native-platform image when present
   - Pull only when needed and only with user approval outside CI

2. **Start container**
   - Mount workspace at /mnt/workspace
   - Use unique container name (quality-checker-<timestamp>)
   - Run in detached mode

3. **Execute requested checks**
   - Run checks in sequence
   - Capture output
   - Continue on errors (collect all findings)

4. **Collect results**
   - Copy result files from container
   - Parse XML/log outputs
   - Categorize findings

5. **Report to user**
   - Summary count
   - Critical issues highlighted
   - Link to detailed reports
   - Next steps recommendations

6. **Cleanup**
   - Stop container
   - Remove container
   - Optional: clean up result files

## Error Handling

- **Container pull fails**: Report error, suggest manual pull
- **Container start fails**: Check Docker daemon, ports, permissions
- **Build fails**: Report build errors, stop further checks
- **Tools missing**: Verify container version, report missing tools
- **Out of memory**: Suggest increasing Docker memory limit
