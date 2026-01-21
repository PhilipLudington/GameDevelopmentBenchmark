"""Metrics collection for game performance analysis."""

import time
from collections import deque
from dataclasses import dataclass, field
from typing import Any

import psutil


@dataclass
class FrameMetrics:
    """Metrics for a single frame."""

    frame_number: int
    frame_time_ms: float
    fps: float
    memory_mb: float
    cpu_percent: float
    timestamp: float = field(default_factory=time.time)


@dataclass
class GameMetrics:
    """Aggregated metrics for a game session."""

    total_frames: int
    elapsed_time_s: float
    avg_fps: float
    min_fps: float
    max_fps: float
    avg_frame_time_ms: float
    p95_frame_time_ms: float
    p99_frame_time_ms: float
    avg_memory_mb: float
    peak_memory_mb: float
    avg_cpu_percent: float
    peak_cpu_percent: float
    events: list[dict[str, Any]] = field(default_factory=list)


class MetricsCollector:
    """Collects performance metrics during game execution.

    Usage:
        collector = MetricsCollector()
        collector.start()

        for frame in game_loop:
            # ... game logic ...
            collector.record_frame()

        metrics = collector.stop()
        print(f"Average FPS: {metrics.avg_fps}")
    """

    def __init__(self, window_size: int = 60):
        """Initialize the metrics collector.

        Args:
            window_size: Number of frames to use for rolling averages
        """
        self.window_size = window_size
        self._frame_times: deque[float] = deque(maxlen=window_size)
        self._all_frame_times: list[float] = []
        self._memory_samples: list[float] = []
        self._cpu_samples: list[float] = []
        self._events: list[dict[str, Any]] = []
        self._start_time: float | None = None
        self._last_frame_time: float | None = None
        self._frame_count = 0
        self._process: psutil.Process | None = None
        self._running = False

    def start(self) -> None:
        """Start collecting metrics."""
        self._start_time = time.time()
        self._last_frame_time = self._start_time
        self._frame_count = 0
        self._frame_times.clear()
        self._all_frame_times = []
        self._memory_samples = []
        self._cpu_samples = []
        self._events = []
        self._process = psutil.Process()
        self._running = True

    def record_frame(self) -> FrameMetrics:
        """Record metrics for the current frame.

        Returns:
            FrameMetrics for this frame
        """
        if not self._running:
            raise RuntimeError("Metrics collection not started. Call start() first.")

        current_time = time.time()
        frame_time = (current_time - self._last_frame_time) * 1000  # ms
        self._last_frame_time = current_time

        self._frame_count += 1
        self._frame_times.append(frame_time)
        self._all_frame_times.append(frame_time)

        # Sample system metrics (not every frame to reduce overhead)
        memory_mb = 0.0
        cpu_percent = 0.0
        if self._frame_count % 10 == 0 and self._process:
            try:
                memory_mb = self._process.memory_info().rss / (1024 * 1024)
                cpu_percent = self._process.cpu_percent()
                self._memory_samples.append(memory_mb)
                self._cpu_samples.append(cpu_percent)
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass

        # Calculate instantaneous FPS
        fps = 1000.0 / frame_time if frame_time > 0 else 0

        return FrameMetrics(
            frame_number=self._frame_count,
            frame_time_ms=frame_time,
            fps=fps,
            memory_mb=memory_mb,
            cpu_percent=cpu_percent,
            timestamp=current_time,
        )

    def record_event(self, event_type: str, data: dict[str, Any] | None = None) -> None:
        """Record a game event.

        Args:
            event_type: Type of event (e.g., "score", "collision", "game_over")
            data: Additional event data
        """
        self._events.append({
            "type": event_type,
            "frame": self._frame_count,
            "timestamp": time.time(),
            "data": data or {},
        })

    def get_current_fps(self) -> float:
        """Get the current FPS based on recent frames.

        Returns:
            Current FPS (rolling average)
        """
        if not self._frame_times:
            return 0.0
        avg_frame_time = sum(self._frame_times) / len(self._frame_times)
        return 1000.0 / avg_frame_time if avg_frame_time > 0 else 0

    def stop(self) -> GameMetrics:
        """Stop collecting and return aggregated metrics.

        Returns:
            GameMetrics with aggregated statistics
        """
        if not self._running:
            raise RuntimeError("Metrics collection not started.")

        self._running = False
        end_time = time.time()
        elapsed = end_time - self._start_time if self._start_time else 0

        # Calculate frame time statistics
        if self._all_frame_times:
            sorted_times = sorted(self._all_frame_times)
            avg_frame_time = sum(sorted_times) / len(sorted_times)
            p95_idx = int(len(sorted_times) * 0.95)
            p99_idx = int(len(sorted_times) * 0.99)
            p95_frame_time = sorted_times[min(p95_idx, len(sorted_times) - 1)]
            p99_frame_time = sorted_times[min(p99_idx, len(sorted_times) - 1)]

            fps_values = [1000.0 / t for t in sorted_times if t > 0]
            avg_fps = sum(fps_values) / len(fps_values) if fps_values else 0
            min_fps = min(fps_values) if fps_values else 0
            max_fps = max(fps_values) if fps_values else 0
        else:
            avg_frame_time = 0
            p95_frame_time = 0
            p99_frame_time = 0
            avg_fps = 0
            min_fps = 0
            max_fps = 0

        # Calculate memory statistics
        avg_memory = sum(self._memory_samples) / len(self._memory_samples) if self._memory_samples else 0
        peak_memory = max(self._memory_samples) if self._memory_samples else 0

        # Calculate CPU statistics
        avg_cpu = sum(self._cpu_samples) / len(self._cpu_samples) if self._cpu_samples else 0
        peak_cpu = max(self._cpu_samples) if self._cpu_samples else 0

        return GameMetrics(
            total_frames=self._frame_count,
            elapsed_time_s=elapsed,
            avg_fps=avg_fps,
            min_fps=min_fps,
            max_fps=max_fps,
            avg_frame_time_ms=avg_frame_time,
            p95_frame_time_ms=p95_frame_time,
            p99_frame_time_ms=p99_frame_time,
            avg_memory_mb=avg_memory,
            peak_memory_mb=peak_memory,
            avg_cpu_percent=avg_cpu,
            peak_cpu_percent=peak_cpu,
            events=self._events,
        )

    def __enter__(self) -> "MetricsCollector":
        """Context manager entry."""
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        """Context manager exit."""
        if self._running:
            self.stop()


def measure_performance(
    update_fn: callable,
    num_frames: int = 300,
    warmup_frames: int = 60,
) -> GameMetrics:
    """Measure performance of a game update function.

    Args:
        update_fn: Function to call each frame
        num_frames: Number of frames to measure
        warmup_frames: Frames to skip before measuring

    Returns:
        GameMetrics with performance statistics
    """
    collector = MetricsCollector()

    # Warmup
    for _ in range(warmup_frames):
        update_fn()

    # Measure
    collector.start()
    for _ in range(num_frames):
        update_fn()
        collector.record_frame()

    return collector.stop()
