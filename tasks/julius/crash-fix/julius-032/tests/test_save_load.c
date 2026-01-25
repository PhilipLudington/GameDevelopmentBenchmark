/*
 * Test for julius-032: Crash on malformed save file with bad header
 *
 * This test verifies that loading malformed save files is handled
 * gracefully without crashing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_MAGIC 0x434A5553  /* "JUSC" */
#define MAX_VERSION 10
#define MAX_DATA_SIZE (10 * 1024 * 1024)  /* 10 MB */

typedef struct {
    int magic;
    int version;
    int data_size;
    int checksum;
} save_header;

/* Test tracking */
static int error_logged = 0;
static int last_error_value = 0;
static int data_processed = 0;

/* Simulated functions */
void log_error(const char *msg, int param1, int param2)
{
    (void)msg;
    (void)param1;
    error_logged++;
    last_error_value = param2;
}

void process_save_data(char *data, int size)
{
    (void)data;
    (void)size;
    data_processed++;
}

/*
 * Load save file function - buggy or fixed version
 */
int load_save_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    save_header header;

#ifdef BUGGY_VERSION
    /* BUG: No validation of header values */
    fread(&header, sizeof(header), 1, f);

    /* This could fail with negative size or allocate huge memory */
    if (header.data_size <= 0 || header.data_size > MAX_DATA_SIZE) {
        /* Even in buggy version, prevent actual crash in test */
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

#else
    /* FIXED: Validate all header fields */
    if (fread(&header, sizeof(header), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    if (header.magic != SAVE_MAGIC) {
        log_error("Invalid save file magic", 0, header.magic);
        fclose(f);
        return 0;
    }

    if (header.version <= 0 || header.version > MAX_VERSION) {
        log_error("Invalid save version", 0, header.version);
        fclose(f);
        return 0;
    }

    if (header.data_size <= 0 || header.data_size > MAX_DATA_SIZE) {
        log_error("Invalid data size", 0, header.data_size);
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
#endif
}

/* Reset test state */
void reset_test_state(void)
{
    error_logged = 0;
    last_error_value = 0;
    data_processed = 0;
}

/* Create test save file */
void create_test_file(const char *filename, save_header *header, const char *data, int data_len)
{
    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(header, sizeof(*header), 1, f);
        if (data && data_len > 0) {
            fwrite(data, data_len, 1, f);
        }
        fclose(f);
    }
}

/* Test valid save file */
int test_valid_save(void)
{
    reset_test_state();

    save_header header = {
        .magic = SAVE_MAGIC,
        .version = 5,
        .data_size = 100,
        .checksum = 0
    };

    char data[100] = {0};
    create_test_file("test_valid.sav", &header, data, 100);

    int result = load_save_file("test_valid.sav");

    if (result != 1) {
        printf("FAIL: Valid save file should load successfully\n");
        return 1;
    }

    if (data_processed != 1) {
        printf("FAIL: Save data should be processed\n");
        return 1;
    }

    printf("PASS: Valid save file test\n");
    return 0;
}

/* Test invalid magic number */
int test_invalid_magic(void)
{
    reset_test_state();

    save_header header = {
        .magic = 0xDEADBEEF,  /* Wrong magic */
        .version = 5,
        .data_size = 100,
        .checksum = 0
    };

    char data[100] = {0};
    create_test_file("test_bad_magic.sav", &header, data, 100);

    int result = load_save_file("test_bad_magic.sav");

    if (result != 0) {
        printf("FAIL: Invalid magic should fail to load\n");
        return 1;
    }

#ifndef BUGGY_VERSION
    if (error_logged == 0) {
        printf("FAIL: Invalid magic should log error\n");
        return 1;
    }
#endif

    printf("PASS: Invalid magic test\n");
    return 0;
}

/* Test invalid version */
int test_invalid_version(void)
{
    reset_test_state();

    save_header header = {
        .magic = SAVE_MAGIC,
        .version = 999,  /* Too high */
        .data_size = 100,
        .checksum = 0
    };

    char data[100] = {0};
    create_test_file("test_bad_version.sav", &header, data, 100);

    int result = load_save_file("test_bad_version.sav");

    if (result != 0) {
        printf("FAIL: Invalid version should fail to load\n");
        return 1;
    }

#ifndef BUGGY_VERSION
    if (error_logged == 0) {
        printf("FAIL: Invalid version should log error\n");
        return 1;
    }
#endif

    printf("PASS: Invalid version test\n");
    return 0;
}

/* Test negative data size */
int test_negative_data_size(void)
{
    reset_test_state();

    save_header header = {
        .magic = SAVE_MAGIC,
        .version = 5,
        .data_size = -1000,  /* Negative */
        .checksum = 0
    };

    create_test_file("test_neg_size.sav", &header, NULL, 0);

    int result = load_save_file("test_neg_size.sav");

    if (result != 0) {
        printf("FAIL: Negative data size should fail to load\n");
        return 1;
    }

    printf("PASS: Negative data size test\n");
    return 0;
}

/* Test oversized data */
int test_oversized_data(void)
{
    reset_test_state();

    save_header header = {
        .magic = SAVE_MAGIC,
        .version = 5,
        .data_size = MAX_DATA_SIZE + 1000,  /* Too large */
        .checksum = 0
    };

    create_test_file("test_oversize.sav", &header, NULL, 0);

    int result = load_save_file("test_oversize.sav");

    if (result != 0) {
        printf("FAIL: Oversized data should fail to load\n");
        return 1;
    }

    printf("PASS: Oversized data test\n");
    return 0;
}

/* Test zero version */
int test_zero_version(void)
{
    reset_test_state();

    save_header header = {
        .magic = SAVE_MAGIC,
        .version = 0,  /* Invalid zero */
        .data_size = 100,
        .checksum = 0
    };

    char data[100] = {0};
    create_test_file("test_zero_ver.sav", &header, data, 100);

    int result = load_save_file("test_zero_ver.sav");

    if (result != 0) {
        printf("FAIL: Zero version should fail to load\n");
        return 1;
    }

    printf("PASS: Zero version test\n");
    return 0;
}

/* Test empty file */
int test_empty_file(void)
{
    reset_test_state();

    FILE *f = fopen("test_empty.sav", "wb");
    fclose(f);

    int result = load_save_file("test_empty.sav");

    if (result != 0) {
        printf("FAIL: Empty file should fail to load\n");
        return 1;
    }

    printf("PASS: Empty file test\n");
    return 0;
}

/* Test nonexistent file */
int test_nonexistent_file(void)
{
    reset_test_state();

    int result = load_save_file("nonexistent_file_12345.sav");

    if (result != 0) {
        printf("FAIL: Nonexistent file should fail to load\n");
        return 1;
    }

    printf("PASS: Nonexistent file test\n");
    return 0;
}

int main(void)
{
    int failures = 0;

    printf("=== Save File Load Test ===\n\n");

#ifdef BUGGY_VERSION
    printf("Testing BUGGY version (missing validation)\n\n");
#else
    printf("Testing FIXED version (should pass all tests)\n\n");
#endif

    failures += test_valid_save();
    failures += test_nonexistent_file();
    failures += test_empty_file();
    failures += test_invalid_magic();
    failures += test_invalid_version();
    failures += test_zero_version();
    failures += test_negative_data_size();
    failures += test_oversized_data();

    printf("\n=== Results: %d failures ===\n", failures);

    /* Cleanup test files */
    remove("test_valid.sav");
    remove("test_bad_magic.sav");
    remove("test_bad_version.sav");
    remove("test_neg_size.sav");
    remove("test_oversize.sav");
    remove("test_zero_ver.sav");
    remove("test_empty.sav");

    return failures;
}
