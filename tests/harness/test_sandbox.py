"""Tests for the sandbox module."""

import os
import tempfile
from pathlib import Path

import pytest

from harness.sandbox import Sandbox, SandboxConfig


class TestSandbox:
    """Tests for the Sandbox class."""

    def test_sandbox_creation(self):
        """Test sandbox can be created."""
        sandbox = Sandbox()
        assert sandbox.config is not None
        assert sandbox.working_dir is None

    def test_sandbox_setup(self, tmp_path):
        """Test sandbox setup with source directory."""
        # Create a source directory with a file
        source_dir = tmp_path / "source"
        source_dir.mkdir()
        (source_dir / "main.py").write_text("print('hello')")

        sandbox = Sandbox()
        sandbox.setup(source_dir)

        assert sandbox.working_dir is not None
        assert (sandbox.game_dir / "main.py").exists()

        sandbox.cleanup()

    def test_sandbox_cleanup(self, tmp_path):
        """Test sandbox cleanup removes temp directory."""
        source_dir = tmp_path / "source"
        source_dir.mkdir()
        (source_dir / "main.py").write_text("print('hello')")

        sandbox = Sandbox()
        sandbox.setup(source_dir)
        working_dir = sandbox.working_dir

        sandbox.cleanup()

        assert sandbox.working_dir is None
        assert not working_dir.exists()

    def test_sandbox_context_manager(self, tmp_path):
        """Test sandbox works as context manager."""
        source_dir = tmp_path / "source"
        source_dir.mkdir()
        (source_dir / "main.py").write_text("print('hello')")

        with Sandbox() as sandbox:
            sandbox.setup(source_dir)
            assert sandbox.working_dir is not None
            working_dir = sandbox.working_dir

        assert not working_dir.exists()

    def test_apply_changes(self, tmp_path):
        """Test applying code changes."""
        source_dir = tmp_path / "source"
        source_dir.mkdir()
        (source_dir / "main.py").write_text("print('original')")

        sandbox = Sandbox()
        sandbox.setup(source_dir)

        sandbox.apply_changes({"main.py": "print('modified')"})

        content = (sandbox.game_dir / "main.py").read_text()
        assert content == "print('modified')"

        sandbox.cleanup()

    def test_get_changes(self, tmp_path):
        """Test detecting changes."""
        source_dir = tmp_path / "source"
        source_dir.mkdir()
        (source_dir / "main.py").write_text("print('original')")

        sandbox = Sandbox()
        sandbox.setup(source_dir)

        # Modify file directly
        (sandbox.game_dir / "main.py").write_text("print('modified')")

        changes = sandbox.get_changes()
        assert "main.py" in changes
        assert changes["main.py"] == "print('modified')"

        sandbox.cleanup()


class TestSandboxConfig:
    """Tests for SandboxConfig."""

    def test_default_config(self):
        """Test default configuration values."""
        config = SandboxConfig()
        assert config.timeout == 60
        assert config.max_memory_mb == 512
        assert config.headless is True

    def test_custom_config(self):
        """Test custom configuration."""
        config = SandboxConfig(timeout=120, headless=False)
        assert config.timeout == 120
        assert config.headless is False
