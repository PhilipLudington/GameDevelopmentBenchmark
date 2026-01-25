# Bug Report: Heap buffer overflow in config value parsing

## Summary

The configuration file parser allocates a fixed-size heap buffer for config values but fails to check the length of input values when copying, causing heap buffer overflow with long config values.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/platform/config.c`
- Severity: High (heap corruption, potential code execution)

## Bug Description

Julius reads configuration from INI-style files where each line has a key=value format. The parser allocates a 128-byte heap buffer for storing config values, but the parsing code copies values without checking their length.

```c
#define CONFIG_VALUE_SIZE 128

static char *parse_config_value(const char *line)
{
    char *value = malloc(CONFIG_VALUE_SIZE);
    if (!value) return NULL;

    const char *equals = strchr(line, '=');
    if (equals) {
        // BUG: No length check before copy
        strcpy(value, equals + 1);
    }

    return value;
}
```

When a config value exceeds 127 characters (plus null terminator), the `strcpy()` overflows the heap buffer.

## Steps to Reproduce

1. Create a config file with a line containing a value > 127 characters
2. Load the configuration
3. Heap buffer overflow occurs

Example malformed config:
```
window_title=AAAAAAAAAAAAAAAAAAA...(150+ characters)...AAAAAAAAAA
```

## Expected Behavior

The parser should:
- Check value length before copying
- Truncate values that exceed the buffer size
- Or dynamically allocate based on actual value length

## Current Behavior

Values exceeding 127 characters cause heap buffer overflow, corrupting adjacent heap memory. AddressSanitizer reports:

```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000090
WRITE of size 155 at 0x602000000090
    #0 strcpy
    #1 parse_config_value config.c:87
```

## Relevant Code

Look at `src/platform/config.c`:
- `parse_config_value()` function
- `CONFIG_VALUE_SIZE` constant
- Any other places where config strings are copied

## Suggested Fix Approach

Use length-bounded copy:

```c
static char *parse_config_value(const char *line)
{
    char *value = malloc(CONFIG_VALUE_SIZE);
    if (!value) return NULL;

    const char *equals = strchr(line, '=');
    if (equals) {
        strncpy(value, equals + 1, CONFIG_VALUE_SIZE - 1);
        value[CONFIG_VALUE_SIZE - 1] = '\0';
    } else {
        value[0] = '\0';
    }

    return value;
}
```

## Your Task

Fix the heap buffer overflow in the config parser. Your fix should:

1. Add proper bounds checking when copying config values
2. Ensure the buffer is always null-terminated
3. Handle values exactly at the size limit correctly
4. Not break parsing of normal-length config values

Provide your fix as a unified diff (patch).
