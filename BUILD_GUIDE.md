# ESP32 Headless FM Synth Build Guide

This guide covers how to build, flash, and wire the Headless FM Synth project using PlatformIO.

## Prerequisites

- **Hardware**:
  - Wemos Lolin32 Lite (or compatible ESP32 board)
  - PCM5102 I2S DAC (Standard)
  - Wires and Power Supply (USB)
- **Software**:
  - [VSCode](https://code.visualstudio.com/)
  - [PlatformIO Extension for VSCode](https://platformio.org/install/ide?install=vscode)
  - Git

## Build Instructions

1.  **Clone the Repository**:
    ```bash
    git clone <repository_url>
    cd esp32_fm_synth
    ```

2.  **Open in PlatformIO**:
    - Open VSCode.
    - Click "Open Folder" and select the project directory.
    - Wait for PlatformIO to initialize and download dependencies (`ML_SynthTools`, `ESP32-BLE-MIDI`).

3.  **Configure `platformio.ini` (Optional)**:
    - The default environment is `[env:lolin32_lite]`.
    - If you are using a different board, change `board = lolin32_lite` to your specific model (e.g., `esp32dev`).

4.  **Build**:
    - Click the Checkmark icon (✓) in the bottom PlatformIO toolbar.
    - Or run in terminal: `pio run`

5.  **Upload**:
    - Connect your ESP32 via USB.
    - Click the Arrow icon (→) in the bottom toolbar.
    - Or run in terminal: `pio run --target upload`

## Wiring Guide (I2S PCM5102)

| PCM5102 Pin | ESP32 Pin (Lolin32 Lite) | Description       |
|:-----------:|:------------------------:|:-----------------:|
| VCC         | 5V / VBUS                | Power (5V pref)   |
| GND         | GND                      | Ground            |
| BCK         | GPIO 26                  | Bit Clock         |
| DIN         | GPIO 25                  | Data In           |
| LCK         | GPIO 22                  | Word Clock (LRCK) |
| SCK         | GND                      | System Clock      |

*Note: Verify pin definitions in `include/ml_boards.h` or `include/config.h` if you customize the board definition.*

## Usage

1.  **Power On**: The device will start and advertise as `ESP32_FM_Synth` via BLE.
2.  **Connect**:
    - Use the provided `index.html` PWA (via Chrome/Edge).
    - Or pair via OS Bluetooth settings and use any DAW/MIDI Controller.
3.  **Control**:
    - **CC 74**: Filter Cutoff
    - **CC 71**: Filter Resonance
    - **CC 16-22**: FM Parameters (Ratio, Index, ADSR, Feedback)
    - **Program Change**: Switch Presets (0: Init, 1: E.Piano, 2: Bass, 3: Bell, 4: Lead)
