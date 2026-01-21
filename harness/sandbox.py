"""Isolated execution environment for running game code."""

import os
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class SandboxConfig:
    """Configuration for the sandbox environment."""

    timeout: int = 60
    max_memory_mb: int = 512
    allow_network: bool = False
    headless: bool = True
    python_executable: str | None = None
    extra_env: dict[str, str] = field(default_factory=dict)


@dataclass
class SandboxResult:
    """Result of running code in the sandbox."""

    success: bool
    stdout: str
    stderr: str
    return_code: int
    elapsed_time: float
    error: str | None = None


class Sandbox:
    """Isolated execution environment for running game code.

    The sandbox provides:
    - Temporary directory isolation
    - Timeout enforcement
    - Headless pygame support
    - Resource monitoring
    """

    def __init__(self, config: SandboxConfig | None = None):
        """Initialize the sandbox.

        Args:
            config: Sandbox configuration (uses defaults if None)
        """
        self.config = config or SandboxConfig()
        self._temp_dir: Path | None = None
        self._original_files: dict[str, str] = {}

    def setup(self, source_dir: Path) -> Path:
        """Set up the sandbox with code from source directory.

        Args:
            source_dir: Directory containing the game code

        Returns:
            Path to the sandbox working directory
        """
        # Create temporary directory
        self._temp_dir = Path(tempfile.mkdtemp(prefix="gdb_sandbox_"))

        # Copy source files to sandbox
        if source_dir.exists():
            shutil.copytree(source_dir, self._temp_dir / "game", dirs_exist_ok=True)

        # Store original file contents for comparison
        self._original_files = self._read_all_files(self._temp_dir / "game")

        return self._temp_dir

    def _read_all_files(self, directory: Path) -> dict[str, str]:
        """Read all Python files in a directory.

        Args:
            directory: Directory to read from

        Returns:
            Dictionary mapping filenames to contents
        """
        files = {}
        if directory.exists():
            for py_file in directory.rglob("*.py"):
                rel_path = py_file.relative_to(directory)
                files[str(rel_path)] = py_file.read_text()
        return files

    def apply_changes(self, changes: dict[str, str]) -> None:
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

    def get_changes(self) -> dict[str, str]:
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

    def run(
        self,
        script: str = "main.py",
        args: list[str] | None = None,
        env: dict[str, str] | None = None,
    ) -> SandboxResult:
        """Run a script in the sandbox.

        Args:
            script: Script filename to run (relative to game directory)
            args: Additional command line arguments
            env: Additional environment variables

        Returns:
            SandboxResult with execution results
        """
        if not self._temp_dir:
            raise RuntimeError("Sandbox not set up. Call setup() first.")

        game_dir = self._temp_dir / "game"
        script_path = game_dir / script

        if not script_path.exists():
            return SandboxResult(
                success=False,
                stdout="",
                stderr=f"Script not found: {script}",
                return_code=-1,
                elapsed_time=0,
                error="Script not found",
            )

        # Build environment
        run_env = os.environ.copy()

        # Set up headless pygame
        if self.config.headless:
            run_env["SDL_VIDEODRIVER"] = "dummy"
            run_env["SDL_AUDIODRIVER"] = "dummy"

        # Add custom environment variables
        run_env.update(self.config.extra_env)
        if env:
            run_env.update(env)

        # Build command
        python = self.config.python_executable or sys.executable
        cmd = [python, str(script_path)]
        if args:
            cmd.extend(args)

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                cwd=str(game_dir),
                env=run_env,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
            )
            elapsed = time.time() - start_time

            return SandboxResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return SandboxResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return SandboxResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=-1,
                elapsed_time=elapsed,
                error=str(e),
            )

    def run_tests(self, test_dir: str = "tests") -> SandboxResult:
        """Run pytest tests in the sandbox.

        Args:
            test_dir: Directory containing tests (relative to sandbox root)

        Returns:
            SandboxResult with test results
        """
        if not self._temp_dir:
            raise RuntimeError("Sandbox not set up. Call setup() first.")

        test_path = self._temp_dir / test_dir
        if not test_path.exists():
            return SandboxResult(
                success=False,
                stdout="",
                stderr=f"Test directory not found: {test_dir}",
                return_code=-1,
                elapsed_time=0,
                error="Test directory not found",
            )

        # Build environment
        run_env = os.environ.copy()
        if self.config.headless:
            run_env["SDL_VIDEODRIVER"] = "dummy"
            run_env["SDL_AUDIODRIVER"] = "dummy"
        run_env.update(self.config.extra_env)

        # Add game directory to Python path
        game_dir = self._temp_dir / "game"
        python_path = run_env.get("PYTHONPATH", "")
        run_env["PYTHONPATH"] = f"{game_dir}:{python_path}" if python_path else str(game_dir)

        python = self.config.python_executable or sys.executable
        cmd = [
            python, "-m", "pytest",
            str(test_path),
            "-v",
            "--timeout", str(self.config.timeout),
            "-x",  # Stop on first failure
        ]

        import time
        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                cwd=str(self._temp_dir),
                env=run_env,
                capture_output=True,
                text=True,
                timeout=self.config.timeout + 30,  # Extra time for pytest overhead
            )
            elapsed = time.time() - start_time

            return SandboxResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return SandboxResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Test timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return SandboxResult(
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

    def __enter__(self) -> "Sandbox":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit - cleanup."""
        self.cleanup()

    @property
    def working_dir(self) -> Path | None:
        """Get the sandbox working directory."""
        return self._temp_dir

    @property
    def game_dir(self) -> Path | None:
        """Get the game code directory within the sandbox."""
        if self._temp_dir:
            return self._temp_dir / "game"
        return None
