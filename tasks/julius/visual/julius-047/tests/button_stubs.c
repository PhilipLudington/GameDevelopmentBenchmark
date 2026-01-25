/**
 * Stub implementation for julius-047 button highlight color test
 *
 * This provides the button color lookup functions for testing.
 */

#include <stdio.h>
#include <stdint.h>

typedef uint32_t color_t;

/* Color indices for button states */
typedef enum {
    COLOR_BUTTON_BACKGROUND = 0,
    COLOR_BUTTON_NORMAL = 1,
    COLOR_BUTTON_PRESSED = 2,
    COLOR_BUTTON_BORDER = 3,
    COLOR_BUTTON_HIGHLIGHT = 4,
    COLOR_MAX = 5
} button_color_index;

#define MAX_THEMES 3

/* Theme color palettes - different colors for each state */
static const color_t theme_colors[MAX_THEMES][COLOR_MAX] = {
    /* Theme 0: Default (Roman) */
    {
        0xFF8B7355,  /* Background - tan */
        0xFFAA9977,  /* Normal - light brown */
        0xFF665544,  /* Pressed - dark brown */
        0xFF444444,  /* Border - gray */
        0xFF88AACC   /* Highlight - light blue */
    },
    /* Theme 1: Dark */
    {
        0xFF333333,  /* Background - dark gray */
        0xFF555555,  /* Normal - medium gray */
        0xFF222222,  /* Pressed - very dark */
        0xFF666666,  /* Border - lighter gray */
        0xFF7799BB   /* Highlight - steel blue */
    },
    /* Theme 2: Classic */
    {
        0xFFC0A080,  /* Background - beige */
        0xFFD0B090,  /* Normal - light beige */
        0xFFA08060,  /* Pressed - darker beige */
        0xFF807060,  /* Border - brown-gray */
        0xFF90B0D0   /* Highlight - sky blue */
    }
};

/**
 * Get button background color for a theme
 */
color_t button_get_background_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_BACKGROUND];
}

/**
 * Get button normal color for a theme
 */
color_t button_get_normal_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_NORMAL];
}

/**
 * Get button pressed color for a theme
 */
color_t button_get_pressed_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_PRESSED];
}

/**
 * Get button border color for a theme
 */
color_t button_get_border_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_BORDER];
}

/**
 * Get button highlight color for a theme
 *
 * BUGGY version (commented):
 *   return theme_colors[theme_id][COLOR_BUTTON_BORDER];
 *
 * FIXED version:
 *   return theme_colors[theme_id][COLOR_BUTTON_HIGHLIGHT];
 */
color_t button_get_highlight_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }

    /* FIXED: Using COLOR_BUTTON_HIGHLIGHT for the correct hover color */
    return theme_colors[theme_id][COLOR_BUTTON_HIGHLIGHT];
}

/**
 * Helper function to get the expected highlight color (for testing)
 */
color_t get_expected_highlight_color(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_HIGHLIGHT];
}

/**
 * Helper function to get the border color (to detect the bug)
 */
color_t get_border_color_for_comparison(int theme_id)
{
    if (theme_id < 0 || theme_id >= MAX_THEMES) {
        theme_id = 0;
    }
    return theme_colors[theme_id][COLOR_BUTTON_BORDER];
}
