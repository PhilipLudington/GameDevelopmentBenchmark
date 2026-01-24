"""Parser for AddressSanitizer (ASan) output.

AddressSanitizer is a memory error detector that finds:
- Use after free
- Heap buffer overflow
- Stack buffer overflow
- Global buffer overflow
- Use after return
- Use after scope
- Memory leaks
- Double free
- Invalid free

This module parses ASan output to extract structured error information
for the Julius benchmark evaluation.
"""

import re
from dataclasses import dataclass, field
from enum import Enum
from typing import List, Optional


class ASanErrorType(Enum):
    """Types of AddressSanitizer errors."""

    HEAP_BUFFER_OVERFLOW = "heap-buffer-overflow"
    STACK_BUFFER_OVERFLOW = "stack-buffer-overflow"
    GLOBAL_BUFFER_OVERFLOW = "global-buffer-overflow"
    USE_AFTER_FREE = "heap-use-after-free"
    USE_AFTER_RETURN = "stack-use-after-return"
    USE_AFTER_SCOPE = "stack-use-after-scope"
    DOUBLE_FREE = "double-free"
    INVALID_FREE = "invalid-free"
    ALLOC_DEALLOC_MISMATCH = "alloc-dealloc-mismatch"
    MEMORY_LEAK = "memory-leak"
    STACK_OVERFLOW = "stack-overflow"
    NULL_DEREFERENCE = "null-dereference"
    UNKNOWN = "unknown"


@dataclass
class StackFrame:
    """A single frame in a stack trace."""

    frame_num: int
    address: str
    function: str
    file: Optional[str] = None
    line: Optional[int] = None
    column: Optional[int] = None
    module: Optional[str] = None


@dataclass
class StackTrace:
    """A complete stack trace."""

    frames: List[StackFrame] = field(default_factory=list)
    description: str = ""  # e.g., "freed by thread T0 here:"

    def get_top_user_frame(self) -> Optional[StackFrame]:
        """Get the topmost frame that's likely user code (not runtime/library)."""
        for frame in self.frames:
            if frame.file and not any(
                skip in frame.file
                for skip in ["asan_", "sanitizer_", "interceptors", "/usr/"]
            ):
                return frame
        return self.frames[0] if self.frames else None


@dataclass
class ASanError:
    """A single AddressSanitizer error."""

    error_type: ASanErrorType
    summary: str
    description: str
    address: Optional[str] = None
    size: Optional[int] = None
    stack_traces: List[StackTrace] = field(default_factory=list)
    raw_output: str = ""

    def get_location(self) -> Optional[str]:
        """Get the primary location of the error."""
        if self.stack_traces:
            frame = self.stack_traces[0].get_top_user_frame()
            if frame and frame.file:
                loc = frame.file
                if frame.line:
                    loc += f":{frame.line}"
                return loc
        return None


@dataclass
class ASanReport:
    """Complete ASan report potentially containing multiple errors."""

    errors: List[ASanError] = field(default_factory=list)
    memory_leaks: List[ASanError] = field(default_factory=list)
    raw_output: str = ""
    has_errors: bool = False
    has_leaks: bool = False

    @property
    def error_count(self) -> int:
        """Total number of errors (not counting leaks)."""
        return len(self.errors)

    @property
    def leak_count(self) -> int:
        """Number of memory leaks."""
        return len(self.memory_leaks)

    def get_error_types(self) -> List[ASanErrorType]:
        """Get list of unique error types found."""
        types = set()
        for error in self.errors:
            types.add(error.error_type)
        return list(types)


def parse_asan_output(output: str) -> ASanReport:
    """Parse AddressSanitizer output into structured report.

    Args:
        output: Combined stdout/stderr from program run with ASan

    Returns:
        ASanReport with parsed errors
    """
    report = ASanReport(raw_output=output)

    # Check if ASan detected anything
    if "AddressSanitizer" not in output and "LeakSanitizer" not in output:
        return report

    # Split into individual error reports
    # ASan errors start with "==PID==ERROR: AddressSanitizer:"
    error_pattern = r"=+\d+=+ERROR: AddressSanitizer: (.+?)(?==+\d+=+ERROR:|SUMMARY:|$)"
    error_matches = re.findall(error_pattern, output, re.DOTALL)

    for error_text in error_matches:
        error = _parse_single_error(error_text)
        if error:
            report.errors.append(error)
            report.has_errors = True

    # Parse leak reports
    leak_pattern = r"=+\d+=+ERROR: LeakSanitizer: (.+?)(?==+\d+=+ERROR:|SUMMARY:|$)"
    leak_matches = re.findall(leak_pattern, output, re.DOTALL)

    for leak_text in leak_matches:
        leak = _parse_leak_error(leak_text)
        if leak:
            report.memory_leaks.append(leak)
            report.has_leaks = True

    # Also check for leak summary without explicit ERROR
    if "detected memory leaks" in output.lower():
        report.has_leaks = True
        if not report.memory_leaks:
            # Extract leak details from summary
            leak = _parse_leak_summary(output)
            if leak:
                report.memory_leaks.append(leak)

    return report


def _parse_single_error(error_text: str) -> Optional[ASanError]:
    """Parse a single ASan error section.

    Args:
        error_text: Text of one error report

    Returns:
        ASanError or None if parsing fails
    """
    # Determine error type from first line
    first_line = error_text.split("\n")[0].strip()
    error_type = _classify_error(first_line)

    # Extract address if present
    address_match = re.search(r"on address (0x[0-9a-f]+)", error_text, re.IGNORECASE)
    address = address_match.group(1) if address_match else None

    # Extract size if present
    size_match = re.search(r"of size (\d+)", error_text)
    size = int(size_match.group(1)) if size_match else None

    # Parse stack traces
    stack_traces = _parse_stack_traces(error_text)

    # Build summary
    summary = first_line
    if address:
        summary = f"{error_type.value} at {address}"

    return ASanError(
        error_type=error_type,
        summary=summary,
        description=first_line,
        address=address,
        size=size,
        stack_traces=stack_traces,
        raw_output=error_text,
    )


def _parse_leak_error(leak_text: str) -> Optional[ASanError]:
    """Parse a LeakSanitizer error.

    Args:
        leak_text: Text of leak report

    Returns:
        ASanError for the leak
    """
    # Extract leak info
    direct_match = re.search(r"(\d+) byte\(s\) .* in (\d+) allocation", leak_text)

    if direct_match:
        bytes_leaked = int(direct_match.group(1))
        num_allocations = int(direct_match.group(2))
        summary = f"Memory leak: {bytes_leaked} bytes in {num_allocations} allocation(s)"
    else:
        summary = "Memory leak detected"
        bytes_leaked = None

    stack_traces = _parse_stack_traces(leak_text)

    return ASanError(
        error_type=ASanErrorType.MEMORY_LEAK,
        summary=summary,
        description=leak_text.split("\n")[0].strip(),
        size=bytes_leaked,
        stack_traces=stack_traces,
        raw_output=leak_text,
    )


def _parse_leak_summary(output: str) -> Optional[ASanError]:
    """Parse leak info from summary section.

    Args:
        output: Full program output

    Returns:
        ASanError for leaks or None
    """
    summary_match = re.search(
        r"SUMMARY: AddressSanitizer: (\d+) byte\(s\) leaked in (\d+) allocation",
        output,
    )

    if summary_match:
        bytes_leaked = int(summary_match.group(1))
        num_allocations = int(summary_match.group(2))

        return ASanError(
            error_type=ASanErrorType.MEMORY_LEAK,
            summary=f"Memory leak: {bytes_leaked} bytes in {num_allocations} allocation(s)",
            description="Memory leak detected",
            size=bytes_leaked,
            raw_output=output,
        )

    return None


def _classify_error(first_line: str) -> ASanErrorType:
    """Classify error type from first line of error report.

    Args:
        first_line: First line of error (after ERROR:)

    Returns:
        ASanErrorType enum value
    """
    first_line_lower = first_line.lower()

    if "heap-buffer-overflow" in first_line_lower:
        return ASanErrorType.HEAP_BUFFER_OVERFLOW
    elif "stack-buffer-overflow" in first_line_lower:
        return ASanErrorType.STACK_BUFFER_OVERFLOW
    elif "global-buffer-overflow" in first_line_lower:
        return ASanErrorType.GLOBAL_BUFFER_OVERFLOW
    elif "heap-use-after-free" in first_line_lower or "use-after-free" in first_line_lower:
        return ASanErrorType.USE_AFTER_FREE
    elif "stack-use-after-return" in first_line_lower:
        return ASanErrorType.USE_AFTER_RETURN
    elif "stack-use-after-scope" in first_line_lower:
        return ASanErrorType.USE_AFTER_SCOPE
    elif "double-free" in first_line_lower or "attempting double-free" in first_line_lower:
        return ASanErrorType.DOUBLE_FREE
    elif "invalid-free" in first_line_lower or "attempting free" in first_line_lower:
        return ASanErrorType.INVALID_FREE
    elif "alloc-dealloc-mismatch" in first_line_lower:
        return ASanErrorType.ALLOC_DEALLOC_MISMATCH
    elif "stack-overflow" in first_line_lower:
        return ASanErrorType.STACK_OVERFLOW
    elif "null" in first_line_lower and ("dereference" in first_line_lower or "access" in first_line_lower):
        return ASanErrorType.NULL_DEREFERENCE
    else:
        return ASanErrorType.UNKNOWN


def _parse_stack_traces(text: str) -> List[StackTrace]:
    """Parse all stack traces from error text.

    Args:
        text: Error report text

    Returns:
        List of StackTrace objects
    """
    traces = []
    current_trace = None
    current_description = ""

    lines = text.split("\n")
    i = 0

    while i < len(lines):
        line = lines[i].strip()

        # Check for trace header (e.g., "freed by thread T0 here:")
        if re.match(r"^\s*(freed|allocated|previously|READ|WRITE|caused by)", line, re.IGNORECASE):
            # Save previous trace if any
            if current_trace and current_trace.frames:
                traces.append(current_trace)

            current_description = line
            current_trace = StackTrace(description=current_description)
            i += 1
            continue

        # Parse stack frame
        # Format: #N 0xADDR in func file:line:col
        # or: #N 0xADDR (module+0xoffset)
        frame_match = re.match(
            r"^\s*#(\d+)\s+(0x[0-9a-f]+)\s+(?:in\s+)?(\S+)(?:\s+(.+))?",
            line,
            re.IGNORECASE,
        )

        if frame_match:
            if current_trace is None:
                current_trace = StackTrace()

            frame_num = int(frame_match.group(1))
            address = frame_match.group(2)
            function = frame_match.group(3)
            location = frame_match.group(4) or ""

            # Parse file:line:column from location
            file_path = None
            line_num = None
            column = None
            module = None

            file_match = re.match(r"(.+?):(\d+)(?::(\d+))?", location)
            if file_match:
                file_path = file_match.group(1)
                line_num = int(file_match.group(2))
                if file_match.group(3):
                    column = int(file_match.group(3))
            elif "+" in location:
                # Module+offset format
                module = location

            frame = StackFrame(
                frame_num=frame_num,
                address=address,
                function=function,
                file=file_path,
                line=line_num,
                column=column,
                module=module,
            )
            current_trace.frames.append(frame)

        i += 1

    # Don't forget the last trace
    if current_trace and current_trace.frames:
        traces.append(current_trace)

    return traces


def has_asan_error(output: str) -> bool:
    """Quick check if output contains any ASan errors.

    Args:
        output: Program output

    Returns:
        True if ASan errors detected
    """
    return "ERROR: AddressSanitizer:" in output or "ERROR: LeakSanitizer:" in output


def get_asan_summary(output: str) -> Optional[str]:
    """Extract ASan summary line from output.

    Args:
        output: Program output

    Returns:
        Summary line or None
    """
    match = re.search(r"SUMMARY: (?:Address|Leak)Sanitizer: (.+)", output)
    if match:
        return match.group(1)
    return None


def format_asan_report(report: ASanReport) -> str:
    """Format ASan report as human-readable text.

    Args:
        report: Parsed ASan report

    Returns:
        Formatted string
    """
    lines = []

    if report.has_errors:
        lines.append(f"AddressSanitizer found {report.error_count} error(s):")
        for i, error in enumerate(report.errors, 1):
            lines.append(f"\n  {i}. {error.error_type.value}")
            lines.append(f"     {error.summary}")
            loc = error.get_location()
            if loc:
                lines.append(f"     at: {loc}")

    if report.has_leaks:
        lines.append(f"\nLeakSanitizer found {report.leak_count} leak(s):")
        for leak in report.memory_leaks:
            lines.append(f"  - {leak.summary}")

    if not report.has_errors and not report.has_leaks:
        lines.append("No AddressSanitizer errors detected.")

    return "\n".join(lines)
