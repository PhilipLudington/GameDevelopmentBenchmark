# Bug Report: Stack buffer overflow in translation string copy

## Summary

The translation system copies translated strings into fixed-size stack buffers without checking the length of the source string, causing a stack buffer overflow when the translated string exceeds the buffer size.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/translation/translation.c`
- Severity: High (memory corruption, potential code execution)

## Bug Description

Julius supports multiple languages through a translation system. When displaying translated text, the code copies translated strings into local stack buffers for formatting. However, the current implementation uses `strcpy()` without checking if the source string fits in the destination buffer.

The `translation_for_key()` function retrieves translated text for UI elements. When preparing display strings, the code copies the translation into a fixed 64-byte buffer:

```c
static void format_translated_string(int key, char *output, int max_len)
{
    char buffer[64];
    const char *translation = translation_for_key(key);

    // BUG: No length check before copy
    strcpy(buffer, translation);

    // ... formatting logic ...
    snprintf(output, max_len, "[%s]", buffer);
}
```

Some translations, particularly in languages like German or Russian, can exceed 64 characters, causing a stack buffer overflow.

## Steps to Reproduce

1. Load a translation file with a string longer than 64 characters
2. Display UI element using that translation key
3. Stack buffer overflow occurs

## Expected Behavior

The code should safely handle translations of any length by:
- Using length-bounded copy functions (strncpy or snprintf)
- Truncating if necessary while preserving null termination

## Current Behavior

When a translation exceeds 64 characters, the `strcpy()` overwrites adjacent stack memory, causing:
- Potential crash due to corrupted return address
- Memory corruption affecting local variables
- Security vulnerability (potential code execution)

AddressSanitizer output:
```
==12345==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffc12345678
WRITE of size 85 at 0x7ffc12345678
    #0 strcpy
    #1 format_translated_string translation.c:45
```

## Relevant Code

Look at `src/translation/translation.c`:
- `format_translated_string()` function
- Any other locations where translations are copied to fixed-size buffers

## Suggested Fix Approach

Replace unbounded `strcpy()` calls with bounded alternatives:

```c
// Instead of:
strcpy(buffer, translation);

// Use:
strncpy(buffer, translation, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';

// Or better:
snprintf(buffer, sizeof(buffer), "%s", translation);
```

## Your Task

Fix the stack buffer overflow by ensuring translation strings are safely copied with proper length bounds. Your fix should:

1. Replace `strcpy()` with a length-bounded copy operation
2. Ensure the buffer is always null-terminated
3. Not change the function's behavior for strings that fit in the buffer
4. Handle edge cases (empty strings, exactly buffer-sized strings)

Provide your fix as a unified diff (patch).
