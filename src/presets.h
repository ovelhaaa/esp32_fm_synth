#ifndef PRESETS_H
#define PRESETS_H

#include <Arduino.h>

struct SynthPreset {
    const char* name;
    // FM Parameters (normalized 0.0 - 1.0 or specific values)
    float ratio;       // 0.0 - 10.0+
    float index;       // 0.0 - 10.0+
    float attack;      // 0.0 - 1.0
    float decay;       // 0.0 - 1.0
    float sustain;     // 0.0 - 1.0 (Level)
    float release;     // 0.0 - 1.0

    // Filter Parameters
    float cutoff;      // Hz
    float resonance;   // 0.0 - 4.2
};

// Preset Definitions
void loadPreset(uint8_t id);
const char* getPresetName(uint8_t id);
uint8_t getPresetCount();

#endif
