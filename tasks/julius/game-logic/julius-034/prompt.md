# Bug Report: Palace Upgrade Validation Allows Multiple Upgrades

## Summary

The palace upgrade validation incorrectly uses OR logic instead of AND logic, allowing players to build multiple palace upgrades when the game rules only permit one type of upgrade at a time.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/building/construction.c`
- Severity: Medium (gameplay rule violation, balance issue)

## Bug Description

In Caesar III, the governor's house can be upgraded to a villa, and the villa to a palace. The game should only allow ONE type of governor's residence in the city at any time. When checking if an upgrade is allowed, the code should verify that NO existing upgraded building exists.

The bug is in the validation logic:

```c
int can_upgrade_governors_house(void)
{
    // Check if any upgraded buildings exist
    int has_villa = city_buildings_has_governor_villa();
    int has_palace = city_buildings_has_governor_palace();

    // BUG: Using OR (||) when should use AND (&&)
    // This returns true if EITHER doesn't exist, not if BOTH don't exist
    if (!has_villa || !has_palace) {
        return 1;  // Allow upgrade
    }

    return 0;  // Deny upgrade
}
```

With this bug:
- If you have a villa but no palace: `!0 || !1` = `1 || 0` = 1 (allowed - WRONG!)
- If you have a palace but no villa: `!1 || !0` = `0 || 1` = 1 (allowed - WRONG!)
- Only if you have BOTH: `!1 || !1` = `0 || 0` = 0 (denied)

## Steps to Reproduce

1. Build a Governor's House
2. Upgrade it to a Governor's Villa
3. Build another Governor's House
4. Attempt to upgrade to Villa again
5. The upgrade is incorrectly allowed (should be denied)

## Expected Behavior

The validation should use AND logic:
- Only allow upgrade if NO villa AND NO palace exists
- `!has_villa && !has_palace`

## Current Behavior

The OR logic allows upgrade if EITHER doesn't exist:
- Allows upgrade if no villa (even if palace exists)
- Allows upgrade if no palace (even if villa exists)

## Relevant Code

Look at `src/building/construction.c`:
- `can_upgrade_governors_house()` function
- `can_upgrade_to_villa()` function
- `can_upgrade_to_palace()` function
- Boolean logic in upgrade validation

## Suggested Fix Approach

Change OR to AND in the validation:

```c
int can_upgrade_governors_house(void)
{
    int has_villa = city_buildings_has_governor_villa();
    int has_palace = city_buildings_has_governor_palace();

    // FIXED: Use AND - deny if ANY upgraded building exists
    if (!has_villa && !has_palace) {
        return 1;  // Allow upgrade only if neither exists
    }

    return 0;  // Deny upgrade
}
```

Or equivalently:
```c
    if (has_villa || has_palace) {
        return 0;  // Deny if either exists
    }
    return 1;
```

## Your Task

Fix the boolean logic error by using AND instead of OR. Your fix should:

1. Deny upgrade if a villa already exists
2. Deny upgrade if a palace already exists
3. Only allow upgrade if neither upgraded building exists
4. Maintain correct behavior for the initial house-to-villa upgrade

Provide your fix as a unified diff (patch).
