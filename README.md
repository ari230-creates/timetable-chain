# Timetable Chain

> **Intelligent Timetable Generation & Blockchain Ledger System**  
> *C++ Project*  
> Implemented in modern C++17 with Qt 6 Widgets and a custom SHA-256 cryptographic engine.

---

## 1. Overview & Architecture

**Timetable Chain** is a desktop application combining:
1. **Intelligent Constraint Satisfaction Solver**: An algorithmic timetable generator using recursive backtracking with symmetry breaking and Minimum Remaining Values (MRV) heuristic.
2. **Pedagogical Quality Scorer**: A penalty-based scoring function optimizing for daily subject dispersion and faculty rest intervals.
3. **Cryptographic Blockchain Ledger**: An immutable append-only ledger featuring SHA-256 proof-of-work (PoW) mining and a comprehensive **3-layer verification system** (Integrity, Cryptographic Linkage, and Semantic Rule Consistency).

### System Architecture
The application strictly decouples the domain core from the GUI layer:
- **`core/`**: Pure C++17 static library (`libcore.a`) with zero Qt dependencies.
- **`gui/`**: Qt 6 Widgets desktop application (`timetable-gui`) with global QSS stylesheets, light/dark themes, responsive worker threads, custom QPainter visualizations, and PDF/CSV export.
- **`cli/`**: Lightweight terminal CLI (`timetable`) for automated headless benchmarking and verification.
- **`tests/`**: Unit test and GUI integration test suites registered with CTest.

```
timetable-chain/
├── CMakeLists.txt              # Root build configuration (CMake >= 3.16)
├── README.md                   # Project documentation & viva defense guide
├── data/
│   └── sample.txt              # Standard curriculum dataset
├── core/                       # Pure C++17 Backend Library
│   ├── include/core/
│   │   ├── block.hpp           # Block struct & PoW mining
│   │   ├── blockchain.hpp      # Append-only ledger & 3-layer verification
│   │   ├── constraint_checker.hpp # Hard constraint enforcement rules
│   │   ├── dataset.hpp         # Curricular domain entities & parser
│   │   ├── entry.hpp           # Timetable entry binding (subj, fac, slot)
│   │   ├── faculty.hpp         # Teacher qualifications & availability
│   │   ├── generator.hpp       # CSP Backtracking solver (Plain, MRV, Best)
│   │   ├── scorer.hpp          # Schedule quality penalty function
│   │   ├── sha256.hpp          # Custom SHA-256 implementation (FIPS PUB 180-4)
│   │   ├── subject.hpp         # Academic course definition
│   │   ├── timeslot.hpp        # Schedule slot representation
│   │   └── timetable.hpp       # Schedule collection & serialization
│   └── src/
│       ├── block.cpp
│       ├── blockchain.cpp
│       ├── constraint_checker.cpp
│       ├── dataset.cpp
│       ├── generator.cpp
│       ├── scorer.cpp
│       ├── sha256.cpp
│       └── timetable.cpp
├── gui/                        # Qt 6 Widgets GUI Application
│   ├── include/
│   │   ├── block_detail_dialog.hpp # Modal timetable inspector for blocks
│   │   ├── blockchain_page.hpp     # Interactive blockchain ledger UI
│   │   ├── comparison_dialog.hpp   # Strategy benchmark & QPainter bar chart
│   │   ├── data_page.hpp           # Curriculum dataset inspection tables
│   │   ├── generator_page.hpp      # CSP solver controls & animated stat cards
│   │   ├── mainwindow.hpp          # App shell, navigation sidebar & top bar
│   │   ├── manual_builder_page.hpp # Interactive drag/click schedule editor
│   │   ├── schedule_grid.hpp       # Weekly schedule table with custom chips
│   │   ├── slot_dialog.hpp         # Real-time constraint validation dialog
│   │   ├── tamper_dialog.hpp       # Malicious attack simulation dialog
│   │   ├── theme.hpp               # Light/Dark palettes, shadow & icon generator
│   │   └── toast.hpp               # Transient animated overlay notifications
│   ├── src/
│   │   ├── block_detail_dialog.cpp
│   │   ├── blockchain_page.cpp
│   │   ├── comparison_dialog.cpp
│   │   ├── data_page.cpp
│   │   ├── generator_page.cpp
│   │   ├── main.cpp
│   │   ├── mainwindow.cpp
│   │   ├── manual_builder_page.cpp
│   │   ├── schedule_grid.cpp
│   │   ├── slot_dialog.cpp
│   │   ├── tamper_dialog.cpp
│   │   ├── theme.cpp
│   │   └── toast.cpp
│   └── resources/
│       ├── styles.qrc
│       └── styles/
│           ├── dark.qss
│           └── light.qss
├── cli/
│   └── main.cpp                # Terminal demo & automated test harness
└── tests/
    ├── test_main.cpp           # Plain assert core test suite
    └── gui_test.cpp            # Headless Qt integration test suite
```

---

## 2. Compilation & Build Instructions

### Prerequisites
- C++17 compatible compiler (Apple Clang, GCC 9+, or MSVC 2019+)
- CMake >= 3.16
- Qt 6 (Widgets module)

#### macOS Installation via Homebrew:
```bash
brew install qt cmake
```

### Build Steps:
```bash
# From the project root:
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
make -j

# Run the automated CTest suite:
ctest --output-on-failure

# Launch the desktop GUI:
./timetable-gui

# Launch the terminal CLI verifier:
./timetable
```

---

## 3. Two-Minute Interactive Demo Script

Follow these steps for a live demonstration:

1. **Launch the Application**:
   - Run `./build/timetable-gui`.
   - The curriculum dataset `data/sample.txt` is automatically loaded upon startup.
   - Note the left sidebar, the clean card-based UI, and the active theme.

2. **Generate a Schedule (Page 1: Generator)**:
   - Select the **MRV (Most Constrained First)** strategy from the dropdown.
   - Click **Generate**.
   - Notice the animated stat counters: **8 Placements**, **0 Backtracks**, **Penalty Score: 16**.
   - Inspect the interactive schedule grid populated with color-coded chips for each subject.
   - Click **Compare Strategies**: Observe the custom `QPainter` bar chart contrasting Plain (320 placements), MRV (8 placements), and Best (73 placements).

3. **Mine & Commit Two Schedules to the Blockchain**:
   - In Page 1, switch strategy to **Best** and click **Generate & Mine Block**.
   - The worker thread executes asynchronously, mines a block satisfying the 3-zero Proof-of-Work difficulty (`000...`), and appends it to the chain ledger.
   - Switch to **Page 2: Manual Builder**. Click on an unassigned slot or populate a valid schedule, then click **Commit to Blockchain**. Enter note: *"Department Review Schedule"*.

4. **Verify the Blockchain (Page 3: Blockchain)**:
   - Navigate to the **Blockchain** tab.
   - Observe the vertical sequence of block cards interconnected by chain-link graphics.
   - Each card displays index, timestamp, nonce, and shortened cryptographic hash chips. Hover over any hash chip to inspect the full 64-character hex string; click to copy.
   - Click **Verify Blockchain**:
     - The verification engine runs all 3 layers.
     - A green banner confirms: `✓ Chain intact and every timetable valid`.
     - All block status badges turn **VERIFIED** (green).

5. **Simulate Malicious Attacks (Tamper Demo)**:
   - Click **Tamper Demo**.
   - **Attack 1 (Careless Attacker)**:
     - Select Block 1, leave *"Also re-mine the block"* **unchecked**, and click **Tamper Block**.
     - Click **Verify Blockchain**.
     - **Result**: Block 1 turns **RED** with error `Block 1: Hash mismatch (stored != recomputed)`.
   - **Attack 2 (Clever Attacker)**:
     - Open **Tamper Demo** again. Select Block 1, check *"Also re-mine the block (clever attacker)"*, and click **Tamper Block**.
     - Notice that Block 1's proof-of-work has been recomputed.
     - Click **Verify Blockchain**.
     - **Result**: Block 2 turns **RED** with error `Block 2: Broken link (prevHash != previous block hash)`. The cryptographic chain link is broken!

---

## 4. Algorithmic Theory & Viva Defense

### A. Constraint Satisfaction Problem (CSP) Modeling
The timetable scheduling problem is formulated as a finite-domain Constraint Satisfaction Problem:
- **Variables ($X$)**: Each required period for every subject represents an atomic task $t_k \in T$. For $n$ total required periods, $|T| = n$.
- **Domains ($D$)**: The Cartesian product of all faculty qualified to teach the subject and all weekly time slots: $D(t_k) \subseteq \text{Faculty} \times \text{Slots}$.
- **Hard Constraints ($C$)**:
  1. *Qualification*: Faculty $f$ must belong to `subject.canTeach`.
  2. *Availability*: Faculty $f$ must not be marked unavailable in slot $s$.
  3. *Slot Exclusivity*: No two entries may share slot $s$ ($\sum \text{Entry}(*, *, s) \le 1$).
  4. *Faculty Double-Booking*: No faculty may teach in multiple classrooms simultaneously.
  5. *Completeness*: Every subject $S_j$ must receive exactly `periodsPerWeek` allocations.

### B. Pruning & Symmetry Breaking
- **Early Pruning**: Before expanding child states, `ConstraintChecker::canPlace` evaluates all four local hard constraints in $O(1)$ time, eliminating infeasible subtrees immediately.
- **Symmetry Breaking**: Within consecutive tasks of the identical subject, if slot $s_a$ was chosen for the first instance, the second instance is constrained to choose slot $s_b > s_a$. This eliminates $k!$ symmetric permutations of identical subject periods, drastically reducing search space.

### C. Search Strategies
- **Plain Strategy**: Tasks are ordered linearly in subject data-file sequence.
  - Placements: 320, Backtracks: 312, Score: 9.
  - Explores deeply before encountering bottleneck constraints.
- **Minimum Remaining Values (MRV)**: Heuristic prioritizing the most constrained variable first ($|D(t_k)|$ ascending).
  - Subject 3 (Databases) can only be taught by Faculty 2, who has only 3 available slots. By assigning Subject 3 first, the search avoids dead ends.
  - Placements: 8, Backtracks: 0, Score: 16.
  - Finds a valid solution with zero backtracks!
- **Best Optimization Strategy**: Combines MRV variable ordering with bounded branch exploration (up to 50 valid solutions), selecting the schedule minimizing the pedagogical penalty function.
  - Placements: 73, Backtracks: 65, Score: 7.

### D. Penalty Scoring Function
$$\text{Penalty} = 5 \times N_{\text{same-day subject pairs}} + 2 \times N_{\text{back-to-back faculty pairs}}$$
- Encourages balanced distribution of subjects across days rather than clustering.
- Protects instructors from continuous teaching fatigue without breaks.

---

## 5. Blockchain Mechanics & 3-Layer Verification

### Block Structure & Hashing
Each block encapsulates:
$$\text{Payload} = \text{index} \parallel \text{timestamp} \parallel \text{note} \parallel \text{data} \parallel \text{prevHash} \parallel \text{nonce}$$
$$\text{Hash} = \text{SHA-256}(\text{Payload})$$
- Serialized timetable data sorts entries deterministically by `(slotId, subjectId, facultyId)`: identical schedules produce byte-for-byte identical hash payloads.

### Proof-of-Work (PoW) Mining
- Mining increments `nonce` starting from 0 until `hash` begins with `DIFFICULTY = 3` leading zeroes:
  $$\text{hash} = \text{"000..."}$$
- Demonstrates consensus work without external computational bottlenecks.

### The 3-Layer Verification System
Every verification pass runs across all blocks in the ledger:
1. **Layer 1: Structural Integrity**
   - Verifies `block[i].index == i`.
   - Recomputes SHA-256 hash from block contents and verifies equality with stored `hash`.
   - Checks that stored `hash` satisfies the proof-of-work target prefix.
2. **Layer 2: Cryptographic Linkage**
   - Verifies genesis block prevHash is `"0"`.
   - For every block $i > 0$, verifies `block[i].prevHash == block[i-1].hash`.
3. **Layer 3: Semantic Timetable Consistency**
   - Deserializes block data into a `Timetable` and evaluates `ConstraintChecker::isValid()`.
   - Ensures an attacker cannot insert an invalid timetable into the blockchain ledger even if they re-mine the proof-of-work.

---

## 6. Acceptance Checklist Status

| Requirement | Status | Verification Method |
|:---|:---:|:---|
| Builds with zero warnings under `-Wall -Wextra -Wpedantic` | **PASS** | CMake build logs verify 0 compiler warnings across all targets |
| All tests pass with exact numbers from Section 8 | **PASS** | Plain: 320/312 (score 9); MRV: 8/0 (score 16); Best: 73/65 (score 7) |
| GUI launches, auto-loads sample data, grid fills after Generate | **PASS** | Auto-loads `data/sample.txt`; grid renders color-coded chips |
| UI stays responsive during Best mode and mining | **PASS** | Asynchronous execution via dedicated `QThread` worker |
| Commit twice, Verify $\rightarrow$ all cards green | **PASS** | Verified via CTest test suite and GUI ledger |
| Tamper (careless) $\rightarrow$ hash-mismatch error on that block | **PASS** | Detected in Layer 1 verification (`Hash mismatch`) |
| Tamper (clever) $\rightarrow$ broken-link error on next block | **PASS** | Detected in Layer 2 verification (`Broken link`) |
| Manual Builder rejects "Dr. Rao teaches Databases" | **PASS** | Real-time rejection inline: `Faculty Dr. Rao cannot teach Databases` |
| Dark and light themes both look correct with high contrast | **PASS** | Dynamic QSS stylesheets with dedicated high-contrast palettes |
| Bad data file / bad chain file shows friendly message, no crash | **PASS** | Graceful error reporting with line numbers and Toast alerts |

---

## 7. License
Academic project developed for Design and Analysis of Algorithms (DAA). Free for academic and educational use.
