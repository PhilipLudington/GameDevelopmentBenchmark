#!/bin/bash
source ~/.secrets
cd ~/repos/GameDevelopmentBenchmark
source venv/bin/activate

OUTPUT_DIR="results/runs/20260213_195519"
LOG="$OUTPUT_DIR/remaining_run.log"

# All 141 missing task IDs
MISSING=(
asteroids-feature-001 asteroids-feature-002 asteroids-feature-003 asteroids-feature-004
asteroids-feature-005 asteroids-feature-006 asteroids-feature-007 asteroids-feature-008
asteroids-feature-009 asteroids-feature-010 asteroids-feature-011 asteroids-mini-001
asteroids-mini-002 asteroids-optimization-001 asteroids-optimization-002
breakout-feature-001 breakout-feature-002 breakout-feature-003 breakout-feature-004
breakout-feature-005 breakout-feature-006 breakout-feature-007 breakout-feature-008
breakout-feature-009 breakout-feature-010 breakout-feature-011 breakout-feature-012
breakout-feature-013 breakout-feature-014 breakout-mini-001 breakout-mini-002
breakout-mini-003 breakout-optimization-001 breakout-optimization-002
platformer-feature-001 platformer-feature-002 platformer-feature-003 platformer-feature-004
platformer-feature-005 platformer-feature-006 platformer-feature-007 platformer-feature-008
platformer-feature-009 platformer-feature-010 platformer-feature-011 platformer-feature-012
platformer-mini-001 platformer-mini-002 platformer-optimization-001
pong-006 pong-007 pong-008 pong-009 pong-010
pong-feature-011 pong-feature-012 pong-feature-013 pong-feature-014
pong-feature-015 pong-feature-016 pong-feature-017
pong-mini-002 pong-mini-003 pong-mini-004 pong-optimization-001
quake-001 quake-002 quake-003 quake-004
quake-feat-001 quake-feat-002 quake-feat-003
quake-opt-001 quake-opt-002 quake-opt-003
snake-feature-001 snake-feature-002 snake-feature-003 snake-feature-004
snake-feature-005 snake-feature-006 snake-feature-007 snake-feature-008
snake-feature-009 snake-feature-010 snake-feature-011 snake-feature-012
snake-feature-013 snake-feature-014 snake-mini-001 snake-mini-002
snake-mini-003 snake-mini-004 snake-optimization-001 snake-optimization-002
space_invaders-feature-001 space_invaders-feature-002 space_invaders-feature-003
space_invaders-feature-004 space_invaders-feature-005 space_invaders-feature-006
space_invaders-feature-007 space_invaders-feature-008 space_invaders-feature-009
space_invaders-feature-010 space_invaders-feature-011 space_invaders-feature-012
space_invaders-mini-001 space_invaders-mini-002 space_invaders-mini-003
space_invaders-optimization-001 space_invaders-optimization-002 space_invaders-optimization-003
tetris-feature-001 tetris-feature-002 tetris-feature-003 tetris-feature-004
tetris-feature-005 tetris-feature-006 tetris-feature-007 tetris-feature-008
tetris-feature-009 tetris-feature-010 tetris-mini-001 tetris-mini-002
tetris-optimization-001
tower_defense-feature-001 tower_defense-feature-002 tower_defense-feature-003
tower_defense-feature-004 tower_defense-feature-005 tower_defense-feature-006
tower_defense-feature-007 tower_defense-feature-008 tower_defense-feature-009
tower_defense-feature-010 tower_defense-feature-011 tower_defense-feature-012
tower_defense-mini-001 tower_defense-mini-002 tower_defense-optimization-001
)

MODEL="anthropic:claude-sonnet-4-5"
TOTAL=${#MISSING[@]}
PASSED=0
FAILED=0

echo "Resuming benchmark: $TOTAL remaining tasks" | tee "$LOG"
echo "Output: $OUTPUT_DIR" | tee -a "$LOG"
echo "Started: $(date)" | tee -a "$LOG"
echo "==========================================" | tee -a "$LOG"

# Find task dir by task ID
find_task_dir() {
    local task_id="$1"
    find tasks -name "task.json" -exec sh -c '
        id=$(jq -r .id "$1" 2>/dev/null)
        if [ "$id" = "'"$task_id"'" ]; then
            dirname "$1"
            exit 0
        fi
    ' _ {} \; 2>/dev/null | head -1
}

for i in "${!MISSING[@]}"; do
    idx=$((i + 1))
    task_id="${MISSING[$i]}"
    task_dir=$(find_task_dir "$task_id")
    
    if [ -z "$task_dir" ]; then
        echo "  [$idx/$TOTAL] $task_id... SKIP (not found)" | tee -a "$LOG"
        continue
    fi

    # Run single task via python
    result=$(python -c "
import json, sys
from evaluation.runner import EvaluationRunner
from harness.sandbox import SandboxConfig
from models.base import create_model
from pathlib import Path

model = create_model('$MODEL')
sandbox_config = SandboxConfig(timeout=120, headless=True)
runner = EvaluationRunner(
    task_dir=Path('$task_dir'),
    model=model,
    sandbox_config=sandbox_config,
    verbose=False,
)
result = runner.run()
result_dict = {
    'task_id': result.task_id,
    'model_name': result.model_name,
    'model_version': result.metadata.get('model_version', result.model_name),
    'benchmark_version': '0.1.0',
    'success': result.success,
    'score': result.score,
    'elapsed_time': result.elapsed_time,
    'error': result.error,
    'usage': result.usage,
    'metadata': result.metadata,
}
outfile = Path('$OUTPUT_DIR') / f'{result.task_id}_anthropic_claude-sonnet-4-5.json'
with open(outfile, 'w') as f:
    json.dump(result_dict, f, indent=2)
print('PASSED' if result.success else 'FAILED')
" 2>/dev/null)

    if [ "$result" = "PASSED" ]; then
        echo "  [$idx/$TOTAL] $task_id... PASSED" | tee -a "$LOG"
        PASSED=$((PASSED + 1))
    else
        echo "  [$idx/$TOTAL] $task_id... FAILED" | tee -a "$LOG"
        FAILED=$((FAILED + 1))
    fi
done

echo "==========================================" | tee -a "$LOG"
echo "Done: $PASSED passed, $FAILED failed out of $TOTAL" | tee -a "$LOG"
echo "Finished: $(date)" | tee -a "$LOG"
