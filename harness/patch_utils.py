"""Utilities for working with git patches and unified diffs."""

import re
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


@dataclass
class PatchHunk:
    """Represents a single hunk in a unified diff."""

    old_start: int
    old_count: int
    new_start: int
    new_count: int
    lines: list[str]


@dataclass
class PatchFile:
    """Represents changes to a single file in a patch."""

    old_path: str
    new_path: str
    hunks: list[PatchHunk]
    is_new_file: bool = False
    is_deleted: bool = False
    is_binary: bool = False


@dataclass
class Patch:
    """Represents a complete patch that may modify multiple files."""

    files: list[PatchFile]
    raw_content: str


@dataclass
class PatchResult:
    """Result of applying a patch."""

    success: bool
    output: str
    error: Optional[str] = None
    files_modified: list[str] = None

    def __post_init__(self):
        if self.files_modified is None:
            self.files_modified = []


def parse_unified_diff(patch_content: str) -> Patch:
    """Parse a unified diff format patch.

    Supports git-style patches with:
    - diff --git headers
    - a/ and b/ prefixes
    - --- and +++ file markers
    - @@ hunk headers

    Args:
        patch_content: Raw patch content

    Returns:
        Parsed Patch object
    """
    files = []
    current_file: Optional[PatchFile] = None
    current_hunk: Optional[PatchHunk] = None

    lines = patch_content.split("\n")
    i = 0

    while i < len(lines):
        line = lines[i]

        # Git-style diff header
        if line.startswith("diff --git"):
            # Save previous file if any
            if current_file:
                if current_hunk:
                    current_file.hunks.append(current_hunk)
                files.append(current_file)

            # Parse git diff header: diff --git a/path b/path
            match = re.match(r"diff --git a/(.+) b/(.+)", line)
            if match:
                current_file = PatchFile(
                    old_path=match.group(1),
                    new_path=match.group(2),
                    hunks=[],
                )
            current_hunk = None
            i += 1
            continue

        # Check for new file mode
        if line.startswith("new file mode"):
            if current_file:
                current_file.is_new_file = True
            i += 1
            continue

        # Check for deleted file mode
        if line.startswith("deleted file mode"):
            if current_file:
                current_file.is_deleted = True
            i += 1
            continue

        # Check for binary file
        if line.startswith("Binary files"):
            if current_file:
                current_file.is_binary = True
            i += 1
            continue

        # Traditional diff header (--- file)
        if line.startswith("---"):
            if not current_file:
                # Traditional diff format without git header
                old_path = line[4:].strip()
                if old_path.startswith("a/"):
                    old_path = old_path[2:]
                # Look ahead for +++ line
                if i + 1 < len(lines) and lines[i + 1].startswith("+++"):
                    new_path = lines[i + 1][4:].strip()
                    if new_path.startswith("b/"):
                        new_path = new_path[2:]
                    current_file = PatchFile(
                        old_path=old_path,
                        new_path=new_path,
                        hunks=[],
                    )
                    i += 2
                    continue
            i += 1
            continue

        # Skip +++ line (handled above)
        if line.startswith("+++"):
            i += 1
            continue

        # Hunk header
        if line.startswith("@@"):
            # Save previous hunk if any
            if current_hunk and current_file:
                current_file.hunks.append(current_hunk)

            # Parse hunk header: @@ -start,count +start,count @@
            match = re.match(r"@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@", line)
            if match:
                current_hunk = PatchHunk(
                    old_start=int(match.group(1)),
                    old_count=int(match.group(2)) if match.group(2) else 1,
                    new_start=int(match.group(3)),
                    new_count=int(match.group(4)) if match.group(4) else 1,
                    lines=[],
                )
            i += 1
            continue

        # Hunk content (context, addition, or removal)
        if current_hunk is not None and line and line[0] in (" ", "+", "-", "\\"):
            current_hunk.lines.append(line)
            i += 1
            continue

        # Empty line might be part of hunk context
        if current_hunk is not None and line == "":
            # Check if this is genuinely empty context (space removed from start)
            # or end of hunk
            if i + 1 < len(lines):
                next_line = lines[i + 1]
                if next_line and next_line[0] in (" ", "+", "-"):
                    current_hunk.lines.append(" ")  # Empty context line
                    i += 1
                    continue

        i += 1

    # Save final file and hunk
    if current_file:
        if current_hunk:
            current_file.hunks.append(current_hunk)
        files.append(current_file)

    return Patch(files=files, raw_content=patch_content)


def apply_patch(
    patch: Patch | str,
    target_dir: Path,
    reverse: bool = False,
    strip: int = 1,
) -> PatchResult:
    """Apply a patch to a directory using git apply.

    Args:
        patch: Patch object or raw patch content
        target_dir: Directory to apply patch to
        reverse: If True, reverse the patch (revert changes)
        strip: Number of leading path components to strip (default: 1 for a/b prefixes)

    Returns:
        PatchResult with success status and output
    """
    if isinstance(patch, str):
        patch_content = patch
    else:
        patch_content = patch.raw_content

    # Write patch to temporary file
    with tempfile.NamedTemporaryFile(mode="w", suffix=".patch", delete=False) as f:
        f.write(patch_content)
        patch_file = Path(f.name)

    try:
        cmd = ["git", "apply", f"-p{strip}"]
        if reverse:
            cmd.append("-R")
        cmd.append(str(patch_file))

        result = subprocess.run(
            cmd,
            cwd=str(target_dir),
            capture_output=True,
            text=True,
        )

        files_modified = []
        if isinstance(patch, Patch):
            files_modified = [f.new_path for f in patch.files if not f.is_deleted]
            files_modified.extend([f.old_path for f in patch.files if f.is_deleted])

        if result.returncode == 0:
            return PatchResult(
                success=True,
                output=result.stdout,
                files_modified=files_modified,
            )
        else:
            return PatchResult(
                success=False,
                output=result.stdout,
                error=result.stderr,
                files_modified=[],
            )

    finally:
        patch_file.unlink()


def apply_patch_fallback(
    patch: Patch | str,
    target_dir: Path,
    reverse: bool = False,
) -> PatchResult:
    """Apply a patch using standard patch command (fallback when git not available).

    Args:
        patch: Patch object or raw patch content
        target_dir: Directory to apply patch to
        reverse: If True, reverse the patch

    Returns:
        PatchResult with success status and output
    """
    if isinstance(patch, str):
        patch_content = patch
    else:
        patch_content = patch.raw_content

    with tempfile.NamedTemporaryFile(mode="w", suffix=".patch", delete=False) as f:
        f.write(patch_content)
        patch_file = Path(f.name)

    try:
        cmd = ["patch", "-p1", "-d", str(target_dir)]
        if reverse:
            cmd.append("-R")

        result = subprocess.run(
            cmd,
            input=patch_content,
            capture_output=True,
            text=True,
        )

        files_modified = []
        if isinstance(patch, Patch):
            files_modified = [f.new_path for f in patch.files]

        if result.returncode == 0:
            return PatchResult(
                success=True,
                output=result.stdout,
                files_modified=files_modified,
            )
        else:
            return PatchResult(
                success=False,
                output=result.stdout,
                error=result.stderr,
                files_modified=[],
            )

    finally:
        patch_file.unlink()


def create_patch_from_diff(
    old_content: str,
    new_content: str,
    filename: str,
) -> str:
    """Create a unified diff patch from two file contents.

    Args:
        old_content: Original file content
        new_content: Modified file content
        filename: Name of the file

    Returns:
        Unified diff string
    """
    import difflib

    old_lines = old_content.splitlines(keepends=True)
    new_lines = new_content.splitlines(keepends=True)

    # Ensure files end with newline for proper diff
    if old_lines and not old_lines[-1].endswith("\n"):
        old_lines[-1] += "\n"
    if new_lines and not new_lines[-1].endswith("\n"):
        new_lines[-1] += "\n"

    diff = difflib.unified_diff(
        old_lines,
        new_lines,
        fromfile=f"a/{filename}",
        tofile=f"b/{filename}",
    )

    return "".join(diff)


def validate_patch(patch_content: str) -> tuple[bool, str]:
    """Validate that a patch is well-formed.

    Args:
        patch_content: Raw patch content

    Returns:
        Tuple of (is_valid, error_message)
    """
    try:
        patch = parse_unified_diff(patch_content)

        if not patch.files:
            return False, "No files found in patch"

        for file in patch.files:
            if not file.old_path and not file.new_path:
                return False, "Patch file missing path information"

            if not file.is_binary and not file.hunks and not file.is_new_file and not file.is_deleted:
                return False, f"Patch for {file.new_path} has no hunks"

        return True, ""

    except Exception as e:
        return False, f"Failed to parse patch: {e}"


def extract_model_patch(model_response: str) -> Optional[str]:
    """Extract a patch from a model's response.

    Looks for patches in code blocks or raw diff format.

    Args:
        model_response: Model's text response

    Returns:
        Extracted patch content or None if not found
    """
    # Look for diff/patch code blocks
    patterns = [
        r"```(?:diff|patch)\n(.*?)```",
        r"```\n(diff --git.*?)```",
        r"```\n(---.*?\+\+\+.*?)```",
    ]

    for pattern in patterns:
        match = re.search(pattern, model_response, re.DOTALL | re.IGNORECASE)
        if match:
            return match.group(1).strip()

    # Look for raw diff content (not in code block)
    if "diff --git" in model_response:
        # Find start of diff
        start = model_response.find("diff --git")
        # Find end (next code block or end of message)
        end = model_response.find("```", start)
        if end == -1:
            end = len(model_response)
        return model_response[start:end].strip()

    return None


def extract_complete_file(model_response: str) -> Optional[dict[str, str]]:
    """Extract complete file contents from a model's response.

    Looks for code blocks with filenames like:
    ```c src/path/to/file.c
    <file contents>
    ```

    Args:
        model_response: Model's text response

    Returns:
        Dictionary mapping filepath to file content, or None if not found
    """
    files = {}

    # Pattern for code blocks with filename: ```c filepath.c or ```c filepath
    # Supports: ```c src/file.c, ```c file.c, ``` src/file.c
    pattern = r"```(?:c|h|cpp)?\s+([^\n`]+\.(?:c|h|cpp))\n(.*?)```"

    matches = re.findall(pattern, model_response, re.DOTALL | re.IGNORECASE)

    for filepath, content in matches:
        filepath = filepath.strip()
        # Normalize path - remove leading ./ if present
        if filepath.startswith("./"):
            filepath = filepath[2:]
        files[filepath] = content.strip()

    # Also try pattern without language specifier
    if not files:
        pattern = r"```\s*([^\n`]+\.(?:c|h|cpp))\n(.*?)```"
        matches = re.findall(pattern, model_response, re.DOTALL)
        for filepath, content in matches:
            filepath = filepath.strip()
            if filepath.startswith("./"):
                filepath = filepath[2:]
            files[filepath] = content.strip()

    return files if files else None


def compare_patches(patch1: str, patch2: str) -> float:
    """Compare two patches for similarity.

    Returns a score from 0.0 (completely different) to 1.0 (identical).

    Args:
        patch1: First patch content
        patch2: Second patch content

    Returns:
        Similarity score
    """
    import difflib

    # Parse both patches
    p1 = parse_unified_diff(patch1)
    p2 = parse_unified_diff(patch2)

    if not p1.files or not p2.files:
        return 0.0

    # Compare file-by-file
    scores = []

    p2_files = {f.new_path: f for f in p2.files}

    for f1 in p1.files:
        if f1.new_path in p2_files:
            f2 = p2_files[f1.new_path]

            # Compare hunks
            h1_lines = []
            for hunk in f1.hunks:
                h1_lines.extend(hunk.lines)

            h2_lines = []
            for hunk in f2.hunks:
                h2_lines.extend(hunk.lines)

            # Use SequenceMatcher for similarity
            matcher = difflib.SequenceMatcher(None, h1_lines, h2_lines)
            scores.append(matcher.ratio())
        else:
            # File in patch1 but not in patch2
            scores.append(0.0)

    if not scores:
        return 0.0

    return sum(scores) / len(scores)
