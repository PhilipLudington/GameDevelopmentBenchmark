"""Tests for the base model module."""

import pytest

from models.base import (
    ModelConfig,
    GenerationResult,
    parse_model_string,
    create_model,
)


class TestModelConfig:
    """Tests for ModelConfig."""

    def test_default_values(self):
        """Test default configuration values."""
        config = ModelConfig(
            name="test",
            provider="openai",
            model_id="gpt-4",
        )
        assert config.temperature == 0.0
        assert config.max_tokens == 4096
        assert config.timeout == 120

    def test_custom_values(self):
        """Test custom configuration values."""
        config = ModelConfig(
            name="test",
            provider="openai",
            model_id="gpt-4",
            temperature=0.7,
            max_tokens=2048,
        )
        assert config.temperature == 0.7
        assert config.max_tokens == 2048


class TestGenerationResult:
    """Tests for GenerationResult."""

    def test_creation(self):
        """Test result creation."""
        result = GenerationResult(
            content="Hello world",
            model="gpt-4",
        )
        assert result.content == "Hello world"
        assert result.model == "gpt-4"
        assert result.usage is None


class TestParseModelString:
    """Tests for parse_model_string."""

    def test_valid_string(self):
        """Test parsing valid model string."""
        provider, model_id = parse_model_string("openai:gpt-4")
        assert provider == "openai"
        assert model_id == "gpt-4"

    def test_complex_model_id(self):
        """Test parsing model string with complex ID."""
        provider, model_id = parse_model_string("anthropic:claude-3-opus-20240229")
        assert provider == "anthropic"
        assert model_id == "claude-3-opus-20240229"

    def test_invalid_string(self):
        """Test parsing invalid model string raises error."""
        with pytest.raises(ValueError):
            parse_model_string("invalid")


class TestCreateModel:
    """Tests for create_model."""

    def test_create_mock_model(self):
        """Test creating mock model."""
        model = create_model("mock:pass")
        assert model is not None
        assert model.is_available()

    def test_create_api_model(self):
        """Test creating API model (may not be available)."""
        model = create_model("openai:gpt-4")
        assert model is not None

    def test_unknown_provider(self):
        """Test unknown provider raises error."""
        with pytest.raises(ValueError):
            create_model("unknown:model")
