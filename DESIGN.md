# Game Development Benchmark for AI Models

A benchmark suite for evaluating AI model capabilities in game development tasks, inspired by SWE-Bench.

## Vision

Create a comprehensive, multi-engine benchmark that tests AI models on real-world game development challenges—from simple bug fixes to full game creation. The benchmark progressively increases in complexity across engines and task types.

## Design Principles

1. **Cost-conscious** - Maximize automated evaluation; minimize paid compute and human hours
2. **Progressive complexity** - Start simple, scale up only after validating approach
3. **Reproducible** - All tasks must have deterministic evaluation criteria
4. **Representative** - Cover the breadth of real game development work

---

## Evaluation Strategy

### Phase A: Automated Evaluation (Primary)

Low-cost, high-volume evaluation that gates all tasks before human review.

#### A1: Unit Tests
- Logic correctness (score calculation, inventory management, state transitions)
- Physics behavior within tolerances
- API contract compliance
- **Cost: Near zero** (CI/CD on free tiers)

#### A2: Integration Tests
- Systems interacting correctly (player + enemies + environment)
- Save/load round-trips
- Scene transitions
- **Cost: Near zero**

#### A3: Automated Gameplay
- Headless gameplay bots attempt to complete objectives
- Metrics: completion rate, score achieved, time to goal
- Regression detection via recorded playthroughs
- **Cost: Compute only** (can run on commodity hardware)

#### A4: Performance Benchmarks
- Frame time measurements
- Memory usage profiles
- Load time checks
- **Cost: Compute only**

### Phase B: Human Evaluation (Final Phase)

Reserved for tasks that pass all automated checks. Conducted in batches to amortize costs.

#### B1: Playability Assessment
- Can a human complete the intended gameplay loop?
- Are controls responsive and intuitive?
- Binary pass/fail with structured rubric

#### B2: Visual/Audio Quality
- Does output match specification/reference?
- Are there obvious glitches or artifacts?
- Likert scale (1-5)

#### B3: Subjective Quality (Optional)
- "Fun factor" - engagement rating
- Polish assessment
- Only for full game tasks, not bug fixes

#### Cost Mitigation for Human Evaluation
- **Batch processing** - Accumulate tasks, evaluate in scheduled rounds
- **Community volunteers** - Open source contributors, game jam communities
- **Student partnerships** - Game design programs seeking real-world exercises
- **Tiered approach** - Only escalate to paid evaluation for benchmark-critical tasks
- **Crowdsource platforms** - Use only when necessary (Prolific, etc.) with micro-tasks

---

## Engine Phases

Progressive rollout based on complexity and licensing costs.

### Phase 1: Pygame
**Timeline: Initial development**
**Cost: Free**

- Pure Python, extensive documentation
- Large corpus of open-source games and tutorials
- Easy to instrument and test headlessly
- No licensing concerns

**Task sources:**
- GitHub issues from pygame projects
- Classic game clones (Pong, Breakout, Snake variations)
- Pygame community challenges

### Phase 2: Godot
**Timeline: After Phase 1 validated**
**Cost: Free (MIT licensed)**

- Real engine complexity (scene trees, signals, physics)
- GDScript + C# + GDExtension options
- Excellent headless/CI support
- Active open-source game ecosystem

**Task sources:**
- Godot Asset Library projects
- GitHub issues from Godot games
- Official demo projects with injected bugs

### Phase 3: Agentite (Custom Engine on Klar)
**Timeline: After Phase 2**
**Cost: Development time only**

- Tests true generalization vs. training data memorization
- Novel APIs that models cannot have seen
- Full control over complexity curves
- Documents AI's ability to learn new frameworks

**Task sources:**
- Synthetic tasks designed for specific capabilities
- Porting challenges from Phases 1-2

### Phase 4: Extended Engine Support
**Timeline: Long-term / as resources allow**
**Cost: Varies (some require licenses)**

Priority order based on cost and accessibility:

| Engine | License Cost | Priority | Notes |
|--------|--------------|----------|-------|
| Raylib | Free | High | C, simple, good for low-level tasks |
| Bevy | Free | High | Rust ECS, modern architecture |
| MonoGame | Free | Medium | C#, XNA successor |
| Sokol | Free | Medium | Minimal C headers |
| LÖVE | Free | Medium | Lua, 2D focused |
| Mach/zig-gamedev | Free | Medium | Zig ecosystem |
| O3DE | Free | Lower | Complex, AWS-backed |
| GameMaker | Paid | Lower | Popular but costs $ |
| Unity | Free tier | Lower | Complex licensing |
| Unreal | Revenue share | Lowest | Heavy, licensing complexity |
| CryEngine | Revenue share | Lowest | Very complex |

**Strategy:** Exhaust free options before considering paid engines. Paid engines only if grant funding or sponsorship secured.

---

## Task Categories

### Category 1: Bug Fixes
Isolated issues with clear reproduction steps and test coverage.

**Examples:**
- Collision detection off-by-one errors
- Score not updating correctly
- Sprite rendering in wrong order
- Memory leak in particle system

**Evaluation:** Automated tests only

### Category 2: Feature Implementation
Add specified functionality to existing codebase.

**Examples:**
- Add double-jump mechanic
- Implement enemy patrol AI
- Add save/load system
- Create inventory UI

**Evaluation:** Automated tests + automated gameplay

### Category 3: Optimization
Improve performance to meet specified targets.

**Examples:**
- Reduce frame time from 20ms to 16ms
- Cut memory usage by 30%
- Optimize pathfinding for 100+ agents

**Evaluation:** Automated benchmarks

### Category 4: Full Mini-Games
Create complete playable games from specification.

**Examples:**
- "Create a tower defense game with 3 enemy types and 4 tower types"
- "Implement a match-3 puzzle game with 10 levels"

**Evaluation:** All automated phases + human evaluation

### Category 5: Porting
Translate working game from one engine to another.

**Examples:**
- Port Pygame game to Godot
- Port Godot game to Agentite
- Port 2D game to different framework

**Evaluation:** Automated equivalence testing + human comparison

---

## Difficulty Tiers

### Tier 1: Trivial
- Single-file changes
- Obvious fix from error message
- < 10 lines modified
- Example: Fix typo in variable name causing crash

### Tier 2: Easy
- Localized to one system
- Clear specification
- 10-50 lines modified
- Example: Fix collision bounds on player sprite

### Tier 3: Medium
- Multiple files involved
- Requires understanding system interactions
- 50-200 lines modified
- Example: Implement basic enemy AI with three states

### Tier 4: Hard
- Cross-cutting concerns
- Performance-sensitive
- Architectural decisions required
- 200-500 lines modified
- Example: Add networked multiplayer to single-player game

### Tier 5: Expert
- Full game implementation
- Novel problem-solving required
- System design from scratch
- 500+ lines
- Example: Create complete game from design document

---

## Data Collection

### Real Issues
- Scrape GitHub issues from open-source game projects
- Filter for: reproducible, has test or clear verification, resolved
- Tag by engine, category, and difficulty
- Obtain permission / respect licenses

### Synthetic Tasks
- Design tasks to cover capability gaps
- Inject bugs into working codebases
- Create specification documents for feature tasks
- Version control for reproducibility

### Quality Control
- Each task requires:
  - Clear problem statement
  - Reproduction steps (for bugs)
  - Acceptance criteria
  - Working baseline (for regression tasks)
  - At least one automated verification method

---

## Infrastructure (Cost-Conscious)

### Compute
- GitHub Actions free tier for CI
- Self-hosted runners on commodity hardware for heavy tasks
- No cloud GPU unless absolutely required

### Storage
- Git LFS for game assets (free tier limits apply)
- Compress aggressively
- Prefer procedural generation over large assets

### Hosting
- Static site for leaderboard (GitHub Pages = free)
- Results stored in repository as JSON/CSV

---

## Metrics & Reporting

### Primary Metrics
- **Pass rate** by engine, category, and tier
- **Automated test pass rate** (prerequisite for all else)
- **Gameplay completion rate** (bot success %)
- **Human approval rate** (for tasks reaching Phase B)

### Secondary Metrics
- Lines of code generated
- Compilation success rate
- Time/tokens to solution
- Cost per task (compute + human eval)

---

## Milestones

### M1: Proof of Concept
- 50 Pygame tasks (bug fixes + simple features)
- Automated evaluation pipeline working
- Baseline results from 2-3 models
- **Human evaluation: None yet**

### M2: Pygame Complete
- 200+ Pygame tasks across all categories
- Difficulty distribution validated
- Public leaderboard launched
- **Human evaluation: Pilot batch (20 tasks, volunteers)**

### M3: Godot Launch
- 100+ Godot tasks
- Cross-engine comparison possible
- **Human evaluation: Structured volunteer program**

### M4: Agentite Integration
- Custom engine operational
- Porting tasks available
- Generalization metrics defined
- **Human evaluation: Expanded to all full-game tasks**

### M5: Extended Engines
- 3+ additional engines
- Comprehensive coverage
- **Human evaluation: Full Phase B protocol**

---

## Phase B: Human Evaluation Protocol

*This phase runs independently after sufficient automated-passing submissions accumulate.*

### Cadence
- Monthly evaluation rounds (or when batch reaches 50+ tasks)
- Results published with 2-week delay for verification

### Evaluator Recruitment
1. **Volunteers** - Game jam communities, open source contributors
2. **Students** - Partnerships with game design programs
3. **Paid** - Only for critical/disputed evaluations

### Evaluation Interface
- Web-based review tool (simple, self-hosted)
- Randomized task assignment
- Blind to model identity
- Structured rubrics with examples

### Quality Assurance
- 10% of tasks evaluated by multiple reviewers
- Inter-rater reliability tracked
- Calibration tasks with known scores

### Cost Tracking
- Log hours per evaluation type
- Track $/task for paid evaluations
- Optimize rubrics to minimize time

---

## Budget Considerations

### Phase 1 Target: $0
- All free tools and platforms
- Volunteer evaluators only
- Self-hosted on existing hardware

### Phase 2 Target: < $500
- Potential costs: domain name, occasional cloud compute burst
- Small incentives for volunteer coordinators

### Scaling Costs (if successful)
- Grant applications for academic recognition
- Sponsorship from engine companies (Godot Foundation, etc.)
- Community fundraising only if benchmark proves valuable

### What We Won't Do
- Pay for proprietary engine licenses until justified
- Use expensive cloud GPU for tasks that don't need it
- Rush human evaluation before automated pipeline is solid

---

## Open Questions

1. **Licensing** - How to handle proprietary engine tasks if we expand?
2. **Asset creation** - Should tasks include art/audio, or focus on code only?
3. **Multiplayer** - How to test networked games affordably?
4. **Mobile/Console** - Platform-specific tasks feasible?
5. **Agentite scope** - What features does the custom engine need?

---

## References

- [SWE-Bench](https://www.swebench.com/) - Software engineering benchmark methodology
- [Pygame](https://www.pygame.org/) - Phase 1 engine
- [Godot](https://godotengine.org/) - Phase 2 engine
- [Klar](https://github.com/PhilipLudington/Klar) - Language for Agentite engine
- [Klar-Toolkit](https://github.com/PhilipLudington/Klar-Toolkit) - Klar development tools
- [MCP-Klar](https://github.com/PhilipLudington/MCP-Klar) - Klar MCP server
- [Agentite](https://github.com/PhilipLudington/Agentite) - Custom game engine for Phase 3

---

*Document version: 0.1 (Draft)*
*Last updated: 2026-01-21*
