# Timetable Chain — Web Deployment Guide

## 1. Architecture Assessment & Technical Rationale

### WebAssembly (Emscripten) vs. Native C++17 Micro-Service

| Evaluation Dimension | WebAssembly via Emscripten | Native C++17 Micro-Service (Selected) |
| :--- | :--- | :--- |
| **Algorithmic Fidelity** | High (compiled C++) | **100% Native** (Direct linking to `libcore.a`) |
| **Toolchain Dependencies** | Requires `emcc`, `emsdk`, python glue | **Zero external dependencies** (Standard Clang/GCC + CMake) |
| **Cold Start & Compilation** | Slow Emscripten pipeline, large `.wasm` binaries | **Instant C++ compilation** (under 4 seconds) |
| **PoW Mining & Multi-Threading**| Single-threaded browser thread / WebWorkers with SharedArrayBuffer CORS isolation hurdles | **Full native POSIX multi-threading** and lightning-fast SHA-256 execution |
| **Deployment Simplicity** | Fragile headers for Cross-Origin Isolation | **Single self-contained binary** serving both JSON API & React SPA |
| **Maintainability** | Dual build systems for Qt GUI vs Web | **Single root CMakeLists.txt** builds desktop CLI, Qt GUI, and Web Server |

**Decision**: The native C++17 micro-service architecture was selected. It directly links the pure C++17 `core/` static library, exposing a high-performance REST API via single-header `httplib.h` and `json.hpp`. In production, the single binary `./timetable-server` serves both the backend API and the optimized React SPA bundle from `web/dist`.

---

## 2. Directory Layout

```
timetable-chain/
├── CMakeLists.txt            # Root build configuration for core, cli, gui, tests, server
├── Makefile                  # 1-command build, test, and run automation
├── Dockerfile                # Multi-stage production container build
├── render.yaml               # 1-click cloud deployment configuration
├── fly.toml                  # Fly.io deployment manifest
├── core/                     # Pure C++17 algorithmic engine (untouched)
│   ├── include/core/         # Headers (generator, constraint_checker, blockchain, etc.)
│   └── src/                  # Implementation (backtracking CSP, sha256, block mining)
├── server/                   # Native C++17 HTTP REST API Server
│   ├── include/              # httplib.h, json.hpp
│   └── src/server_main.cpp   # API routes & static file server
├── web/                      # Modern React 19 + TypeScript + Vite + Tailwind CSS frontend
│   ├── src/
│   │   ├── components/       # Navbar, Sidebar, ScheduleGrid, Modals
│   │   ├── pages/            # Generator, ManualBuilder, Blockchain, CurriculumData
│   │   ├── context/          # AppContext state management & localStorage persistence
│   │   └── services/         # REST client binding to C++ backend
│   └── dist/                 # Production static bundle
├── data/                     # Sample curriculum datasets (sample.txt, sample_full_week.txt)
└── tests/                    # CTest test suites (test_main.cpp, gui_test.cpp)
```

---

## 3. Local Execution Guide

### Option A: Using the Automated Makefile (Recommended)

```bash
# 1. Build core, server, and React frontend:
make build

# 2. Run all unit and integration tests:
make test

# 3. Launch the unified server on http://localhost:8080:
make run-server
```

Open `http://localhost:8080` in any web browser.

### Option B: Development Mode with Hot Reloading

```bash
# Terminal 1: Launch C++ HTTP Server on port 8080
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target timetable-server
./build/timetable-server 8080

# Terminal 2: Launch Vite Dev Server on port 3000 (proxies /api to 8080)
cd web
npm install
npm run dev
```

Open `http://localhost:3000`.

---

## 4. Public Cloud Deployment

### 1. Render (Free Tier / Container Service)
1. Push this repository to GitHub.
2. Sign in to [Render](https://render.com/).
3. Click **New +** -> **Web Service**.
4. Select your repository. Render will automatically detect `render.yaml` or `Dockerfile`.
5. Environment: **Docker**. Port: **8080**.
6. Click **Deploy Web Service**.

### 2. Fly.io
```bash
fly launch
fly deploy
```

### 3. Docker (Any Linux VPS / AWS EC2 / DigitalOcean Droplet)
```bash
# Build image
docker build -t timetable-chain:latest .

# Run container on port 80
docker run -d -p 80:8080 --name timetable-chain --restart always timetable-chain:latest
```

---

## 5. API Endpoints Reference

All computations run exclusively in native C++:

| Endpoint | Method | Description |
| :--- | :--- | :--- |
| `/api/health` | `GET` | Server status and C++ engine diagnostics |
| `/api/sample` | `GET` | Returns standard sample curriculum dataset (`sample.txt`) |
| `/api/sample-full-week` | `GET` | Returns 6-day academic week dataset (`sample_full_week.txt`) |
| `/api/dataset/parse` | `POST` | Parses raw curriculum text and validates referential integrity |
| `/api/dataset/validate` | `POST` | Comprehensive syntax & semantic diagnostics with line numbers |
| `/api/generate` | `POST` | Runs C++ CSP generator (`Plain`, `MRV`, or `Best`) |
| `/api/compare` | `POST` | Runs benchmark across all 3 strategies back-to-back |
| `/api/manual/can-place` | `POST` | Real-time evaluation of all 4 hard constraints for a single slot |
| `/api/manual/validate` | `POST` | Evaluates complete schedule validity & calculates penalty score |
| `/api/blockchain/mine` | `POST` | Solves Proof-of-Work (difficulty 3) and computes SHA-256 block |
| `/api/blockchain/verify` | `POST` | Runs full 3-layer verification across all blocks in the ledger |
| `/api/blockchain/tamper` | `POST` | Simulates Careless or Clever Attacker alterations |
| `/api/blockchain/save` | `POST` | Exports ledger in `TIMETABLE_CHAIN_V1` file format |
| `/api/blockchain/load` | `POST` | Parses and reconstructs ledger from file format |
