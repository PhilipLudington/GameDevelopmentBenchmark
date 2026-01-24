"""Sandbox environment for building and testing Julius (Caesar III reimplementation).

Julius is an open-source reimplementation of Caesar III that provides:
- Large C codebase (~100k lines)
- Real historical bugs from git history
- CMake build system
- Cross-platform support

This sandbox handles:
- Cloning Julius at specific commits
- Applying buggy patches to revert fixes
- Building with AddressSanitizer
- Managing build cache for performance
"""

import hashlib
import os
import shutil
import subprocess
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional

from harness.patch_utils import apply_patch, PatchResult


# Julius repository URL
JULIUS_REPO_URL = "https://github.com/bvschaik/julius.git"

# Default build cache location
DEFAULT_CACHE_DIR = Path.home() / ".cache" / "gdb_julius"


@dataclass
class JuliusSandboxConfig:
    """Configuration for the Julius sandbox environment."""

    # Build settings
    timeout: int = 300
    enable_asan: bool = True
    enable_ubsan: bool = False
    build_type: str = "Debug"  # Debug, Release, RelWithDebInfo

    # Cache settings
    use_cache: bool = True
    cache_dir: Path = field(default_factory=lambda: DEFAULT_CACHE_DIR)

    # Compiler settings
    cc: str = "clang"  # clang recommended for ASan
    cxx: str = "clang++"

    # CMake options
    cmake_options: Dict[str, str] = field(default_factory=dict)


@dataclass
class BuildResult:
    """Result of building Julius."""

    success: bool
    stdout: str
    stderr: str
    elapsed_time: float
    build_dir: Optional[Path] = None
    error: Optional[str] = None


@dataclass
class CloneResult:
    """Result of cloning Julius repository."""

    success: bool
    stdout: str
    stderr: str
    repo_dir: Optional[Path] = None
    error: Optional[str] = None


class JuliusSandbox:
    """Isolated environment for building and testing Julius.

    The Julius sandbox provides:
    - Git-based workflow (clone at commit, apply patches)
    - CMake build with AddressSanitizer support
    - Build caching for performance
    - Test execution environment
    """

    def __init__(self, config: Optional[JuliusSandboxConfig] = None):
        """Initialize the Julius sandbox.

        Args:
            config: Sandbox configuration (uses defaults if None)
        """
        self.config = config or JuliusSandboxConfig()
        self._temp_dir: Optional[Path] = None
        self._repo_dir: Optional[Path] = None
        self._build_dir: Optional[Path] = None
        self._original_commit: Optional[str] = None

        # Ensure cache directory exists
        if self.config.use_cache:
            self.config.cache_dir.mkdir(parents=True, exist_ok=True)

    def clone(
        self,
        commit: Optional[str] = None,
        branch: Optional[str] = None,
    ) -> CloneResult:
        """Clone Julius repository.

        Args:
            commit: Specific commit to checkout (optional)
            branch: Branch to checkout (optional, defaults to main)

        Returns:
            CloneResult with clone status
        """
        # Create temporary directory for this sandbox instance
        self._temp_dir = Path(tempfile.mkdtemp(prefix="gdb_julius_"))

        # Check cache for this commit
        if self.config.use_cache and commit:
            cached = self._get_cached_repo(commit)
            if cached:
                self._repo_dir = cached
                self._original_commit = commit
                return CloneResult(
                    success=True,
                    stdout="Using cached repository",
                    stderr="",
                    repo_dir=self._repo_dir,
                )

        # Clone fresh repository
        self._repo_dir = self._temp_dir / "julius"

        try:
            # Clone with minimal depth if checking out specific commit
            cmd = ["git", "clone"]
            if not commit:
                cmd.extend(["--depth", "1"])
            if branch:
                cmd.extend(["--branch", branch])
            cmd.extend([JULIUS_REPO_URL, str(self._repo_dir)])

            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
            )

            if result.returncode != 0:
                return CloneResult(
                    success=False,
                    stdout=result.stdout,
                    stderr=result.stderr,
                    error=f"Git clone failed: {result.stderr}",
                )

            # Checkout specific commit if requested
            if commit:
                checkout_result = subprocess.run(
                    ["git", "checkout", commit],
                    cwd=str(self._repo_dir),
                    capture_output=True,
                    text=True,
                    timeout=60,
                )

                if checkout_result.returncode != 0:
                    return CloneResult(
                        success=False,
                        stdout=checkout_result.stdout,
                        stderr=checkout_result.stderr,
                        error=f"Git checkout failed: {checkout_result.stderr}",
                    )

                self._original_commit = commit

                # Cache the repo if caching enabled
                if self.config.use_cache:
                    self._cache_repo(commit)

            return CloneResult(
                success=True,
                stdout=result.stdout,
                stderr=result.stderr,
                repo_dir=self._repo_dir,
            )

        except subprocess.TimeoutExpired:
            return CloneResult(
                success=False,
                stdout="",
                stderr="",
                error=f"Clone timeout after {self.config.timeout}s",
            )

        except Exception as e:
            return CloneResult(
                success=False,
                stdout="",
                stderr=str(e),
                error=str(e),
            )

    def apply_buggy_patch(self, patch_path: Path) -> PatchResult:
        """Apply a buggy patch to revert a fix.

        This is used to create the "broken" version of the code
        that the AI model needs to fix.

        Args:
            patch_path: Path to the buggy.patch file

        Returns:
            PatchResult with application status
        """
        if not self._repo_dir:
            return PatchResult(
                success=False,
                output="",
                error="Repository not cloned. Call clone() first.",
            )

        patch_content = patch_path.read_text()
        return apply_patch(patch_content, self._repo_dir)

    def apply_model_fix(self, patch_content: str) -> PatchResult:
        """Apply a model's proposed fix as a patch.

        Args:
            patch_content: Unified diff from model

        Returns:
            PatchResult with application status
        """
        if not self._repo_dir:
            return PatchResult(
                success=False,
                output="",
                error="Repository not cloned. Call clone() first.",
            )

        return apply_patch(patch_content, self._repo_dir)

    def apply_file_changes(self, changes: Dict[str, str]) -> None:
        """Apply direct file changes (not as patch).

        Args:
            changes: Dictionary mapping file paths to new contents
        """
        if not self._repo_dir:
            raise RuntimeError("Repository not cloned. Call clone() first.")

        for filepath, content in changes.items():
            target = self._repo_dir / filepath
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(content)

    def build(
        self,
        target: Optional[str] = None,
        extra_cmake_args: Optional[List[str]] = None,
    ) -> BuildResult:
        """Build Julius with CMake.

        Args:
            target: Specific CMake target to build (optional)
            extra_cmake_args: Additional CMake arguments

        Returns:
            BuildResult with build status
        """
        if not self._repo_dir:
            return BuildResult(
                success=False,
                stdout="",
                stderr="",
                elapsed_time=0,
                error="Repository not cloned. Call clone() first.",
            )

        start_time = time.time()

        # Create build directory
        self._build_dir = self._temp_dir / "build" if self._temp_dir else self._repo_dir / "build"
        self._build_dir.mkdir(parents=True, exist_ok=True)

        # Prepare CMake arguments
        cmake_args = [
            "cmake",
            str(self._repo_dir),
            f"-DCMAKE_BUILD_TYPE={self.config.build_type}",
            f"-DCMAKE_C_COMPILER={self.config.cc}",
            f"-DCMAKE_CXX_COMPILER={self.config.cxx}",
            # Compatibility with older CMakeLists.txt files
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        ]

        # Add sanitizer flags
        if self.config.enable_asan or self.config.enable_ubsan:
            sanitizers = []
            if self.config.enable_asan:
                sanitizers.append("address")
            if self.config.enable_ubsan:
                sanitizers.append("undefined")

            sanitizer_flag = "-fsanitize=" + ",".join(sanitizers)
            cmake_args.extend([
                f"-DCMAKE_C_FLAGS={sanitizer_flag} -fno-omit-frame-pointer -g",
                f"-DCMAKE_CXX_FLAGS={sanitizer_flag} -fno-omit-frame-pointer -g",
                f"-DCMAKE_EXE_LINKER_FLAGS={sanitizer_flag}",
            ])

        # Add custom CMake options
        for key, value in self.config.cmake_options.items():
            cmake_args.append(f"-D{key}={value}")

        # Add extra arguments
        if extra_cmake_args:
            cmake_args.extend(extra_cmake_args)

        # Set environment for ASan
        env = os.environ.copy()
        if self.config.enable_asan:
            env["ASAN_OPTIONS"] = "detect_leaks=1:abort_on_error=1:print_stacktrace=1"

        try:
            # Run CMake configure
            configure_result = subprocess.run(
                cmake_args,
                cwd=str(self._build_dir),
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
                env=env,
            )

            if configure_result.returncode != 0:
                elapsed = time.time() - start_time
                return BuildResult(
                    success=False,
                    stdout=configure_result.stdout,
                    stderr=configure_result.stderr,
                    elapsed_time=elapsed,
                    error=f"CMake configure failed: {configure_result.stderr}",
                )

            # Run make
            make_cmd = ["make", "-j4"]
            if target:
                make_cmd.append(target)

            build_result = subprocess.run(
                make_cmd,
                cwd=str(self._build_dir),
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
                env=env,
            )

            elapsed = time.time() - start_time

            if build_result.returncode != 0:
                return BuildResult(
                    success=False,
                    stdout=configure_result.stdout + build_result.stdout,
                    stderr=configure_result.stderr + build_result.stderr,
                    elapsed_time=elapsed,
                    error=f"Build failed: {build_result.stderr}",
                )

            return BuildResult(
                success=True,
                stdout=configure_result.stdout + build_result.stdout,
                stderr=configure_result.stderr + build_result.stderr,
                elapsed_time=elapsed,
                build_dir=self._build_dir,
            )

        except subprocess.TimeoutExpired:
            elapsed = time.time() - start_time
            return BuildResult(
                success=False,
                stdout="",
                stderr="",
                elapsed_time=elapsed,
                error=f"Build timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return BuildResult(
                success=False,
                stdout="",
                stderr=str(e),
                elapsed_time=elapsed,
                error=str(e),
            )

    def build_test(
        self,
        test_dir: Path,
        test_sources: List[str],
        output_name: str = "test_runner",
    ) -> BuildResult:
        """Build a standalone test against Julius source.

        This compiles a test file that links against specific Julius
        source files for unit testing.

        Args:
            test_dir: Directory containing test sources
            test_sources: List of test source files
            output_name: Name for test executable

        Returns:
            BuildResult with build status
        """
        if not self._repo_dir:
            return BuildResult(
                success=False,
                stdout="",
                stderr="",
                elapsed_time=0,
                error="Repository not cloned. Call clone() first.",
            )

        start_time = time.time()

        # Build directory for tests
        test_build_dir = self._temp_dir / "test_build" if self._temp_dir else Path(tempfile.mkdtemp())
        test_build_dir.mkdir(parents=True, exist_ok=True)

        # Compiler command
        compiler = self.config.cc
        compile_args = [
            compiler,
            "-Wall", "-Wextra",
            "-I", str(self._repo_dir / "src"),
            "-I", str(test_dir),
        ]

        # Add sanitizer flags
        if self.config.enable_asan:
            compile_args.extend(["-fsanitize=address", "-fno-omit-frame-pointer", "-g"])
        if self.config.enable_ubsan:
            compile_args.extend(["-fsanitize=undefined"])

        # Add source files
        for src in test_sources:
            compile_args.append(str(test_dir / src))

        # Output
        output_path = test_build_dir / output_name
        compile_args.extend(["-o", str(output_path)])

        # Link math library
        compile_args.append("-lm")

        env = os.environ.copy()
        if self.config.enable_asan:
            env["ASAN_OPTIONS"] = "detect_leaks=1:abort_on_error=1:print_stacktrace=1"

        try:
            result = subprocess.run(
                compile_args,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
                env=env,
            )

            elapsed = time.time() - start_time

            if result.returncode != 0:
                return BuildResult(
                    success=False,
                    stdout=result.stdout,
                    stderr=result.stderr,
                    elapsed_time=elapsed,
                    error=f"Test compilation failed: {result.stderr}",
                )

            return BuildResult(
                success=True,
                stdout=result.stdout,
                stderr=result.stderr,
                elapsed_time=elapsed,
                build_dir=test_build_dir,
            )

        except subprocess.TimeoutExpired:
            elapsed = time.time() - start_time
            return BuildResult(
                success=False,
                stdout="",
                stderr="",
                elapsed_time=elapsed,
                error=f"Test build timeout after {self.config.timeout}s",
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return BuildResult(
                success=False,
                stdout="",
                stderr=str(e),
                elapsed_time=elapsed,
                error=str(e),
            )

    def run_test(
        self,
        test_executable: Path,
        args: Optional[List[str]] = None,
        timeout: Optional[int] = None,
    ) -> "TestExecutionResult":
        """Run a compiled test executable.

        Args:
            test_executable: Path to test binary
            args: Command line arguments
            timeout: Execution timeout (uses config default if None)

        Returns:
            TestExecutionResult with test output
        """
        if not test_executable.exists():
            return TestExecutionResult(
                success=False,
                stdout="",
                stderr="",
                return_code=-1,
                elapsed_time=0,
                error=f"Test executable not found: {test_executable}",
            )

        start_time = time.time()
        timeout = timeout or self.config.timeout

        # Set up environment for ASan
        env = os.environ.copy()
        if self.config.enable_asan:
            env["ASAN_OPTIONS"] = "detect_leaks=1:abort_on_error=0:print_stacktrace=1"

        cmd = [str(test_executable)]
        if args:
            cmd.extend(args)

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=timeout,
                env=env,
            )

            elapsed = time.time() - start_time

            return TestExecutionResult(
                success=result.returncode == 0,
                stdout=result.stdout,
                stderr=result.stderr,
                return_code=result.returncode,
                elapsed_time=elapsed,
            )

        except subprocess.TimeoutExpired as e:
            elapsed = time.time() - start_time
            return TestExecutionResult(
                success=False,
                stdout=e.stdout or "" if hasattr(e, "stdout") else "",
                stderr=e.stderr or "" if hasattr(e, "stderr") else "",
                return_code=-1,
                elapsed_time=elapsed,
                error=f"Test timeout after {timeout}s",
                timed_out=True,
            )

        except Exception as e:
            elapsed = time.time() - start_time
            return TestExecutionResult(
                success=False,
                stdout="",
                stderr=str(e),
                return_code=-1,
                elapsed_time=elapsed,
                error=str(e),
            )

    def get_file_content(self, filepath: str) -> Optional[str]:
        """Get content of a file in the repository.

        Args:
            filepath: Path relative to repository root

        Returns:
            File content or None if file doesn't exist
        """
        if not self._repo_dir:
            return None

        target = self._repo_dir / filepath
        if target.exists():
            try:
                return target.read_text()
            except UnicodeDecodeError:
                return None
        return None

    def list_source_files(self, pattern: str = "*.c") -> List[str]:
        """List source files in the repository.

        Args:
            pattern: Glob pattern for files

        Returns:
            List of file paths relative to repository root
        """
        if not self._repo_dir:
            return []

        files = []
        for f in self._repo_dir.rglob(pattern):
            files.append(str(f.relative_to(self._repo_dir)))
        return files

    def _get_cached_repo(self, commit: str) -> Optional[Path]:
        """Get cached repository for a specific commit.

        Args:
            commit: Git commit hash

        Returns:
            Path to cached repo or None if not cached
        """
        cache_key = hashlib.sha256(commit.encode()).hexdigest()[:16]
        cached_path = self.config.cache_dir / f"julius_{cache_key}"

        if cached_path.exists():
            # Copy to temp directory
            dest = self._temp_dir / "julius"
            shutil.copytree(cached_path, dest)
            return dest

        return None

    def _cache_repo(self, commit: str) -> None:
        """Cache repository for a specific commit.

        Args:
            commit: Git commit hash
        """
        if not self._repo_dir:
            return

        cache_key = hashlib.sha256(commit.encode()).hexdigest()[:16]
        cached_path = self.config.cache_dir / f"julius_{cache_key}"

        # Only cache if not already cached
        if not cached_path.exists():
            shutil.copytree(self._repo_dir, cached_path)

    def cleanup(self) -> None:
        """Clean up temporary directories."""
        if self._temp_dir and self._temp_dir.exists():
            shutil.rmtree(self._temp_dir)
            self._temp_dir = None
            self._repo_dir = None
            self._build_dir = None

    def __enter__(self) -> "JuliusSandbox":
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit - cleanup."""
        self.cleanup()

    @property
    def repo_dir(self) -> Optional[Path]:
        """Get the repository directory."""
        return self._repo_dir

    @property
    def build_dir(self) -> Optional[Path]:
        """Get the build directory."""
        return self._build_dir

    @property
    def working_dir(self) -> Optional[Path]:
        """Get the sandbox working directory."""
        return self._temp_dir


@dataclass
class TestExecutionResult:
    """Result of running a test executable."""

    success: bool
    stdout: str
    stderr: str
    return_code: int
    elapsed_time: float
    error: Optional[str] = None
    timed_out: bool = False
    asan_errors: Optional[List[Dict]] = None
