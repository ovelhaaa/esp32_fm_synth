# ESP32 Headless FM Synth (BLE MIDI + Moog Filter)

A fully headless 4-Operator FM Synthesizer with a Virtual Analog Moog Ladder Filter, controlled entirely via Bluetooth LE MIDI. Designed for the Wemos Lolin32 Lite.

## Features

- **Sound Engine**: 4-Operator FM Synthesis (based on `ML_SynthTools`).
- **Post-Processing**: 4-Pole Moog Ladder Filter with self-oscillation resonance.
- **Effects**: Delay and Reverb.
- **Connectivity**: BLE MIDI (Wireless control via Web/DAW).
- **Controls**:
    - Full ADSR Envelopes
    - FM Ratio & Modulation Index
    - Filter Cutoff (Exponential Mapping) & Resonance
    - Presets System
- **Presets**:
    1.  Init (Basic Sine)
    2.  Electric Piano
    3.  Fat Bass
    4.  Bell / Chime
    5.  Acid Lead

## Quick Start

1.  Flash the firmware (see [BUILD_GUIDE.md](BUILD_GUIDE.md)).
2.  Open `index.html` in a Web Bluetooth supported browser (Chrome, Edge).
3.  Click "Connect (Web Bluetooth)" and select `ESP32_FM_Synth`.
4.  Play notes and tweak the sliders!

## MIDI Mapping

| Parameter       | MIDI CC | Range      | Description                 |
|:----------------|:-------:|:----------:|:----------------------------|
| **FM Ratio**    | 16      | 0-127      | Carrier/Modulator Ratio     |
| **FM Index**    | 17      | 0-127      | Modulation Depth            |
| **Attack**      | 18      | 0-127      | Envelope Attack Time        |
| **Decay**       | 19      | 0-127      | Envelope Decay Time         |
| **Sustain**     | 20      | 0-127      | Envelope Sustain Level      |
| **Release**     | 21      | 0-127      | Envelope Release Time       |
| **Feedback**    | 22      | 0-127      | Operator Feedback           |
| **Resonance**   | 71      | 0-127      | Filter Resonance (0 - 4.2)  |
| **Cutoff**      | 74      | 0-127      | Filter Frequency (Exp.)     |
| **Preset Load** | PC      | 0-4        | Program Change Message      |

## Architecture

- **Core 0**: Handles BLE Stack and MIDI parsing.
- **Core 1**: Dedicated high-priority Audio Task (FM Gen -> Filter -> FX -> I2S).
- **Libraries**:
    - [ML_SynthTools](https://github.com/marcel-licence/ML_SynthTools)
    - [ESP32-BLE-MIDI](https://github.com/max22/ESP32-BLE-MIDI)

## Build & Flash

Please refer to [BUILD_GUIDE.md](BUILD_GUIDE.md) for detailed PlatformIO instructions.
