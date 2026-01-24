/**
 * Test for buffer overflow in filename path construction
 *
 * Tests the file_construct_path function in src/core/file.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* External function from Julius src/core/file.c */
extern char *file_construct_path(const char *dir, const char *filename);

#define FILE_MAX_PATH_LENGTH 260

/* Helper to create a string of specified length */
static char *make_string(size_t len, char fill)
{
    char *str = malloc(len + 1);
    if (!str) return NULL;
    memset(str, fill, len);
    str[len] = '\0';
    return str;
}

/*
 * Test 1: Overflow with long path (ASan should catch this if buggy)
 */
void test_overflow_with_long_path(void)
{
    printf("Test: construct path with inputs that exceed buffer...\n");

    /* Create strings that together exceed FILE_MAX_PATH_LENGTH */
    char *long_dir = make_string(200, 'D');      /* 200 chars */
    char *long_file = make_string(100, 'F');     /* 100 chars */
    /* Total: 200 + 1 + 100 = 301 > 260 */

    printf("  Directory length: %zu\n", strlen(long_dir));
    printf("  Filename length: %zu\n", strlen(long_file));
    printf("  Total (with separator): %zu\n", strlen(long_dir) + 1 + strlen(long_file));
    printf("  Buffer size: %d\n", FILE_MAX_PATH_LENGTH);

    /* If buggy, this will overflow. If fixed, should return NULL or handle safely */
    char *path = file_construct_path(long_dir, long_file);

    if (path) {
        printf("  Result: path constructed (length %zu)\n", strlen(path));
        /* If we get here with a fixed version, it should have truncated or handled it */
        free(path);
    } else {
        printf("  Result: NULL (correctly rejected oversized path)\n");
    }

    free(long_dir);
    free(long_file);
    printf("  DONE\n");
}

/*
 * Test 2: Normal path should work
 */
void test_normal_path(void)
{
    printf("Test: construct normal path...\n");

    const char *dir = "/home/user/caesar3";
    const char *filename = "c3.eng";

    char *path = file_construct_path(dir, filename);

    printf("  Directory: %s\n", dir);
    printf("  Filename: %s\n", filename);
    printf("  Result: %s\n", path ? path : "(null)");

    assert(path != NULL);
    assert(strcmp(path, "/home/user/caesar3/c3.eng") == 0);

    free(path);
    printf("  PASS\n");
}

/*
 * Test 3: Path exactly at limit
 */
void test_path_at_limit(void)
{
    printf("Test: construct path exactly at buffer limit...\n");

    /* dir + '/' + filename should be exactly 259 chars (one less than buffer for null) */
    char *dir = make_string(200, 'D');
    char *filename = make_string(58, 'F');  /* 200 + 1 + 58 = 259 */

    printf("  Total length: %zu (limit: %d)\n",
           strlen(dir) + 1 + strlen(filename), FILE_MAX_PATH_LENGTH - 1);

    char *path = file_construct_path(dir, filename);

    /* This should succeed - exactly at limit */
    assert(path != NULL);
    printf("  Result: %s\n", path ? "constructed successfully" : "NULL");

    free(path);
    free(dir);
    free(filename);
    printf("  PASS\n");
}

/*
 * Test 4: Path one byte over limit should be rejected (if fixed)
 */
void test_path_over_limit(void)
{
    printf("Test: construct path one byte over limit...\n");

    char *dir = make_string(200, 'D');
    char *filename = make_string(60, 'F');  /* 200 + 1 + 60 = 261 > 260 */

    printf("  Total length: %zu (limit: %d)\n",
           strlen(dir) + 1 + strlen(filename), FILE_MAX_PATH_LENGTH - 1);

    char *path = file_construct_path(dir, filename);

    /* Fixed version should return NULL, buggy version will overflow */
    printf("  Result: %s\n", path ? "constructed (BUGGY!)" : "NULL (correct)");

    if (path) free(path);
    free(dir);
    free(filename);
    printf("  DONE\n");
}

/*
 * Test 5: NULL inputs should be handled
 */
void test_null_inputs(void)
{
    printf("Test: NULL inputs...\n");

    char *path1 = file_construct_path(NULL, "file.txt");
    char *path2 = file_construct_path("/dir", NULL);
    char *path3 = file_construct_path(NULL, NULL);

    assert(path1 == NULL);
    assert(path2 == NULL);
    assert(path3 == NULL);

    printf("  NULL dir: %s\n", path1 ? "ERROR" : "NULL (correct)");
    printf("  NULL file: %s\n", path2 ? "ERROR" : "NULL (correct)");
    printf("  Both NULL: %s\n", path3 ? "ERROR" : "NULL (correct)");

    printf("  PASS\n");
}

int main(void)
{
    printf("=== Path Buffer Overflow Test Suite ===\n\n");

    test_normal_path();
    test_null_inputs();
    test_path_at_limit();
    test_path_over_limit();
    test_overflow_with_long_path();

    printf("\n=== All tests completed ===\n");
    return 0;
}
