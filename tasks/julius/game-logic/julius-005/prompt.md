# Bug Report: Hotkey Config Ordering Mismatch

## Summary

The hotkey configuration system has an ordering mismatch between the enum definitions in the header file and the string name array in the implementation. This causes hotkeys to be incorrectly mapped when reading/writing configuration files.

## Environment

- Project: Julius (Caesar III reimplementation)
- Files: `src/input/hotkey_config.h`, `src/input/hotkey_config.c`
- Severity: Medium (functionality issue, data corruption in config)

## Bug Description

The hotkey system uses an enum to identify different hotkey actions and a corresponding string array to map these to human-readable names for the config file. The enum in the header file defines:

```c
typedef enum {
    // ... other hotkeys ...
    HOTKEY_BUILD_CLEAR_LAND,
    HOTKEY_BUILD_VACANT_HOUSE,
    // ... more hotkeys ...
} hotkey_type;
```

However, in the implementation file, the string names array has these two entries in the **opposite order**:

```c
static const char *hotkey_names[] = {
    // ... other names ...
    "build_vacant_house",    // WRONG: should match HOTKEY_BUILD_CLEAR_LAND
    "build_clear_land",      // WRONG: should match HOTKEY_BUILD_VACANT_HOUSE
    // ... more names ...
};
```

This means when the config file is parsed:
- `build_vacant_house` in the INI file gets mapped to `HOTKEY_BUILD_CLEAR_LAND`
- `build_clear_land` in the INI file gets mapped to `HOTKEY_BUILD_VACANT_HOUSE`

## Steps to Reproduce

1. Set a custom hotkey for "Clear Land" in the options
2. Save the configuration
3. Reload the game
4. The hotkey now triggers "Vacant House" instead

Or programmatically:
1. Look up the string name for `HOTKEY_BUILD_CLEAR_LAND`
2. Observe it returns "build_vacant_house" instead of "build_clear_land"

## Expected Behavior

The enum value `HOTKEY_BUILD_CLEAR_LAND` should map to string `"build_clear_land"`, and `HOTKEY_BUILD_VACANT_HOUSE` should map to `"build_vacant_house"`.

## Current Behavior

The mapping is reversed, causing hotkey assignments to be swapped.

## Relevant Code

Look at:
- `src/input/hotkey_config.h` - the enum definition (defines the correct order)
- `src/input/hotkey_config.c` - the string names array (has wrong order)

The fix should modify the implementation to match the header.

## Suggested Fix Approach

Swap the order of the two string entries in `hotkey_names[]` to match the enum order:
- `"build_clear_land"` should come before `"build_vacant_house"`

## Your Task

Fix the ordering mismatch by correcting the string array order to match the enum definition. Your fix should:
1. Ensure enum values map to their correct string names
2. Be minimal - only swap the two affected entries
3. Not change the header file (the enum order is considered the source of truth)

Provide your fix as a unified diff (patch).
