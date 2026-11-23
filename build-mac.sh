#!/bin/bash
set -e
if ! command -v brew >/dev/null 2>&1; then
  echo "Install Homebrew first: https://brew.sh"
  exit 1
fi
if ! brew list raylib >/dev/null 2>&1; then brew install raylib; fi
cc -std=c11 -Wall -Wextra -Wpedantic -O2 gui.c -o tracker-gui $(pkg-config --cflags --libs raylib) -lm
echo "Built ./tracker-gui"
./tracker-gui
