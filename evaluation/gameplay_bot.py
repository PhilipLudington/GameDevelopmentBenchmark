"""Automated gameplay framework for testing game behavior."""

import os
import random
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum, auto
from typing import Any, Callable

# Set up headless mode
os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame


class Action(Enum):
    """Available game actions."""
    NONE = auto()
    UP = auto()
    DOWN = auto()
    LEFT = auto()
    RIGHT = auto()
    SPACE = auto()
    ESCAPE = auto()
    PAUSE = auto()


@dataclass
class GameplayResult:
    """Result of a gameplay session."""
    success: bool
    frames_played: int
    objectives_completed: list[str]
    final_state: dict[str, Any]
    events: list[dict[str, Any]]
    error: str | None = None


class GameplayBot(ABC):
    """Abstract base class for gameplay bots.

    Bots can:
    - Observe game state (screen, state dict)
    - Choose actions to perform
    - Track objectives
    - Report results
    """

    def __init__(self, objectives: list[str] | None = None):
        """Initialize the bot.

        Args:
            objectives: List of objective descriptions to complete
        """
        self.objectives = objectives or []
        self.completed_objectives: list[str] = []
        self.events: list[dict[str, Any]] = []
        self.frame_count = 0

    @abstractmethod
    def on_frame(self, surface: pygame.Surface, game_state: dict[str, Any]) -> None:
        """Process a frame of game state.

        Args:
            surface: Current game screen
            game_state: Dictionary with game state information
        """
        pass

    @abstractmethod
    def get_action(self) -> Action:
        """Get the next action to perform.

        Returns:
            Action to perform
        """
        pass

    def is_objective_complete(self, objective: str) -> bool:
        """Check if a specific objective is complete.

        Args:
            objective: Objective description

        Returns:
            True if objective is completed
        """
        return objective in self.completed_objectives

    def are_all_objectives_complete(self) -> bool:
        """Check if all objectives are complete.

        Returns:
            True if all objectives are completed
        """
        return all(obj in self.completed_objectives for obj in self.objectives)

    def mark_objective_complete(self, objective: str) -> None:
        """Mark an objective as completed.

        Args:
            objective: Objective that was completed
        """
        if objective not in self.completed_objectives:
            self.completed_objectives.append(objective)
            self.log_event("objective_complete", {"objective": objective})

    def log_event(self, event_type: str, data: dict[str, Any] | None = None) -> None:
        """Log a game event.

        Args:
            event_type: Type of event
            data: Additional event data
        """
        self.events.append({
            "type": event_type,
            "frame": self.frame_count,
            "data": data or {},
        })

    def reset(self) -> None:
        """Reset bot state for a new game."""
        self.completed_objectives = []
        self.events = []
        self.frame_count = 0


class RandomBot(GameplayBot):
    """Bot that takes random valid actions."""

    def __init__(
        self,
        valid_actions: list[Action] | None = None,
        action_probability: float = 0.3,
        **kwargs,
    ):
        """Initialize random bot.

        Args:
            valid_actions: List of valid actions (defaults to UP, DOWN, NONE)
            action_probability: Probability of taking a non-NONE action
        """
        super().__init__(**kwargs)
        self.valid_actions = valid_actions or [Action.UP, Action.DOWN, Action.NONE]
        self.action_probability = action_probability
        self.last_state: dict[str, Any] = {}

    def on_frame(self, surface: pygame.Surface, game_state: dict[str, Any]) -> None:
        """Process frame."""
        self.frame_count += 1
        self.last_state = game_state

    def get_action(self) -> Action:
        """Get random action."""
        if random.random() > self.action_probability:
            return Action.NONE
        return random.choice([a for a in self.valid_actions if a != Action.NONE])


class ScriptedBot(GameplayBot):
    """Bot that follows a predefined action sequence."""

    def __init__(self, script: list[tuple[int, Action]], loop: bool = False, **kwargs):
        """Initialize scripted bot.

        Args:
            script: List of (frame, action) tuples
            loop: Whether to loop the script
        """
        super().__init__(**kwargs)
        self.script = sorted(script, key=lambda x: x[0])
        self.loop = loop
        self.script_index = 0
        self.last_state: dict[str, Any] = {}

    def on_frame(self, surface: pygame.Surface, game_state: dict[str, Any]) -> None:
        """Process frame."""
        self.frame_count += 1
        self.last_state = game_state

    def get_action(self) -> Action:
        """Get next scripted action."""
        if self.script_index >= len(self.script):
            if self.loop:
                self.script_index = 0
            else:
                return Action.NONE

        frame, action = self.script[self.script_index]
        if self.frame_count >= frame:
            self.script_index += 1
            return action
        return Action.NONE


class PongBot(GameplayBot):
    """Bot that plays Pong by tracking the ball."""

    def __init__(self, skill_level: float = 0.8, **kwargs):
        """Initialize Pong bot.

        Args:
            skill_level: How good the bot is (0.0 to 1.0)
        """
        super().__init__(**kwargs)
        self.skill_level = skill_level
        self.last_state: dict[str, Any] = {}
        self.target_y: int | None = None

    def on_frame(self, surface: pygame.Surface, game_state: dict[str, Any]) -> None:
        """Process frame and update target."""
        self.frame_count += 1
        self.last_state = game_state

        # Track ball position
        ball_pos = game_state.get("ball_pos")
        if ball_pos:
            # Add some imperfection based on skill level
            if random.random() < self.skill_level:
                self.target_y = ball_pos[1]
            else:
                self.target_y = ball_pos[1] + random.randint(-50, 50)

        # Check objectives
        self._check_objectives(game_state)

    def _check_objectives(self, game_state: dict[str, Any]) -> None:
        """Check and mark completed objectives."""
        player1_score = game_state.get("player1_score", 0)
        player2_score = game_state.get("player2_score", 0)
        state_name = game_state.get("state", "")

        # Score-based objectives
        if player1_score >= 1 and "score_first_point" in self.objectives:
            self.mark_objective_complete("score_first_point")

        if player1_score >= 5 and "score_5_points" in self.objectives:
            self.mark_objective_complete("score_5_points")

        if state_name == "GAME_OVER" and player1_score > player2_score:
            if "win_game" in self.objectives:
                self.mark_objective_complete("win_game")

        # Track scoring events
        if self.last_state.get("player1_score", 0) < player1_score:
            self.log_event("player1_scored", {"score": player1_score})

    def get_action(self) -> Action:
        """Get action to move paddle towards ball."""
        if self.target_y is None:
            return Action.NONE

        player_y = self.last_state.get("player1_pos", 300)
        diff = self.target_y - player_y

        # Add reaction delay based on skill
        if abs(diff) < 10:
            return Action.NONE

        if diff < 0:
            return Action.UP
        else:
            return Action.DOWN


class GameplayRunner:
    """Runs a bot against a game for evaluation."""

    def __init__(
        self,
        game_module: Any,
        bot: GameplayBot,
        max_frames: int = 3600,
        timeout_seconds: int = 60,
    ):
        """Initialize the gameplay runner.

        Args:
            game_module: Module containing the game (must have Game class)
            bot: Bot to use for playing
            max_frames: Maximum frames to run
            timeout_seconds: Maximum time to run
        """
        self.game_module = game_module
        self.bot = bot
        self.max_frames = max_frames
        self.timeout_seconds = timeout_seconds

    def action_to_keys(self, action: Action) -> dict[int, bool]:
        """Convert action to key state.

        Args:
            action: Action to convert

        Returns:
            Dictionary mapping pygame key codes to pressed state
        """
        keys = {}
        if action == Action.UP:
            keys[pygame.K_UP] = True
            keys[pygame.K_w] = True
        elif action == Action.DOWN:
            keys[pygame.K_DOWN] = True
            keys[pygame.K_s] = True
        elif action == Action.LEFT:
            keys[pygame.K_LEFT] = True
            keys[pygame.K_a] = True
        elif action == Action.RIGHT:
            keys[pygame.K_RIGHT] = True
            keys[pygame.K_d] = True
        elif action == Action.SPACE:
            keys[pygame.K_SPACE] = True
        elif action == Action.ESCAPE:
            keys[pygame.K_ESCAPE] = True
        elif action == Action.PAUSE:
            keys[pygame.K_p] = True
        return keys

    def inject_action(self, action: Action) -> None:
        """Inject an action into pygame's event queue.

        Args:
            action: Action to inject
        """
        key_map = {
            Action.UP: pygame.K_UP,
            Action.DOWN: pygame.K_DOWN,
            Action.LEFT: pygame.K_LEFT,
            Action.RIGHT: pygame.K_RIGHT,
            Action.SPACE: pygame.K_SPACE,
            Action.ESCAPE: pygame.K_ESCAPE,
            Action.PAUSE: pygame.K_p,
        }

        if action in key_map:
            key = key_map[action]
            pygame.event.post(pygame.event.Event(pygame.KEYDOWN, key=key))
            pygame.event.post(pygame.event.Event(pygame.KEYUP, key=key))

    def run(self) -> GameplayResult:
        """Run the gameplay session.

        Returns:
            GameplayResult with session outcomes
        """
        import time

        # Create game instance
        Game = getattr(self.game_module, "Game", None)
        if Game is None:
            return GameplayResult(
                success=False,
                frames_played=0,
                objectives_completed=[],
                final_state={},
                events=[],
                error="Game class not found in module",
            )

        try:
            game = Game(headless=True)

            # Start game
            GameState = getattr(self.game_module, "GameState", None)
            if GameState:
                game.set_state(GameState.PLAYING)

            self.bot.reset()
            start_time = time.time()
            final_state: dict[str, Any] = {}

            # Run game loop
            for frame in range(self.max_frames):
                # Check timeout
                if time.time() - start_time > self.timeout_seconds:
                    self.bot.log_event("timeout", {"frame": frame})
                    break

                # Step game
                game_state = game.step()
                final_state = game_state

                # Process frame with bot
                surface = game.screen if hasattr(game, "screen") else None
                if surface:
                    self.bot.on_frame(surface, game_state)

                # Get and inject action
                action = self.bot.get_action()
                if action != Action.NONE:
                    self.inject_action(action)

                # Check if game ended
                if game_state.get("state") == "GAME_OVER":
                    self.bot.log_event("game_over", game_state)
                    break

                # Check if all objectives complete
                if self.bot.are_all_objectives_complete():
                    self.bot.log_event("all_objectives_complete", {"frame": frame})
                    break

                # Check if game is no longer running
                if not game.running:
                    break

            return GameplayResult(
                success=self.bot.are_all_objectives_complete(),
                frames_played=self.bot.frame_count,
                objectives_completed=self.bot.completed_objectives,
                final_state=final_state,
                events=self.bot.events,
            )

        except Exception as e:
            return GameplayResult(
                success=False,
                frames_played=self.bot.frame_count,
                objectives_completed=self.bot.completed_objectives,
                final_state={},
                events=self.bot.events,
                error=str(e),
            )


def create_pong_bot(objectives: list[str] | None = None, skill: float = 0.9) -> PongBot:
    """Create a Pong bot with default objectives.

    Args:
        objectives: Custom objectives (default: win the game)
        skill: Bot skill level

    Returns:
        Configured PongBot
    """
    default_objectives = objectives or ["win_game"]
    return PongBot(skill_level=skill, objectives=default_objectives)
