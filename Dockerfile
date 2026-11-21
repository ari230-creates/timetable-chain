# ==============================================================================
# Multi-Stage Dockerfile for TIMETABLE CHAIN Web Application
# ==============================================================================

# Stage 1: Build React Frontend
FROM node:22-alpine AS frontend-builder
WORKDIR /app/web
COPY web/package*.json ./
RUN npm install
COPY web/ ./
RUN npm run build

# Stage 2: Build C++17 Core & Native HTTP Server
FROM gcc:13-bookworm AS backend-builder
RUN apt-get update && apt-get install -y cmake make libpthread-stubs0-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY CMakeLists.txt ./
COPY core/ ./core/
COPY server/ ./server/
COPY data/ ./data/
COPY tests/ ./tests/
COPY cli/ ./cli/

# Compile core and timetable-server
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target timetable-server -j$(nproc)

# Stage 3: Minimal Production Runtime
FROM debian:bookworm-slim AS runtime
WORKDIR /app
RUN apt-get update && apt-get install -y ca-certificates curl && rm -rf /var/lib/apt/lists/*

# Copy binaries, static web assets, and default datasets
COPY --from=backend-builder /app/build/timetable-server ./
COPY --from=frontend-builder /app/web/dist ./web/dist
COPY --from=backend-builder /app/data ./data

# Environment configuration
ENV PORT=8080
EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
  CMD curl -f http://localhost:${PORT}/api/health || exit 1

ENTRYPOINT ["./timetable-server"]
