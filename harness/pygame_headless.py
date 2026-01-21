"""Headless pygame utilities for automated testing."""

import os
from dataclasses import dataclass
from typing import Any, Callable

# Set up headless mode before importing pygame
os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
os.environ.setdefault("SDL_AUDIODRIVER", "dummy")

import pygame


@dataclass
class FrameData:
    """Data captured from a single frame."""

    frame_number: int
    surface: pygame.Surface
    game_state: dict[str, Any]
    elapsed_ms: float


@dataclass
class InputEvent:
    """Represents an input event to inject."""

    type: int  # pygame event type
    key: int | None = None  # Key code for keyboard events
    button: int | None = None  # Button for mouse events
    pos: tuple[int, int] | None = None  # Position for mouse events
    rel: tuple[int, int] | None = None  # Relative motion for mouse events

    def to_pygame_event(self) -> pygame.event.Event:
        """Convert to a pygame event."""
        attrs = {}
        if self.key is not None:
            attrs["key"] = self.key
        if self.button is not None:
            attrs["button"] = self.button
        if self.pos is not None:
            attrs["pos"] = self.pos
        if self.rel is not None:
            attrs["rel"] = self.rel
        return pygame.event.Event(self.type, **attrs)


class HeadlessGame:
    """Wrapper for running pygame games in headless mode.

    This class provides utilities for:
    - Running games without a display
    - Capturing frame data
    - Injecting input events
    - Extracting game state
    """

    def __init__(
        self,
        width: int = 800,
        height: int = 600,
        fps: int = 60,
    ):
        """Initialize headless game environment.

        Args:
            width: Display width
            height: Display height
            fps: Target frames per second
        """
        self.width = width
        self.height = height
        self.fps = fps
        self.screen: pygame.Surface | None = None
        self.clock: pygame.time.Clock | None = None
        self.frame_count = 0
        self._initialized = False

    def init(self) -> None:
        """Initialize pygame in headless mode."""
        if self._initialized:
            return

        pygame.init()
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.clock = pygame.time.Clock()
        self._initialized = True

    def quit(self) -> None:
        """Clean up pygame."""
        if self._initialized:
            pygame.quit()
            self._initialized = False
            self.screen = None
            self.clock = None

    def capture_frame(self, game_state: dict[str, Any] | None = None) -> FrameData:
        """Capture the current frame.

        Args:
            game_state: Optional game state dictionary

        Returns:
            FrameData with frame information
        """
        if not self._initialized or not self.screen:
            raise RuntimeError("HeadlessGame not initialized. Call init() first.")

        # Copy the current screen surface
        surface_copy = self.screen.copy()

        self.frame_count += 1
        elapsed = self.clock.get_time() if self.clock else 0

        return FrameData(
            frame_number=self.frame_count,
            surface=surface_copy,
            game_state=game_state or {},
            elapsed_ms=elapsed,
        )

    def inject_events(self, events: list[InputEvent]) -> None:
        """Inject input events into pygame's event queue.

        Args:
            events: List of InputEvent objects to inject
        """
        for event in events:
            pygame.event.post(event.to_pygame_event())

    def inject_key_press(self, key: int) -> None:
        """Inject a key press event.

        Args:
            key: pygame key constant (e.g., pygame.K_SPACE)
        """
        self.inject_events([
            InputEvent(type=pygame.KEYDOWN, key=key),
        ])

    def inject_key_release(self, key: int) -> None:
        """Inject a key release event.

        Args:
            key: pygame key constant
        """
        self.inject_events([
            InputEvent(type=pygame.KEYUP, key=key),
        ])

    def inject_mouse_click(self, pos: tuple[int, int], button: int = 1) -> None:
        """Inject a mouse click event.

        Args:
            pos: Click position (x, y)
            button: Mouse button (1=left, 2=middle, 3=right)
        """
        self.inject_events([
            InputEvent(type=pygame.MOUSEBUTTONDOWN, pos=pos, button=button),
            InputEvent(type=pygame.MOUSEBUTTONUP, pos=pos, button=button),
        ])

    def inject_mouse_move(self, pos: tuple[int, int]) -> None:
        """Inject a mouse motion event.

        Args:
            pos: New mouse position (x, y)
        """
        self.inject_events([
            InputEvent(type=pygame.MOUSEMOTION, pos=pos),
        ])

    def tick(self) -> float:
        """Advance the clock by one frame.

        Returns:
            Time elapsed since last tick in milliseconds
        """
        if not self.clock:
            raise RuntimeError("HeadlessGame not initialized")
        return self.clock.tick(self.fps)

    def get_surface(self) -> pygame.Surface:
        """Get the display surface.

        Returns:
            The main display surface
        """
        if not self.screen:
            raise RuntimeError("HeadlessGame not initialized")
        return self.screen

    def run_frames(
        self,
        update_fn: Callable[[pygame.Surface], dict[str, Any] | None],
        num_frames: int,
        capture_interval: int = 1,
    ) -> list[FrameData]:
        """Run the game for a number of frames.

        Args:
            update_fn: Function that updates the game and returns game state
            num_frames: Number of frames to run
            capture_interval: Capture every Nth frame

        Returns:
            List of captured FrameData
        """
        self.init()
        captured_frames = []

        for i in range(num_frames):
            # Process events
            pygame.event.pump()

            # Update game
            game_state = update_fn(self.screen)

            # Capture if needed
            if i % capture_interval == 0:
                captured_frames.append(self.capture_frame(game_state))

            # Tick clock
            self.tick()

        return captured_frames

    def __enter__(self) -> "HeadlessGame":
        """Context manager entry."""
        self.init()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit."""
        self.quit()


def setup_headless() -> None:
    """Set up environment variables for headless pygame.

    Call this before importing pygame in any module.
    """
    os.environ["SDL_VIDEODRIVER"] = "dummy"
    os.environ["SDL_AUDIODRIVER"] = "dummy"


def is_headless() -> bool:
    """Check if running in headless mode.

    Returns:
        True if headless mode is enabled
    """
    return os.environ.get("SDL_VIDEODRIVER") == "dummy"


def surface_to_array(surface: pygame.Surface) -> "np.ndarray":
    """Convert a pygame surface to a numpy array.

    Args:
        surface: Pygame surface to convert

    Returns:
        Numpy array with shape (height, width, 3)
    """
    try:
        import numpy as np
        return np.array(pygame.surfarray.array3d(surface)).transpose((1, 0, 2))
    except ImportError:
        raise ImportError("numpy required for surface_to_array")


def save_frame(surface: pygame.Surface, path: str) -> None:
    """Save a surface to an image file.

    Args:
        surface: Surface to save
        path: Output file path
    """
    pygame.image.save(surface, path)


def compare_surfaces(
    surface1: pygame.Surface,
    surface2: pygame.Surface,
    threshold: float = 0.01,
) -> bool:
    """Compare two surfaces for similarity.

    Args:
        surface1: First surface
        surface2: Second surface
        threshold: Maximum allowed difference ratio (0.0 to 1.0)

    Returns:
        True if surfaces are similar within threshold
    """
    if surface1.get_size() != surface2.get_size():
        return False

    try:
        import numpy as np
        arr1 = surface_to_array(surface1)
        arr2 = surface_to_array(surface2)
        diff = np.abs(arr1.astype(float) - arr2.astype(float))
        diff_ratio = np.mean(diff) / 255.0
        return diff_ratio <= threshold
    except ImportError:
        # Fallback: compare pixel by pixel (slower)
        width, height = surface1.get_size()
        diff_count = 0
        total = width * height

        for x in range(width):
            for y in range(height):
                if surface1.get_at((x, y)) != surface2.get_at((x, y)):
                    diff_count += 1

        return (diff_count / total) <= threshold
