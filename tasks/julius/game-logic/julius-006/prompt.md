# Bug Report: Clone Building Ignores Disallowed Buildings

## Summary

The clone building functionality (triggered by hotkey) bypasses scenario building restrictions. While the menu correctly hides disallowed buildings, using the clone hotkey on a disallowed building type still allows placement.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/scenario/building.c`
- Severity: Medium (gameplay integrity issue, scenario bypass)

## Bug Description

In Caesar III scenarios, certain buildings can be disallowed to create specific challenges. The game has a validation function `scenario_building_allowed()` that checks if a building can be placed. However, this function only checks if the **building menu group** is allowed, not the **specific building type**.

For example, if the scenario disallows "Temple of Mars" but allows other temples:
- The build menu correctly shows only allowed temples
- But if the player has previously built a Temple of Mars and uses the "clone building" hotkey, they can still place more Temples of Mars

The problem is that buildings are organized into groups:
- Small temples: Ceres, Neptune, Mercury, Mars, Venus
- Large temples: same five variants
- Forts: Legionaries, Javelin, Mounted
- Farms: Wheat, Vegetable, Fruit, Olive, Vines, Pig
- etc.

The current code only checks: "Is the temples group allowed?" not "Is this specific temple allowed?"

## Steps to Reproduce

1. Create or load a scenario that allows temples but disallows Temple of Mars
2. Build a Temple of Mars before restrictions take effect (or find an existing one)
3. Select the Temple of Mars
4. Use the clone building hotkey
5. The game allows placing another Temple of Mars despite it being disallowed

## Expected Behavior

`scenario_building_allowed()` should check both:
1. Is the building group allowed?
2. Is this specific building type allowed within that group?

## Current Behavior

Only checks if the menu group is allowed, ignoring individual building type restrictions.

## Relevant Code

Look at `src/scenario/building.c`, specifically the `scenario_building_allowed()` function.

The fix requires adding individual case statements for building types that can be independently restricted:
- Small temples (5 types)
- Large temples (5 types)
- Forts (3 types)
- Farms (6 types)
- Raw materials (4 types)
- Workshops (5 types)

## Suggested Fix Approach

Before returning the menu group allowance, check if the specific building type has its own restriction:

```c
switch (building_type) {
    case BUILDING_SMALL_TEMPLE_CERES:
        if (!is_building_allowed[BUILDING_SMALL_TEMPLE_CERES])
            return 0;
        break;
    // ... other specific building types
}
return is_menu_group_allowed[get_menu_group(building_type)];
```

## Your Task

Fix the validation function to check individual building types, not just menu groups. Your fix should:
1. Add checks for all building types that can be independently restricted
2. Only return the menu group allowance if the specific building is also allowed
3. Handle all building groups listed in the bug description

Provide your fix as a unified diff (patch).
