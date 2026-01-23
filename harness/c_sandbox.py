"""Isolated execution environment for compiling and running C code."""

import os
import shutil
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional


@dataclass
class CSandboxConfig:
    """Configuration for the C sandbox environment."""

    timeout: int = 120
    compiler: str = "gcc"
    compiler_flags: List[str] = field(default_factory=lambda: ["-Wall", "-Wextra", "-O2", "-std=c99"])
    linker_flags: List[str] = field(default_factory=lambda: ["-lm"])
    extra_env: Dict[str, str] = field(default_factory=dict)


@dataclass
class CompilationResult:
    """Result of compiling C code."""

    success: bool
    stdout: str
    stderr: str
    return_code: int
    elapsed_time: float
    executable_path: Optional[Path] = None
    error: Optional[str] = None


@dataclass
class ExecutionResult:
    """Result of running compiled C code."""

    success: bool
    stdout: str
    stderr: str
    return_code: int
    elapsed_time: float
    error: Optional[str] = None


class CSandbox:
    """Isolated execution environment for compiling and running C code.

    The C sandbox provides:
    - Temporary directory isolation
    - GCC/Clang compilation with configurable flags
    - Makefile-based build support
    - Timeout enforcement
    """

    def __init__(self, config: Optional[CSandboxConfig] = None):
        """Initialize the C sandbox.

        Args:
            config: Sandbox configuration (uses defaults if None)
        """
        self.config = config or CSandboxConfig()
        self._temp_dir: Optional[Path] = None
        self._original_files: Dict[str, str] = {}

    def setup(self, source_dir: Path) -> Path:
        """Set up the sandbox with code from source directory.

        Args:
            source_dir: Directory containing the C source code

        Returns:
            Path to the sandbox working directory
        """
        # Create temporary directory
        self._temp_dir = Path(tempfile.mkdtemp(prefix="gdb_c_sandbox_"))

        # Copy source files to sandbox
        if source_dir.exists():
            shutil.copytree(source_dir, self._temp_dir / "game", dirs_exist_ok=True)

        # Store original file contents for comparison
        self._original_files = self._read_all_files(self._temp_dir / "game")

        return self._temp_dir

    def _read_all_files(self, directory: Path) -> Dict[str, str]:
        """Read all C source and header files in a directory.

        Args:
            directory: Directory to read from

        Returns:
            Dictionary mapping filenames to contents
        """
        files = {}
        if directory.exists():
            for ext in ["*.c", "*.h", "*.cpp"]:
                for src_file in directory.rglob(ext):
                    rel_path = src_file.relative_to(directory)
                    try:
                        files[str(rel_path)] = src_file.read_text()
                    except UnicodeDecodeError:
                        # Skip binary files
                        pass
        return files

    def apply_changes(self, changes: Dict[str, str]) -> None:
        """Apply code changes to the sandbox.

        Args:
            changes: Dictionary mapping filenames to new contents
        """
        if not self._temp_dir:
            raise RuntimeError("Sandbox not set up. Call setup() first.")

        game_dir = self._temp_dir / "game"
        for filename, content in changes.items():
            file_path = game_dir / filename
            file_path.parent.mkdir(parents=True, exist_ok=True)
            file_path.write_text(content)

    def get_changes(self) -> Dict[str, str]:
        """Get the differences between original and current files.

        Returns:
            Dictionary mapping filenames to their current contents
            (only for files that changed)
        """
        if not self._temp_dir:
            return {}

        current_files = self._read_all_files(self._temp_dir / "game")
        changes = {}

        for filename, content in current_files.items():
            original = self._original_files.get(filename)
            if original != content:
                changes[filename] = content

        return changes

    def compile(
        self,
        source_files: Optional[List[str]] = None,
        output_name: str = "test_program",
        extra_flags: Optional[List[str]] = None,
    ) -> CompilationResult:
        """Compile C source files in the sandbox.

        Args:
            source_files: List of source files to compile (relative to game dir)
                         If None, compiles all .c files
            output_name: Name for the output executable
            extra_flags: Additional compiler flags

        Returns:
            CompilationResult with compilation outcome
        """
        if not self._temp_dir:
            raise RuntimeError("Sandbox not set up. Call setup() first.")

        game_dir = self._temp_dir / "game"
        output_path = self._temp_dir / output_name

        # Find source files
        if source_files:
            sources = [game_dir / f for f in source_files]
        else:
            sources = list(game_dir.glob("*.c"))

        if not sources:
            return CompilationResult(
                success=False,
                stdout="",
                stderr="No source files found",
                return_code=-1,
                elapsed_time=0,
                error="No source files found",
            )

        # Build compilation command
        cmd = [self.config.compiler]
        cmd.extend(self.config.compiler_flags)
        if extra_flags:
            cmd.extend(extra_flags)
        cmd.extend(["-I", str(game_dir)])  # Include game dir for headers
        cmd.extend([str(s) for s in sources])
        cmd.extend(["-o", str(output_path)])
        cmd.extend(self.config.linker_flags)

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                cwd=str(game_dir),
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
            )
            elapsed = time.time() - start_time

            return CompilationResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
                executable_path=output_path if result.returncode == 0 else None,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return CompilationResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Compilation timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return CompilationResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=-1,
                elapsed_time=elapsed,
                error=str(e),
            )

    def run_make(
        self,
        target: str = "test",
        makefile_dir: Optional[Path] = None,
    ) -> ExecutionResult:
        """Run a make target in the sandbox.

        Args:
            target: Make target to run
            makefile_dir: Directory containing Makefile (relative to sandbox root)
                         Defaults to tests directory

        Returns:
            ExecutionResult with make outcome
        """
        if not self._temp_dir:
            raise RuntimeError("Sandbox not set up. Call setup() first.")

        if makefile_dir:
            make_dir = self._temp_dir / makefile_dir
        else:
            make_dir = self._temp_dir / "tests"

        if not make_dir.exists():
            return ExecutionResult(
                success=False,
                stdout="",
                stderr=f"Makefile directory not found: {make_dir}",
                return_code=-1,
                elapsed_time=0,
                error="Makefile directory not found",
            )

        makefile = make_dir / "Makefile"
        if not makefile.exists():
            return ExecutionResult(
                success=False,
                stdout="",
                stderr=f"Makefile not found: {makefile}",
                return_code=-1,
                elapsed_time=0,
                error="Makefile not found",
            )

        # Build environment
        env = os.environ.copy()
        env.update(self.config.extra_env)

        cmd = ["make", "-C", str(make_dir), target]

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
                env=env,
            )
            elapsed = time.time() - start_time

            return ExecutionResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return ExecutionResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Make timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return ExecutionResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=-1,
                elapsed_time=elapsed,
                error=str(e),
            )

    def run_executable(
        self,
        executable: Path,
        args: Optional[List[str]] = None,
        env: Optional[Dict[str, str]] = None,
    ) -> ExecutionResult:
        """Run a compiled executable in the sandbox.

        Args:
            executable: Path to the executable
            args: Command line arguments
            env: Additional environment variables

        Returns:
            ExecutionResult with execution outcome
        """
        if not executable.exists():
            return ExecutionResult(
                success=False,
                stdout="",
                stderr=f"Executable not found: {executable}",
                return_code=-1,
                elapsed_time=0,
                error="Executable not found",
            )

        # Build environment
        run_env = os.environ.copy()
        run_env.update(self.config.extra_env)
        if env:
            run_env.update(env)

        cmd = [str(executable)]
        if args:
            cmd.extend(args)

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
                env=run_env,
            )
            elapsed = time.time() - start_time

            return ExecutionResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return ExecutionResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Execution timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return ExecutionResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=-1,
                elapsed_time=elapsed,
                error=str(e),
            )

    def cleanup(self) -> None:
        """Clean up the sandbox directory."""
        if self._temp_dir and self._temp_dir.exists():
            shutil.rmtree(self._temp_dir)
            self._temp_dir = None

    def __enter__(self) -> "CSandbox":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit - cleanup."""
        self.cleanup()

    @property
    def working_dir(self) -> Optional[Path]:
        """Get the sandbox working directory."""
        return self._temp_dir

    @property
    def game_dir(self) -> Optional[Path]:
        """Get the game code directory within the sandbox."""
        if self._temp_dir:
            return self._temp_dir / "game"
        return None
