---
applyTo: "**/*.sh"
---

# Shell Script Standards for Embedded Systems

## Platform Independence

### Use POSIX Shell
- Use `#!/bin/sh` not `#!/bin/bash`
- Avoid bashisms (use shellcheck to verify)
- Test on busybox ash (common in embedded)

```bash
#!/bin/sh
# GOOD: POSIX compliant

# BAD: Bash-specific
if [[ $var == "value" ]]; then  # Use [ ] instead
    array=(1 2 3)  # Arrays not in POSIX
fi

# GOOD: POSIX compliant
if [ "$var" = "value" ]; then
    set -- 1 2 3  # Use positional parameters
fi
```

## Resource Awareness

### Minimize Process Spawning
- Use shell builtins when possible
- Avoid pipes when not necessary
- Batch operations to reduce forks

```bash
# BAD: Multiple processes
cat file | grep pattern | wc -l

# GOOD: Fewer processes
grep -c pattern file

# BAD: Loop with external commands
for file in *.txt; do
    cat "$file" >> output
done

# GOOD: Single cat invocation
cat *.txt > output
```

### Memory Usage
- Avoid reading entire files into variables
- Process streams line by line
- Clean up temporary files

```bash
# BAD: Loads entire file into memory
content=$(cat large_file.log)
echo "$content" | grep ERROR

# GOOD: Stream processing
grep ERROR large_file.log

# GOOD: Line-by-line processing
while IFS= read -r line; do
    process_line "$line"
done < large_file.log
```

## Error Handling

### Always Check Exit Codes
```bash
# GOOD: Check critical operations
if ! mkdir -p /tmp/telemetry; then
    logger -t telemetry "ERROR: Failed to create directory"
    exit 1
fi

# GOOD: Use set -e for fail-fast
set -e  # Exit on any error
set -u  # Exit on undefined variable
set -o pipefail  # Catch errors in pipes

# GOOD: Trap for cleanup
cleanup() {
    rm -f "$TEMP_FILE"
}
trap cleanup EXIT INT TERM

TEMP_FILE=$(mktemp)
# ... use temp file ...
# cleanup happens automatically
```

## Script Quality

### Defensive Programming
```bash
# GOOD: Quote all variables
rm -f "$file_path"  # Not: rm -f $file_path

# GOOD: Use -- to separate options from arguments
grep -r -- "$pattern" "$directory"

# GOOD: Check variable is set
: "${CONFIG_FILE:?CONFIG_FILE must be set}"

# GOOD: Validate inputs
if [ -z "$1" ]; then
    echo "Usage: $0 <component_name>" >&2
    exit 1
fi
```

### Logging
```bash
# Use logger for syslog integration
log_info() {
    logger -t telemetry -p user.info "$*"
}

log_error() {
    logger -t telemetry -p user.error "$*"
    echo "ERROR: $*" >&2
}

# Usage
log_info "Starting telemetry collection"
if ! start_service; then
    log_error "Failed to start service"
    exit 1
fi
```

## Testing Scripts

### Use shellcheck
```bash
# Run shellcheck on all scripts
shellcheck script.sh

# In CI
find . -name "*.sh" -exec shellcheck {} +
```

### Test on Target Platform
- Test on actual embedded device or emulator
- Verify with busybox tools
- Check resource usage (memory, CPU)

## Anti-Patterns

```bash
# BAD: Unquoted variables
for file in $FILES; do  # Word splitting!
    
# GOOD: Quoted
for file in "$FILES"; do

# BAD: Parsing ls output
for file in $(ls *.txt); do

# GOOD: Use glob
for file in *.txt; do

# BAD: Useless use of cat
cat file | grep pattern

# GOOD: grep can read files
grep pattern file

# BAD: Not checking if file exists
rm /tmp/file  # Error if doesn't exist

# GOOD: Check or use -f
rm -f /tmp/file  # Or: [ -f /tmp/file ] && rm /tmp/file
```
