# Bug Report: Crash on malformed save file with bad header

## Summary

The game crashes when attempting to load a save file with a corrupted or malformed header. The loader trusts header values without validation, leading to buffer overflows and crashes.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/game/save.c`
- Severity: Critical (crash, potential security issue)

## Bug Description

The save file loader reads the header and uses its values directly without validation:

```c
#define SAVE_MAGIC 0x434A5553  /* "JUSC" */
#define MAX_VERSION 10
#define MAX_DATA_SIZE (10 * 1024 * 1024)  /* 10 MB */

typedef struct {
    int magic;
    int version;
    int data_size;
    int checksum;
} save_header;

int load_save_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    save_header header;
    fread(&header, sizeof(header), 1, f);

    // BUG: No validation of header values
    char *data = malloc(header.data_size);  // Could be negative or huge
    fread(data, header.data_size, 1, f);

    process_save_data(data, header.data_size);

    free(data);
    fclose(f);
    return 1;
}
```

## Steps to Reproduce

1. Create a save file with corrupted header:
   - Invalid magic number
   - Negative data_size
   - Version number > MAX_VERSION
2. Attempt to load the corrupted save file
3. Game crashes (malloc failure, buffer overflow, or invalid read)

## Expected Behavior

When loading a malformed save file:
1. Validate the magic number
2. Check version is supported
3. Validate data_size is reasonable
4. Return error if validation fails
5. Display user-friendly error message
6. Not crash

## Current Behavior

No header validation:
- Invalid magic: processes garbage data
- Negative data_size: malloc fails or allocates huge amount
- Huge data_size: out of memory crash
- Invalid version: undefined behavior

## Relevant Code

Look at `src/game/save.c`:
- `load_save_file()` function
- Header reading and validation
- Data size handling

## Suggested Fix Approach

Add comprehensive header validation:

```c
int load_save_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    save_header header;
    if (fread(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    // FIXED: Validate header
    if (header.magic != SAVE_MAGIC) {
        log_error("Invalid save file magic number", 0, header.magic);
        fclose(f);
        return 0;
    }

    if (header.version <= 0 || header.version > MAX_VERSION) {
        log_error("Unsupported save file version", 0, header.version);
        fclose(f);
        return 0;
    }

    if (header.data_size <= 0 || header.data_size > MAX_DATA_SIZE) {
        log_error("Invalid save file data size", 0, header.data_size);
        fclose(f);
        return 0;
    }

    char *data = malloc(header.data_size);
    if (!data) {
        fclose(f);
        return 0;
    }

    fread(data, header.data_size, 1, f);
    process_save_data(data, header.data_size);

    free(data);
    fclose(f);
    return 1;
}
```

## Your Task

Fix the save file loading crash by adding proper header validation. Your fix should:

1. Validate the magic number
2. Check version is within supported range
3. Validate data_size is positive and not too large
4. Return failure with appropriate error logging
5. Not crash on any malformed input

Provide your fix as a unified diff (patch).
