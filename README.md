# Digital Asset Tracker — 100% C GUI

This edition contains no Python or other application-language code. The desktop interface is written in C and uses **raylib** for windowing, drawing and keyboard/mouse input. Portfolio data and transaction history are stored locally in `tracker_data.csv`.

## macOS setup and run

1. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```
2. Install Homebrew if it is not already installed: https://brew.sh
3. Open Terminal in this folder and run:
   ```bash
   chmod +x build-mac.sh
   ./build-mac.sh
   ```
   The script installs raylib with Homebrew if needed, compiles the C GUI, and launches it.

To build without launching:
```bash
brew install raylib
cc -std=c11 -Wall -Wextra -Wpedantic -O2 gui.c -o tracker-gui $(pkg-config --cflags --libs raylib) -lm
```
Then run `./tracker-gui`.

## Windows (MSYS2 UCRT64)
Install raylib in the UCRT64 shell (`pacman -S mingw-w64-ucrt-x86_64-raylib pkgconf`) and compile:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 gui.c -o tracker-gui.exe $(pkg-config --cflags --libs raylib) -lm
```

## Features
- Native desktop GUI: overview, portfolio, market and transaction-history screens
- Buy/sell simulation with quantity validation
- Holdings, average cost and portfolio valuation
- Local CSV persistence across app restarts
- INR-denominated sample prices for eight cryptocurrencies

## Notes
This is a local portfolio simulator, not an exchange, wallet or broker. Prices are built-in illustrative sample values and are not live market quotes. Buy/sell operations only update the local portfolio. Data is saved to `tracker_data.csv` in the working directory.
