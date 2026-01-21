"""CLI-based model implementations (ollama, llama.cpp, etc.)."""

import json
import subprocess
import shutil
from typing import Any

from models.base import (
    GenerationResult,
    ModelConfig,
    ModelConnectionError,
    ModelError,
    ModelInterface,
    ModelTimeoutError,
)


class CLIModel(ModelInterface):
    """Model interface for CLI-based models (ollama, llama.cpp, etc.)."""

    def __init__(self, config: ModelConfig):
        """Initialize the CLI model.

        Args:
            config: Model configuration
        """
        super().__init__(config)
        self._validate_setup()

    def _validate_setup(self) -> None:
        """Validate that the required CLI tools are available."""
        if self.config.provider == "mock":
            return

        cli_commands = {
            "ollama": "ollama",
            "llama": "llama",
            "llamacpp": "llama-cli",
            "claude": "claude",
        }

        command = cli_commands.get(self.config.provider)
        if command and not shutil.which(command):
            # Don't fail on init, just note it's not available
            pass

    def _build_ollama_command(self, prompt: str) -> list[str]:
        """Build command for ollama CLI."""
        return [
            "ollama",
            "run",
            self.config.model_id,
            prompt,
        ]

    def _build_llamacpp_command(self, prompt: str) -> list[str]:
        """Build command for llama.cpp CLI."""
        model_path = self.config.extra.get("model_path", self.config.model_id)
        cmd = [
            "llama-cli",
            "-m", model_path,
            "-p", prompt,
            "-n", str(self.config.max_tokens),
            "--temp", str(self.config.temperature),
        ]

        # Add any extra arguments
        if "extra_args" in self.config.extra:
            cmd.extend(self.config.extra["extra_args"])

        return cmd

    def _build_claude_command(self, prompt: str) -> list[str]:
        """Build command for Claude Code CLI."""
        cmd = [
            "claude",
            "-p", prompt,  # Print mode - single prompt, no interactive session
            "--output-format", "text",  # Plain text output
        ]

        # Add model selection if specified (e.g., claude:sonnet, claude:opus)
        model_id = self.config.model_id
        if model_id and model_id not in ("default", "claude"):
            cmd.extend(["--model", model_id])

        # Add any extra arguments
        if "extra_args" in self.config.extra:
            cmd.extend(self.config.extra["extra_args"])

        return cmd

    def _run_mock(self, prompt: str, context: dict[str, Any] | None) -> GenerationResult:
        """Run a mock model for testing.

        The mock model can be configured with different behaviors:
        - mock:pass - Returns a simple successful response
        - mock:fail - Raises an error
        - mock:echo - Echoes back the prompt
        """
        mode = self.config.model_id

        if mode == "fail":
            raise ModelError("Mock model configured to fail")
        elif mode == "echo":
            return GenerationResult(
                content=f"Echo: {prompt}",
                model="mock:echo",
            )
        elif mode == "code":
            # Return a mock code fix
            return GenerationResult(
                content="```python\n# Fixed code\npass\n```",
                model="mock:code",
            )
        else:
            # Default: pass mode
            return GenerationResult(
                content="Mock response: Task completed successfully.",
                model="mock:pass",
            )

    def generate(self, prompt: str, context: dict[str, Any] | None = None) -> GenerationResult:
        """Generate a response using CLI model.

        Args:
            prompt: The input prompt
            context: Optional context (used to build full prompt)

        Returns:
            GenerationResult with the model's response

        Raises:
            ModelError: If generation fails
        """
        # Handle mock provider
        if self.config.provider == "mock":
            return self._run_mock(prompt, context)

        # Build full prompt with context
        full_prompt = self._build_full_prompt(prompt, context)

        # Build command
        if self.config.provider == "ollama":
            cmd = self._build_ollama_command(full_prompt)
        elif self.config.provider in ("llama", "llamacpp"):
            cmd = self._build_llamacpp_command(full_prompt)
        elif self.config.provider == "claude":
            cmd = self._build_claude_command(full_prompt)
        else:
            raise ModelError(f"Unknown CLI provider: {self.config.provider}")

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=self.config.timeout,
            )

            if result.returncode != 0:
                raise ModelError(f"CLI command failed: {result.stderr}")

            content = result.stdout.strip()

            # For ollama, the response might include the prompt
            # Try to extract just the response
            if self.config.provider == "ollama":
                content = self._clean_ollama_response(content, full_prompt)

            return GenerationResult(
                content=content,
                model=self.get_name(),
            )

        except subprocess.TimeoutExpired:
            raise ModelTimeoutError(f"CLI command timed out after {self.config.timeout}s")
        except FileNotFoundError as e:
            raise ModelConnectionError(f"CLI tool not found: {e}")
        except Exception as e:
            if isinstance(e, ModelError):
                raise
            raise ModelError(f"CLI execution failed: {e}")

    def _build_full_prompt(self, prompt: str, context: dict[str, Any] | None) -> str:
        """Build the full prompt including context.

        Args:
            prompt: The main prompt
            context: Optional context dictionary

        Returns:
            Full prompt string
        """
        parts = []

        if context:
            # Add system message
            if "system" in context:
                parts.append(f"System: {context['system']}\n")

            # Add previous messages
            if "messages" in context:
                for msg in context["messages"]:
                    role = msg.get("role", "user").capitalize()
                    content = msg.get("content", "")
                    parts.append(f"{role}: {content}\n")

            # Add file contents
            if "files" in context:
                parts.append("Files:\n")
                for filename, content in context["files"].items():
                    parts.append(f"\n--- {filename} ---\n{content}\n")

        parts.append(f"User: {prompt}")

        return "\n".join(parts)

    def _clean_ollama_response(self, output: str, prompt: str) -> str:
        """Clean ollama output to extract just the response.

        Args:
            output: Raw CLI output
            prompt: Original prompt

        Returns:
            Cleaned response
        """
        # Ollama typically just returns the response, but sometimes
        # it might include thinking or other markers
        lines = output.split("\n")
        cleaned_lines = []
        in_response = True

        for line in lines:
            # Skip spinner/progress indicators
            if line.startswith("⠋") or line.startswith("⠙") or line.startswith("⠹"):
                continue
            if in_response:
                cleaned_lines.append(line)

        return "\n".join(cleaned_lines).strip()

    def is_available(self) -> bool:
        """Check if the CLI tool is available.

        Returns:
            True if the CLI tool can be found and executed
        """
        if self.config.provider == "mock":
            return True

        cli_commands = {
            "ollama": "ollama",
            "llama": "llama",
            "llamacpp": "llama-cli",
            "claude": "claude",
        }

        command = cli_commands.get(self.config.provider)
        if not command:
            return False

        if not shutil.which(command):
            return False

        # For ollama, also check if the model is available
        if self.config.provider == "ollama":
            try:
                result = subprocess.run(
                    ["ollama", "list"],
                    capture_output=True,
                    text=True,
                    timeout=10,
                )
                return self.config.model_id in result.stdout
            except Exception:
                return False

        return True


class OllamaModel(CLIModel):
    """Convenience class for Ollama models."""

    def __init__(self, model_id: str = "codellama", **kwargs):
        config = ModelConfig(
            name=f"ollama:{model_id}",
            provider="ollama",
            model_id=model_id,
            **kwargs,
        )
        super().__init__(config)


class MockModel(CLIModel):
    """Mock model for testing."""

    def __init__(self, mode: str = "pass", **kwargs):
        config = ModelConfig(
            name=f"mock:{mode}",
            provider="mock",
            model_id=mode,
            **kwargs,
        )
        super().__init__(config)


class ClaudeCodeModel(CLIModel):
    """Model using Claude Code CLI."""

    def __init__(self, model_id: str = "sonnet", **kwargs):
        # Set a longer timeout for Claude Code (it can take a while)
        if "timeout" not in kwargs:
            kwargs["timeout"] = 300  # 5 minutes
        config = ModelConfig(
            name=f"claude:{model_id}",
            provider="claude",
            model_id=model_id,
            **kwargs,
        )
        super().__init__(config)
