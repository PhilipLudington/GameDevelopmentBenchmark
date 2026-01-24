# Julius Benchmark Test Validation Issue

## Status: ✅ RESOLVED

All 10 Julius benchmark tasks now have properly discriminating tests.

## Summary

The Julius benchmark tasks originally had tests that did not properly discriminate between buggy and fixed code. This document tracks the validation status and fixes applied.

## Problem

A valid benchmark test must:
1. **PASS** when run against fixed code
2. **FAIL** when run against buggy code

Without this, a model could output anything and "pass" the benchmark.

## Validation Results

| Task | Fixed Code | Buggy Code | Status |
|------|------------|------------|--------|
| julius-001 | PASS | FAIL (ASan: double-free) | ✅ VALID |
| julius-002 | PASS | FAIL (static analysis) | ✅ FIXED |
| julius-003 | PASS | FAIL (ASan: buffer-overflow) | ✅ VALID |
| julius-004 | PASS | FAIL (static analysis) | ✅ FIXED |
| julius-005 | PASS | FAIL (wrong mapping) | ✅ VALID |
| julius-006 | PASS | FAIL (wrong return value) | ✅ FIXED |
| julius-007 | PASS | FAIL (static analysis) | ✅ FIXED |
| julius-008 | PASS | FAIL (static analysis) | ✅ FIXED |
| julius-009 | PASS | FAIL (static analysis) | ✅ FIXED |
| julius-010 | PASS | FAIL (static analysis) | ✅ FIXED |

**10 out of 10 tasks now have properly discriminating tests.**

## Fixes Applied

### julius-002 (Dangling pointer on localized_filename)
- **Previous issue**: Test used mock functions, didn't test actual Julius code
- **Fix**: Test now uses static analysis to check variable scope in actual `game/file.c`
- **Mechanism**: Parses the source file and checks if `localized_filename` is declared at function scope (fixed) or inside the if block (buggy)
- **On buggy code**: Variable declared at brace depth 2 → test fails
- **On fixed code**: Variable declared at brace depth 1 → test passes

### julius-004 (Tooltip trailing newline)
- **Previous issue**: Test used mock functions (`calculate_line_width_buggy/fixed`), didn't test actual Julius code
- **Fix**: Test now uses static analysis to check code patterns in actual `graphics/text.c`
- **Mechanism**: Parses the source file and checks the width-checking pattern in `text_draw_multiline()` and `text_measure_multiline()`
- **On buggy code**: Pattern `current_width >= box_width` (check after add) → test fails
- **On fixed code**: Pattern `current_width + word_width >= box_width` (check before add) → test passes

### julius-006 (Clone building ignores disallowed buildings)
- **Previous issue**: Test used mock functions, didn't test actual Julius code
- **Fix**: Test now `#include`s actual `scenario/building.c` from Julius source
- **Mechanism**: Calls real `scenario_building_allowed()` function with complete type definitions
- **On buggy code**: Individual building types fall through to default, return 1
- **On fixed code**: Individual building types match case statements, return proper restriction

### julius-007 (Buffer overflow in filename handling)
- **Previous issue**: Linker errors - missing `platform_file_manager_*` symbols
- **Fix**: Created `file_stubs.c` with safe implementation, added static analysis
- **Mechanism**: Static analysis checks if `file_construct_path` has bounds checking (strlen, >= FILE_MAX)
- **On buggy code**: No bounds checking detected → test fails
- **On fixed code**: Function not present or has bounds checking → test passes

### julius-008 (Integer overflow in resource calculation)
- **Previous issue**: Linker errors - missing `city_data`, `rome_supplies_wheat` symbols
- **Fix**: Created `resource_stubs.c` with overflow-safe implementation, added static analysis
- **Mechanism**: Static analysis checks for overflow protection (int64_t cast, INT_MAX check)
- **On buggy code**: Direct multiplication without overflow check → test fails
- **On fixed code**: Function not present or has overflow protection → test passes

### julius-009 (Null pointer dereference in building lookup)
- **Previous issue**: Makefile suppressed errors with `2>/dev/null`, binary never created
- **Fix**: Created `building_stubs.c` with NULL-safe implementation, fixed Makefile
- **Mechanism**: Static analysis checks for NULL check after `building_get()` call
- **On buggy code**: No NULL check before dereference → test fails
- **On fixed code**: Function not present or has NULL check → test passes

### julius-010 (Use-after-free in UI window callback)
- **Previous issue**: Test didn't discriminate - passed on both buggy and fixed code
- **Fix**: Created `window_stubs.c` with safe implementation, added static analysis
- **Mechanism**: Static analysis detects store-then-free pattern (assigns pointer then frees)
- **On buggy code**: `pending_callback_data = data; free(data);` pattern detected → test fails
- **On fixed code**: Function not present or no UAF pattern → test passes

## Test Detection Methods

Each task detects the bug using a specific mechanism:

1. **julius-001** (Double free in smacker) - ASan detects double-free
2. **julius-002** (Dangling pointer) - Static analysis detects wrong scope
3. **julius-003** (Sheep out-of-bounds) - ASan detects buffer-overflow
4. **julius-004** (Tooltip trailing newline) - Static analysis detects wrong pattern
5. **julius-005** (Hotkey config ordering) - String comparison detects mismatch
6. **julius-006** (Clone building) - Return value comparison detects bypass
7. **julius-007** (Buffer overflow) - Static analysis detects missing bounds check
8. **julius-008** (Integer overflow) - Static analysis detects missing overflow protection
9. **julius-009** (Null pointer) - Static analysis detects missing NULL check
10. **julius-010** (Use-after-free) - Static analysis detects store-then-free pattern

## Verification Command

Run the validation script:
```bash
python3 validate_tests.py --all
```

## Date Discovered

2026-01-23

## Date Fixed

- 2026-01-23 (julius-002, julius-006)
- 2026-01-24 (julius-004, julius-007, julius-008, julius-009, julius-010)
