/**
 * Test for double-free bug in smacker video decoder (julius-001)
 *
 * This test compiles against the actual Julius smacker.c and triggers
 * the double-free bug by creating a truncated SMK file.
 *
 * When read_frame_info() fails (truncated file), the buggy code frees
 * frame pointers but doesn't nullify them. Then smacker_close() frees
 * them again, causing a double-free that ASan will detect.
 *
 * Test validation:
 * - On FIXED code: smacker_open() returns NULL, no memory errors
 * - On BUGGY code: ASan detects double-free, test fails
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Forward declaration of smacker type and functions from smacker.c */
typedef struct smacker_t *smacker;
smacker smacker_open(FILE *fp);
void smacker_close(smacker s);

/*
 * Create a truncated SMK file that passes header validation
 * but fails during read_frame_info() because frame data is missing.
 *
 * SMK Header (104 bytes):
 *   0-3:   Signature "SMK2"
 *   4-7:   Width
 *   8-11:  Height
 *   12-15: Frame count (must be > 0 to trigger allocations)
 *   16-19: Frame rate
 *   20-23: Flags
 *   24-51: Audio sizes (7 x 4 bytes)
 *   52-55: Trees size
 *   56-71: Skip sizes
 *   72-99: Audio rates (7 x 4 bytes)
 *   100-103: Padding
 */
static int create_truncated_smk_file(const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to create test file: %s\n", path);
        return 0;
    }

    uint8_t header[104] = {0};

    /* Signature: "SMK2" */
    header[0] = 'S';
    header[1] = 'M';
    header[2] = 'K';
    header[3] = '2';

    /* Width: 320 (little endian) */
    header[4] = 0x40;
    header[5] = 0x01;
    header[6] = 0x00;
    header[7] = 0x00;

    /* Height: 200 (little endian) */
    header[8] = 0xC8;
    header[9] = 0x00;
    header[10] = 0x00;
    header[11] = 0x00;

    /* Frame count: 10 (must be > 0 to trigger allocations in read_frame_info) */
    header[12] = 0x0A;
    header[13] = 0x00;
    header[14] = 0x00;
    header[15] = 0x00;

    /* Frame rate: 100000 microseconds (10 FPS) */
    header[16] = 0xA0;
    header[17] = 0x86;
    header[18] = 0x01;
    header[19] = 0x00;

    /* Write header only - no frame info follows */
    /* This causes read_frame_info() to fail on fread() */
    size_t written = fwrite(header, 1, sizeof(header), fp);
    fclose(fp);

    return written == sizeof(header);
}

/*
 * Test: Open a truncated SMK file
 *
 * Expected behavior:
 * - FIXED code: smacker_open() returns NULL, no memory errors
 * - BUGGY code: Double-free in error path, ASan detects it
 */
static int test_truncated_file_handling(void)
{
    const char *test_file = "/tmp/test_truncated.smk";

    printf("Test: Opening truncated SMK file...\n");

    /* Create the truncated test file */
    if (!create_truncated_smk_file(test_file)) {
        printf("  FAIL: Could not create test file\n");
        return 0;
    }

    /* Open the file */
    FILE *fp = fopen(test_file, "rb");
    if (!fp) {
        printf("  FAIL: Could not open test file\n");
        remove(test_file);
        return 0;
    }

    /*
     * Call smacker_open() with the truncated file.
     *
     * Flow in smacker_open():
     * 1. read_header() succeeds (104-byte header is valid)
     * 2. read_frame_info() allocates frame_sizes, frame_offsets, frame_types
     * 3. read_frame_info() fread() fails (file is truncated)
     * 4. read_frame_info() frees the pointers
     *    - BUGGY: Does NOT nullify pointers
     *    - FIXED: Nullifies pointers via free_frame_info()
     * 5. read_frame_info() returns 0
     * 6. smacker_open() calls smacker_close()
     * 7. smacker_close() tries to free the pointers again
     *    - BUGGY: DOUBLE-FREE! (ASan will catch this)
     *    - FIXED: Safe, pointers are NULL
     */
    printf("  Calling smacker_open() on truncated file...\n");
    smacker s = smacker_open(fp);

    /* smacker_open() should return NULL on error */
    if (s != NULL) {
        printf("  FAIL: smacker_open() should return NULL for truncated file\n");
        smacker_close(s);
        remove(test_file);
        return 0;
    }

    /* If we reach here without ASan error, the code is fixed */
    printf("  PASS: Truncated file handled correctly (no double-free)\n");

    /* Note: fp is closed by smacker internals on error */
    remove(test_file);
    return 1;
}

/*
 * Test: Verify multiple error handling attempts don't cause issues
 */
static int test_multiple_error_attempts(void)
{
    const char *test_file = "/tmp/test_truncated2.smk";

    printf("Test: Multiple truncated file attempts...\n");

    for (int i = 0; i < 3; i++) {
        if (!create_truncated_smk_file(test_file)) {
            printf("  FAIL: Could not create test file (iteration %d)\n", i);
            return 0;
        }

        FILE *fp = fopen(test_file, "rb");
        if (!fp) {
            printf("  FAIL: Could not open test file (iteration %d)\n", i);
            remove(test_file);
            return 0;
        }

        smacker s = smacker_open(fp);
        if (s != NULL) {
            printf("  FAIL: Expected NULL return (iteration %d)\n", i);
            smacker_close(s);
            remove(test_file);
            return 0;
        }
    }

    printf("  PASS: Multiple error attempts handled correctly\n");
    remove(test_file);
    return 1;
}

int main(void)
{
    int passed = 0;
    int total = 2;

    printf("=== Smacker Double-Free Test Suite ===\n");
    printf("Testing against actual Julius smacker.c\n\n");

    if (test_truncated_file_handling()) {
        passed++;
    }

    if (test_multiple_error_attempts()) {
        passed++;
    }

    printf("\n=== Test Results ===\n");
    printf("Results: %d/%d tests passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
