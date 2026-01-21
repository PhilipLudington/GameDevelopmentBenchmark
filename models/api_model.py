"""API-based model implementations (OpenAI, Anthropic, etc.)."""

import os
from typing import Any

import httpx

from models.base import (
    GenerationResult,
    ModelConfig,
    ModelConnectionError,
    ModelError,
    ModelInterface,
    ModelRateLimitError,
    ModelTimeoutError,
)


class APIModel(ModelInterface):
    """Model interface for API-based models (OpenAI, Anthropic, etc.)."""

    def __init__(self, config: ModelConfig):
        """Initialize the API model.

        Args:
            config: Model configuration
        """
        super().__init__(config)
        self._client: httpx.Client | None = None
        self._setup_client()

    def _setup_client(self) -> None:
        """Set up the HTTP client with appropriate configuration."""
        self._client = httpx.Client(
            timeout=httpx.Timeout(self.config.timeout, connect=10.0),
        )

    def _get_api_key(self) -> str:
        """Get the API key for the configured provider.

        Returns:
            API key string

        Raises:
            ModelError: If no API key is found
        """
        env_var_map = {
            "openai": "OPENAI_API_KEY",
            "anthropic": "ANTHROPIC_API_KEY",
            "azure": "AZURE_OPENAI_API_KEY",
        }

        env_var = env_var_map.get(self.config.provider)
        if not env_var:
            # Check for custom API key in extra config
            if "api_key" in self.config.extra:
                return self.config.extra["api_key"]
            raise ModelError(f"No API key configuration for provider: {self.config.provider}")

        api_key = os.environ.get(env_var)
        if not api_key:
            raise ModelError(
                f"API key not found. Set the {env_var} environment variable."
            )
        return api_key

    def _get_endpoint(self) -> str:
        """Get the API endpoint for the configured provider.

        Returns:
            API endpoint URL
        """
        if "endpoint" in self.config.extra:
            return self.config.extra["endpoint"]

        endpoints = {
            "openai": "https://api.openai.com/v1/chat/completions",
            "anthropic": "https://api.anthropic.com/v1/messages",
            "azure": self.config.extra.get(
                "azure_endpoint",
                "https://{resource}.openai.azure.com/openai/deployments/{deployment}/chat/completions",
            ),
        }

        endpoint = endpoints.get(self.config.provider)
        if not endpoint:
            raise ModelError(f"No endpoint configured for provider: {self.config.provider}")
        return endpoint

    def _build_request_openai(self, prompt: str, context: dict[str, Any] | None) -> dict:
        """Build request payload for OpenAI API."""
        messages = []

        # Add system message if provided in context
        if context and "system" in context:
            messages.append({"role": "system", "content": context["system"]})

        # Add context messages if provided
        if context and "messages" in context:
            messages.extend(context["messages"])

        # Add the main prompt
        messages.append({"role": "user", "content": prompt})

        return {
            "model": self.config.model_id,
            "messages": messages,
            "temperature": self.config.temperature,
            "max_tokens": self.config.max_tokens,
        }

    def _build_request_anthropic(self, prompt: str, context: dict[str, Any] | None) -> dict:
        """Build request payload for Anthropic API."""
        messages = []

        # Add context messages if provided
        if context and "messages" in context:
            messages.extend(context["messages"])

        # Add the main prompt
        messages.append({"role": "user", "content": prompt})

        request = {
            "model": self.config.model_id,
            "messages": messages,
            "max_tokens": self.config.max_tokens,
            "temperature": self.config.temperature,
        }

        # Add system message if provided
        if context and "system" in context:
            request["system"] = context["system"]

        return request

    def _parse_response_openai(self, response: dict) -> GenerationResult:
        """Parse OpenAI API response."""
        choice = response["choices"][0]
        return GenerationResult(
            content=choice["message"]["content"],
            model=response.get("model", self.config.model_id),
            usage=response.get("usage"),
            finish_reason=choice.get("finish_reason"),
            raw_response=response,
        )

    def _parse_response_anthropic(self, response: dict) -> GenerationResult:
        """Parse Anthropic API response."""
        content_blocks = response.get("content", [])
        content = ""
        for block in content_blocks:
            if block.get("type") == "text":
                content += block.get("text", "")

        return GenerationResult(
            content=content,
            model=response.get("model", self.config.model_id),
            usage=response.get("usage"),
            finish_reason=response.get("stop_reason"),
            raw_response=response,
        )

    def generate(self, prompt: str, context: dict[str, Any] | None = None) -> GenerationResult:
        """Generate a response from the API model.

        Args:
            prompt: The input prompt
            context: Optional context with system message, previous messages, etc.

        Returns:
            GenerationResult with the model's response

        Raises:
            ModelError: If generation fails
        """
        if self._client is None:
            self._setup_client()

        try:
            api_key = self._get_api_key()
            endpoint = self._get_endpoint()

            # Build headers based on provider
            headers = {"Content-Type": "application/json"}
            if self.config.provider == "openai":
                headers["Authorization"] = f"Bearer {api_key}"
            elif self.config.provider == "anthropic":
                headers["x-api-key"] = api_key
                headers["anthropic-version"] = "2023-06-01"
            elif self.config.provider == "azure":
                headers["api-key"] = api_key

            # Build request payload
            if self.config.provider in ("openai", "azure"):
                payload = self._build_request_openai(prompt, context)
            elif self.config.provider == "anthropic":
                payload = self._build_request_anthropic(prompt, context)
            else:
                # Custom provider - use OpenAI format by default
                payload = self._build_request_openai(prompt, context)

            # Make request
            response = self._client.post(endpoint, headers=headers, json=payload)

            # Handle errors
            if response.status_code == 429:
                raise ModelRateLimitError("Rate limit exceeded")
            elif response.status_code >= 400:
                raise ModelError(f"API error {response.status_code}: {response.text}")

            response_data = response.json()

            # Parse response based on provider
            if self.config.provider in ("openai", "azure"):
                return self._parse_response_openai(response_data)
            elif self.config.provider == "anthropic":
                return self._parse_response_anthropic(response_data)
            else:
                # Try OpenAI format for custom providers
                return self._parse_response_openai(response_data)

        except httpx.TimeoutException:
            raise ModelTimeoutError(f"Request timed out after {self.config.timeout}s")
        except httpx.ConnectError as e:
            raise ModelConnectionError(f"Failed to connect: {e}")
        except Exception as e:
            if isinstance(e, ModelError):
                raise
            raise ModelError(f"Generation failed: {e}")

    def is_available(self) -> bool:
        """Check if the API is available.

        Returns:
            True if the API can be reached and authenticated
        """
        try:
            self._get_api_key()
            return True
        except ModelError:
            return False

    def __del__(self):
        """Clean up the HTTP client."""
        if self._client:
            self._client.close()


class OpenAIModel(APIModel):
    """Convenience class for OpenAI models."""

    def __init__(self, model_id: str = "gpt-4", **kwargs):
        config = ModelConfig(
            name=f"openai:{model_id}",
            provider="openai",
            model_id=model_id,
            **kwargs,
        )
        super().__init__(config)


class AnthropicModel(APIModel):
    """Convenience class for Anthropic models."""

    def __init__(self, model_id: str = "claude-3-opus-20240229", **kwargs):
        config = ModelConfig(
            name=f"anthropic:{model_id}",
            provider="anthropic",
            model_id=model_id,
            **kwargs,
        )
        super().__init__(config)
