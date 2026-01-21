"""Abstract base class for model interfaces."""

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import Any


@dataclass
class ModelConfig:
    """Configuration for a model."""

    name: str
    provider: str
    model_id: str
    temperature: float = 0.0
    max_tokens: int = 4096
    timeout: int = 120
    extra: dict[str, Any] = field(default_factory=dict)


@dataclass
class GenerationResult:
    """Result of a model generation."""

    content: str
    model: str
    usage: dict[str, int] | None = None
    finish_reason: str | None = None
    raw_response: Any = None


class ModelInterface(ABC):
    """Abstract base class for AI model interfaces.

    This class defines the interface that all model implementations must follow.
    Models can be API-based (OpenAI, Anthropic) or CLI-based (ollama, llama.cpp).
    """

    def __init__(self, config: ModelConfig):
        """Initialize the model interface.

        Args:
            config: Model configuration
        """
        self.config = config

    @abstractmethod
    def generate(self, prompt: str, context: dict[str, Any] | None = None) -> GenerationResult:
        """Generate a response from the model.

        Args:
            prompt: The input prompt for the model
            context: Optional context dictionary with additional information
                    (e.g., file contents, previous attempts)

        Returns:
            GenerationResult containing the model's response

        Raises:
            ModelError: If generation fails
        """
        pass

    @abstractmethod
    def is_available(self) -> bool:
        """Check if the model is available and can accept requests.

        Returns:
            True if the model is ready to generate, False otherwise
        """
        pass

    def get_name(self) -> str:
        """Get the model's display name.

        Returns:
            Human-readable model name
        """
        return f"{self.config.provider}:{self.config.model_id}"

    def get_config(self) -> dict[str, Any]:
        """Get the model's configuration as a dictionary.

        Returns:
            Configuration dictionary
        """
        return {
            "name": self.config.name,
            "provider": self.config.provider,
            "model_id": self.config.model_id,
            "temperature": self.config.temperature,
            "max_tokens": self.config.max_tokens,
            "timeout": self.config.timeout,
            **self.config.extra,
        }


class ModelError(Exception):
    """Base exception for model-related errors."""

    pass


class ModelTimeoutError(ModelError):
    """Raised when a model request times out."""

    pass


class ModelRateLimitError(ModelError):
    """Raised when hitting rate limits."""

    pass


class ModelConnectionError(ModelError):
    """Raised when unable to connect to the model."""

    pass


def parse_model_string(model_string: str) -> tuple[str, str]:
    """Parse a model string in the format 'provider:model_id'.

    Args:
        model_string: String like 'openai:gpt-4' or 'anthropic:claude-3-opus'

    Returns:
        Tuple of (provider, model_id)

    Raises:
        ValueError: If the string format is invalid
    """
    if ":" not in model_string:
        raise ValueError(
            f"Invalid model string: '{model_string}'. "
            "Expected format: 'provider:model_id' (e.g., 'openai:gpt-4')"
        )

    parts = model_string.split(":", 1)
    return parts[0].lower(), parts[1]


def create_model(model_string: str, **kwargs) -> ModelInterface:
    """Create a model instance from a model string.

    Args:
        model_string: String like 'openai:gpt-4' or 'ollama:codellama'
        **kwargs: Additional configuration options

    Returns:
        ModelInterface instance

    Raises:
        ValueError: If the provider is unknown
    """
    from models.api_model import APIModel
    from models.cli_model import CLIModel

    provider, model_id = parse_model_string(model_string)

    # API-based providers
    api_providers = {"openai", "anthropic", "azure", "custom"}

    # CLI-based providers
    cli_providers = {"ollama", "llama", "llamacpp", "mock", "claude"}

    if provider in api_providers:
        config = ModelConfig(
            name=model_string,
            provider=provider,
            model_id=model_id,
            **kwargs,
        )
        return APIModel(config)
    elif provider in cli_providers:
        config = ModelConfig(
            name=model_string,
            provider=provider,
            model_id=model_id,
            **kwargs,
        )
        return CLIModel(config)
    else:
        raise ValueError(
            f"Unknown provider: '{provider}'. "
            f"Supported API providers: {api_providers}. "
            f"Supported CLI providers: {cli_providers}."
        )
