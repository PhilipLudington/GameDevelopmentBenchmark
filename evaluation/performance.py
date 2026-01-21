"""Performance benchmark runner for game evaluation."""

import os
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import psutil

# Set up headless mode
os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
os.environ.setdefault("SDL_AUDIODRIVER", "dummy")


@dataclass
class PerformanceResult:
    """Result of a performance benchmark."""

    success: bool
    avg_fps: float
    min_fps: float
    max_fps: float
    p95_frame_time_ms: float
    avg_memory_mb: float
    peak_memory_mb: float
    avg_cpu_percent: float
    meets_fps_target: bool
    meets_memory_target: bool
    frames_measured: int
    elapsed_time_s: float
    error: str | None = None


class PerformanceBenchmark:
    """Runs performance benchmarks on games.

    Measures:
    - Frame rate (FPS)
    - Frame time consistency
    - Memory usage
    - CPU usage
    """

    def __init__(
        self,
        target_fps: float = 60.0,
        max_memory_mb: float = 256.0,
        warmup_frames: int = 60,
        measure_frames: int = 300,
    ):
        """Initialize the benchmark.

        Args:
            target_fps: Target frames per second
            max_memory_mb: Maximum allowed memory in MB
            warmup_frames: Frames to skip before measuring
            measure_frames: Frames to measure
        """
        self.target_fps = target_fps
        self.max_memory_mb = max_memory_mb
        self.warmup_frames = warmup_frames
        self.measure_frames = measure_frames

    def run(self, game_module: Any) -> PerformanceResult:
        """Run performance benchmark on a game.

        Args:
            game_module: Module containing Game class

        Returns:
            PerformanceResult with benchmark data
        """
        Game = getattr(game_module, "Game", None)
        GameState = getattr(game_module, "GameState", None)

        if Game is None:
            return PerformanceResult(
                success=False,
                avg_fps=0,
                min_fps=0,
                max_fps=0,
                p95_frame_time_ms=0,
                avg_memory_mb=0,
                peak_memory_mb=0,
                avg_cpu_percent=0,
                meets_fps_target=False,
                meets_memory_target=False,
                frames_measured=0,
                elapsed_time_s=0,
                error="Game class not found",
            )

        try:
            # Create game
            game = Game(headless=True)
            if GameState:
                game.set_state(GameState.PLAYING)

            process = psutil.Process()
            frame_times: list[float] = []
            memory_samples: list[float] = []
            cpu_samples: list[float] = []

            # Warmup
            for _ in range(self.warmup_frames):
                game.step()

            # Measure
            start_time = time.time()
            last_frame_time = start_time

            for i in range(self.measure_frames):
                game.step()

                # Measure frame time
                current_time = time.time()
                frame_time = (current_time - last_frame_time) * 1000  # ms
                frame_times.append(frame_time)
                last_frame_time = current_time

                # Sample memory/CPU every 10 frames
                if i % 10 == 0:
                    try:
                        memory_mb = process.memory_info().rss / (1024 * 1024)
                        cpu_percent = process.cpu_percent()
                        memory_samples.append(memory_mb)
                        cpu_samples.append(cpu_percent)
                    except (psutil.NoSuchProcess, psutil.AccessDenied):
                        pass

            elapsed = time.time() - start_time

            # Calculate statistics
            frame_times_sorted = sorted(frame_times)
            avg_frame_time = sum(frame_times) / len(frame_times)
            p95_idx = int(len(frame_times_sorted) * 0.95)
            p95_frame_time = frame_times_sorted[min(p95_idx, len(frame_times_sorted) - 1)]

            fps_values = [1000.0 / t for t in frame_times if t > 0]
            avg_fps = sum(fps_values) / len(fps_values) if fps_values else 0
            min_fps = min(fps_values) if fps_values else 0
            max_fps = max(fps_values) if fps_values else 0

            avg_memory = sum(memory_samples) / len(memory_samples) if memory_samples else 0
            peak_memory = max(memory_samples) if memory_samples else 0
            avg_cpu = sum(cpu_samples) / len(cpu_samples) if cpu_samples else 0

            # Check targets
            meets_fps = avg_fps >= self.target_fps * 0.95  # 5% tolerance
            meets_memory = peak_memory <= self.max_memory_mb

            return PerformanceResult(
                success=True,
                avg_fps=avg_fps,
                min_fps=min_fps,
                max_fps=max_fps,
                p95_frame_time_ms=p95_frame_time,
                avg_memory_mb=avg_memory,
                peak_memory_mb=peak_memory,
                avg_cpu_percent=avg_cpu,
                meets_fps_target=meets_fps,
                meets_memory_target=meets_memory,
                frames_measured=self.measure_frames,
                elapsed_time_s=elapsed,
            )

        except Exception as e:
            return PerformanceResult(
                success=False,
                avg_fps=0,
                min_fps=0,
                max_fps=0,
                p95_frame_time_ms=0,
                avg_memory_mb=0,
                peak_memory_mb=0,
                avg_cpu_percent=0,
                meets_fps_target=False,
                meets_memory_target=False,
                frames_measured=0,
                elapsed_time_s=0,
                error=str(e),
            )

    def run_from_path(self, game_path: Path) -> PerformanceResult:
        """Run benchmark on a game from file path.

        Args:
            game_path: Path to the game's main.py

        Returns:
            PerformanceResult with benchmark data
        """
        import importlib.util

        if not game_path.exists():
            return PerformanceResult(
                success=False,
                avg_fps=0,
                min_fps=0,
                max_fps=0,
                p95_frame_time_ms=0,
                avg_memory_mb=0,
                peak_memory_mb=0,
                avg_cpu_percent=0,
                meets_fps_target=False,
                meets_memory_target=False,
                frames_measured=0,
                elapsed_time_s=0,
                error=f"Game not found: {game_path}",
            )

        try:
            # Load module
            spec = importlib.util.spec_from_file_location("game_module", game_path)
            if spec is None or spec.loader is None:
                raise ImportError(f"Cannot load module from {game_path}")

            module = importlib.util.module_from_spec(spec)
            sys.modules["game_module"] = module
            spec.loader.exec_module(module)

            return self.run(module)

        except Exception as e:
            return PerformanceResult(
                success=False,
                avg_fps=0,
                min_fps=0,
                max_fps=0,
                p95_frame_time_ms=0,
                avg_memory_mb=0,
                peak_memory_mb=0,
                avg_cpu_percent=0,
                meets_fps_target=False,
                meets_memory_target=False,
                frames_measured=0,
                elapsed_time_s=0,
                error=str(e),
            )


def compare_performance(
    original: PerformanceResult,
    modified: PerformanceResult,
) -> dict[str, Any]:
    """Compare two performance results.

    Args:
        original: Performance before modification
        modified: Performance after modification

    Returns:
        Dictionary with comparison data
    """
    if not original.success or not modified.success:
        return {
            "valid": False,
            "error": original.error or modified.error,
        }

    fps_change = (modified.avg_fps - original.avg_fps) / original.avg_fps if original.avg_fps > 0 else 0
    memory_change = (modified.peak_memory_mb - original.peak_memory_mb) / original.peak_memory_mb if original.peak_memory_mb > 0 else 0

    return {
        "valid": True,
        "fps_change_percent": fps_change * 100,
        "memory_change_percent": memory_change * 100,
        "improved_fps": fps_change > 0.05,  # 5% improvement threshold
        "improved_memory": memory_change < -0.05,  # 5% reduction threshold
        "original": {
            "avg_fps": original.avg_fps,
            "peak_memory_mb": original.peak_memory_mb,
        },
        "modified": {
            "avg_fps": modified.avg_fps,
            "peak_memory_mb": modified.peak_memory_mb,
        },
    }
