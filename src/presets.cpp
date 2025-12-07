#include "presets.h"
#include <Microtracker.h>

// External references
extern void FmSynth_SetRatio(float ratio);
extern void FmSynth_SetModIndex(float index);
extern void FmSynth_Attack(uint8_t op, float val);
extern void FmSynth_Decay1(uint8_t op, float val);
extern void FmSynth_DecayL(uint8_t op, float val);
extern void FmSynth_Release(uint8_t op, float val);

// We need access to the filter object in main.cpp
extern Microtracker moogFilter;

// wrapper functions to match main.cpp logic
extern void applyFmParameter(int param, float value);

/*
 * Preset Library
 */
const SynthPreset presets[] = {
    // Name, Ratio, Index, A, D, S, R, Cutoff, Res
    { "Init",       1.0f,  0.0f,  0.0f, 0.5f, 1.0f, 0.2f, 18000.0f, 0.0f },
    { "E. Piano",   1.0f,  3.5f,  0.0f, 0.6f, 0.0f, 0.4f, 8000.0f,  0.2f },
    { "Fat Bass",   0.5f,  1.5f,  0.0f, 0.3f, 0.8f, 0.2f, 400.0f,   0.5f },
    { "Bell",       2.4f,  2.0f,  0.0f, 0.8f, 0.0f, 2.0f, 12000.0f, 0.1f },
    { "Acid Lead",  1.0f,  5.0f,  0.1f, 0.4f, 0.6f, 0.3f, 1500.0f,  0.8f }
};

const uint8_t PRESET_COUNT = sizeof(presets) / sizeof(presets[0]);

uint8_t getPresetCount() {
    return PRESET_COUNT;
}

const char* getPresetName(uint8_t id) {
    if (id >= PRESET_COUNT) return "Unknown";
    return presets[id].name;
}

void loadPreset(uint8_t id) {
    if (id >= PRESET_COUNT) return;

    SynthPreset p = presets[id];

    // Apply Filter
    moogFilter.SetCutoff(p.cutoff);
    moogFilter.SetResonance(p.resonance);

    applyFmParameter(16, p.ratio / 10.0f); // Ratio
    applyFmParameter(17, p.index / 10.0f); // Index
    applyFmParameter(18, p.attack);
    applyFmParameter(19, p.decay);
    applyFmParameter(20, p.sustain);
    applyFmParameter(21, p.release);
}
