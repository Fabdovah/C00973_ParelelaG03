#!/usr/bin/env bash
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
mkdir -p tests/results
LOG="tests/results/execution.log"
: > "$LOG"
failed=0
run() { echo -e "\n### $*" | tee -a "$LOG"; "$@" 2>&1 | tee -a "$LOG"; status=${PIPESTATUS[0]}; if ((status != 0)); then failed=1; fi; }
run make -C src/ServidorProductos
run make -C src/Simulacion
run g++ -std=c++17 -Wall -Wextra -pthread tests/filesystem_test.cc src/ServidorProductos/filesystem.cc -o tests/results/filesystem_test
if [[ -x tests/results/filesystem_test ]]; then run tests/results/filesystem_test; fi
run python3 -m unittest discover -s tests -p 'test_*.py' -v
if ((failed)); then echo 'RESULTADO: FALLOS; revisar execution.log' | tee -a "$LOG"; exit 1; fi
echo 'RESULTADO: SUITE AUTOMATIZADA SIN FALLOS (ver skips)' | tee -a "$LOG"
