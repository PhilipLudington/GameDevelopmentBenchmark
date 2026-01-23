"""Parser for C code blocks from model responses."""

import re
from typing import Dict, List, Tuple


def parse_c_code_blocks(response: str) -> Dict[str, str]:
    """Parse C code blocks from model response.

    Extracts C/C++ code blocks and attempts to determine which file
    they belong to based on:
    1. Filename in code fence: ```c filename.c
    2. Comment at start: /* filename.c */ or // filename.c
    3. Based on content heuristics (headers vs implementation)

    Args:
        response: Model's response text

    Returns:
        Dictionary mapping filenames to code contents
    """
    changes: Dict[str, str] = {}

    # Pattern for fenced code blocks with optional filename
    # Matches: ```c filename.c or ```c or ```
    # Also handles cpp, h extensions
    pattern = r"```(?:c(?:pp)?|h)?\s*(?:([^\n`]+\.(?:c|h|cpp)))?\n(.*?)```"
    matches = re.findall(pattern, response, re.DOTALL | re.IGNORECASE)

    for filename, code in matches:
        filename = filename.strip() if filename else ""
        code = code.strip()

        if not code:
            continue

        # Try to extract filename from code comments
        if not filename:
            filename = _extract_filename_from_comments(code)

        # Try to infer filename from content
        if not filename:
            filename = _infer_filename_from_content(code)

        # Default based on whether it looks like a header
        if not filename:
            if _looks_like_header(code):
                filename = "quakedef.h"
            else:
                filename = "main.c"

        changes[filename] = code

    return changes


def _extract_filename_from_comments(code: str) -> str:
    """Extract filename from C-style comments at start of code.

    Args:
        code: C code content

    Returns:
        Extracted filename or empty string
    """
    lines = code.split("\n", 3)

    for line in lines[:3]:
        line = line.strip()

        # Check C-style comment: /* filename.c */
        match = re.search(r"/\*\s*(?:file:?\s*)?([^\s*/]+\.(?:c|h|cpp))\s*\*/", line, re.IGNORECASE)
        if match:
            return match.group(1)

        # Check C++ style comment: // filename.c
        match = re.search(r"//\s*(?:file:?\s*)?([^\s]+\.(?:c|h|cpp))", line, re.IGNORECASE)
        if match:
            return match.group(1)

        # Check preprocessor style: /* -*- filename.c -*- */
        match = re.search(r"([a-z_][a-z0-9_]*\.(?:c|h|cpp))", line, re.IGNORECASE)
        if match:
            # Make sure it's in a comment context
            if "/*" in line or "//" in line:
                return match.group(1)

    return ""


def _infer_filename_from_content(code: str) -> str:
    """Infer filename from C code content.

    Args:
        code: C code content

    Returns:
        Inferred filename or empty string
    """
    # Look for known Quake file patterns
    patterns = [
        # Function definitions that indicate specific files
        (r"\bR_RecursiveWorldNode\s*\(", "r_bsp.c"),
        (r"\bZ_Malloc\s*\(|Z_TagMalloc\s*\(", "zone.c"),
        (r"\bCL_AdjustAngles\s*\(|CL_BaseMove\s*\(", "cl_input.c"),
        (r"\bS_PaintChannelFrom\s*\(|SND_PaintChannelFrom\s*\(", "snd_mix.c"),
        (r"\bMod_DecompressVis\s*\(", "model.c"),
        (r"\bSV_LinkEdict\s*\(|SV_AreaEdicts\s*\(", "world.c"),
        (r"\bR_BuildLightMap\s*\(|R_AddDynamicLights\s*\(", "r_light.c"),
        (r"\bSkeleton_Init\s*\(|Skeleton_Blend\s*\(", "skeleton.c"),
        (r"\bR_SetupAliasFrame\s*\(", "r_alias.c"),
        (r"\bPortal_Create\s*\(|Portal_Cull\s*\(", "portal.c"),
        (r"\bTerrain_Init\s*\(|Terrain_GetHeight\s*\(", "terrain.c"),
    ]

    for pattern, filename in patterns:
        if re.search(pattern, code):
            return filename

    return ""


def _looks_like_header(code: str) -> bool:
    """Check if code looks like a header file.

    Args:
        code: C code content

    Returns:
        True if code appears to be a header file
    """
    # Header indicators
    header_patterns = [
        r"#ifndef\s+\w+_H",
        r"#define\s+\w+_H",
        r"#pragma\s+once",
        r"typedef\s+struct\s+\w+\s*\{",
        r"typedef\s+enum\s*\{",
    ]

    # Implementation indicators
    impl_patterns = [
        r"^\s*\w+\s+\w+\s*\([^)]*\)\s*\{",  # Function definition with body
        r"#include\s+[<\"][^>\"]+\.c[>\"]",  # Including .c file
    ]

    header_score = sum(1 for p in header_patterns if re.search(p, code, re.MULTILINE))
    impl_score = sum(1 for p in impl_patterns if re.search(p, code, re.MULTILINE))

    return header_score > impl_score


def extract_file_sections(response: str) -> List[Tuple[str, str]]:
    """Extract file sections from model response.

    Some models output files with clear section markers like:
    "Here's the fixed r_bsp.c:" followed by a code block.

    Args:
        response: Model's response text

    Returns:
        List of (filename, description) tuples found before code blocks
    """
    sections = []

    # Pattern for "filename.c:" or "Here's the fixed filename.c:"
    pattern = r"(?:(?:here'?s?|the|modified|fixed|updated)\s+)?([a-z_][a-z0-9_]*\.(?:c|h|cpp))\s*:"

    matches = re.finditer(pattern, response, re.IGNORECASE)
    for match in matches:
        filename = match.group(1)
        # Get context around the match
        start = max(0, match.start() - 50)
        end = min(len(response), match.end() + 50)
        context = response[start:end]
        sections.append((filename, context))

    return sections


def validate_c_code(code: str) -> Tuple[bool, str]:
    """Basic validation of C code syntax.

    Args:
        code: C code content

    Returns:
        Tuple of (is_valid, error_message)
    """
    # Check for basic structure
    if not code.strip():
        return False, "Empty code"

    # Check balanced braces
    brace_count = code.count("{") - code.count("}")
    if brace_count != 0:
        return False, f"Unbalanced braces: {brace_count:+d}"

    # Check balanced parentheses
    paren_count = code.count("(") - code.count(")")
    if paren_count != 0:
        return False, f"Unbalanced parentheses: {paren_count:+d}"

    # Check for common issues
    if "```" in code:
        return False, "Code contains markdown fence markers"

    return True, ""
