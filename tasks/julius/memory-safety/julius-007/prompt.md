# Bug Report: Buffer Overflow in Filename Path Construction

## Summary

The file path construction code has a heap buffer overflow vulnerability. When constructing full paths from a base directory and filename, the code doesn't check if the combined length exceeds the allocated buffer size.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/core/file.c`
- Severity: High (memory corruption, potential code execution)

## Bug Description

The `file_construct_path()` function builds a full file path by combining:
1. A base directory path
2. A path separator
3. A filename

It allocates a fixed-size buffer (MAX_PATH_LENGTH = 260 bytes) but uses `strcpy()` and `strcat()` without checking bounds:

```c
#define MAX_PATH_LENGTH 260

char *file_construct_path(const char *dir, const char *filename)
{
    char *buffer = malloc(MAX_PATH_LENGTH);
    strcpy(buffer, dir);           // No bounds check!
    strcat(buffer, "/");           // No bounds check!
    strcat(buffer, filename);      // No bounds check!
    return buffer;
}
```

If `dir` + "/" + `filename` exceeds 260 bytes, the code writes past the buffer, causing heap corruption.

## Steps to Reproduce

1. Provide a base directory path of 200 characters
2. Provide a filename of 100 characters
3. The combined path (200 + 1 + 100 = 301) exceeds MAX_PATH_LENGTH
4. Memory corruption occurs

## Expected Behavior

The function should either:
1. Truncate the path to fit the buffer, OR
2. Return an error if the path is too long

## Current Behavior

Writes past buffer bounds, triggering AddressSanitizer:
```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x...
WRITE of size 1 at 0x... thread T0
```

## Relevant Code

Look at `src/core/file.c`, specifically:
- `file_construct_path()` function
- Any other path construction code
- String operations without length checking

## Suggested Fix Approach

Use safe string functions with explicit length limits:

```c
char *file_construct_path(const char *dir, const char *filename)
{
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(filename);
    size_t total = dir_len + 1 + file_len + 1;  // +1 for '/', +1 for null

    if (total > MAX_PATH_LENGTH) {
        return NULL;  // Path too long
    }

    char *buffer = malloc(MAX_PATH_LENGTH);
    if (!buffer) return NULL;

    strncpy(buffer, dir, MAX_PATH_LENGTH - 1);
    buffer[MAX_PATH_LENGTH - 1] = '\0';

    // Safe concatenation
    strncat(buffer, "/", MAX_PATH_LENGTH - strlen(buffer) - 1);
    strncat(buffer, filename, MAX_PATH_LENGTH - strlen(buffer) - 1);

    return buffer;
}
```

Or use snprintf for safer formatting:
```c
snprintf(buffer, MAX_PATH_LENGTH, "%s/%s", dir, filename);
```

## Your Task

Fix the buffer overflow by implementing proper bounds checking. Your fix should:
1. Check combined path length before writing
2. Either truncate safely or return NULL for paths that are too long
3. Ensure no write occurs past the allocated buffer
4. Handle edge cases (empty strings, NULL inputs)

Provide your fix as a unified diff (patch).
