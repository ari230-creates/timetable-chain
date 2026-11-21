# ==============================================================================
# TIMETABLE CHAIN — Unified Build & Run Makefile
# ==============================================================================

.PHONY: all build-core build-server build-web build test run-server run-gui dev-web clean docker-build docker-run

all: build test

build-core:
	cmake -B build-web -DCMAKE_BUILD_TYPE=Release
	cmake --build build-web --target core timetable tests -j4

build-server:
	cmake -B build-web -DCMAKE_BUILD_TYPE=Release
	cmake --build build-web --target timetable-server -j4

build-web:
	cd web && npm install && npm run build

build: build-core build-web build-server

test:
	cmake --build build-web --target tests gui_test -j4
	cd build-web && ctest --output-on-failure

run-server: build-server
	./build-web/timetable-server 8080

dev-web:
	cd web && npm run dev

run-gui:
	cmake --build build-web --target timetable-gui -j4
	./build-web/timetable-gui

docker-build:
	docker build -t timetable-chain:latest .

docker-run:
	docker run -p 8080:8080 timetable-chain:latest

clean:
	rm -rf build-web build-test web/dist web/node_modules
