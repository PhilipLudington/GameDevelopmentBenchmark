#!/bin/bash
source ~/.secrets
cd ~/repos/GameDevelopmentBenchmark
source venv/bin/activate
python scripts/run_benchmark.py -m anthropic:claude-sonnet-4-5
