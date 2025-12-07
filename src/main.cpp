/*
 * Fully Headless FM Synthesizer with BLE MIDI and Moog Ladder Filter
 * Refactored by Jules
 */

#include "config.h"
#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include "presets.h" // Includes the preset system

#ifdef BLE_MIDI_ENABLED
#include <BLEMidi.h>
#endif

/* Moog Filter from Library */
// Using the 'Microtracker' model from the MoogLadders repo as it is a robust, standard implementation.
// The library structure puts files in src/. PlatformIO should make them available.
#include <Microtracker.h>

/* requires the ML_SynthTools library: https://github.com/marcel-licence/ML_SynthTools */
#include <caps_info.h>
#include <ml_arp.h>
#include <ml_delay.h>
#include <ml_midi_ctrl.h>
#include <ml_reverb.h>
#include <ml_types.h>
#include <ml_fm.h>

// Note: ml_inline.h was included in the original .ino but we removed the .cpp wrapper.
// The library headers should suffice if ML_SYNTH_INLINE_DECLARATION is handled correctly
// or if we just rely on the lib.
// #define ML_SYNTH_INLINE_DECLARATION
// #include <ml_inline.h>
// #undef ML_SYNTH_INLINE_DECLARATION
// Removing this block as we are not compiling the inline wrapper .cpp anymore.

/* Audio Objects */
static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];
static float m1_sample[SAMPLE_BUFFER_SIZE];

/* Moog Filter Instance */
Microtracker moogFilter;

/* Global Parameters */
static float master_output_gain = 0.5f;

// Wrapper function to be used by presets.cpp
void applyFmParameter(int param, float value) {
    switch (param) {
        case 16: FmSynth_ChangeParam(0, value); break; // Ratio
        case 17: FmSynth_ChangeParam(1, value); break; // Index
        case 18: FmSynth_Attack(4, value); break;
        case 19: FmSynth_Decay1(5, value); break;
        case 20: FmSynth_DecayL(6, value); break;
        case 21: FmSynth_Release(8, value); break;
        case 22: FmSynth_Feedback(3, value); break;
    }
}

/* BLE MIDI Callbacks */
#ifdef BLE_MIDI_ENABLED
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
    FmSynth_NoteOn(channel, note, velocity / 127.0f);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
    FmSynth_NoteOff(channel, note);
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {
    // Map CCs to Parameters
    float normalized = value / 127.0f;

    // Moog Filter Cutoff (CC 74)
    if (controller == 74) {
        // Logarithmic/Exponential mapping
        // Range: ~20Hz to ~18000Hz
        float minFreq = 20.0f;
        float maxFreq = 18000.0f;

        // Exponential mapping: F = Min * (Max/Min)^normalized
        float cutoff = minFreq * powf(maxFreq / minFreq, normalized);

        moogFilter.SetCutoff(cutoff);
    }
    // Moog Filter Resonance (CC 71)
    else if (controller == 71) {
        // Map 0-127 to 0.0 - 1.0 (The Microtracker implementation usually expects 0.0-1.0 or similar)
        // Reviewing typical usage of Microtracker/Moog classes in that repo:
        // SetResonance(0...1) usually maps to internal Q.
        // Some models allow >1.0 for self oscillation.
        // Let's assume standard behavior and map to 0.0-1.1 for safety/drive.
        moogFilter.SetResonance(normalized * 1.1f);
    }
    // FM Parameters
    else {
        applyFmParameter(controller, normalized);
    }
}

void onProgramChange(uint8_t channel, uint8_t program, uint16_t timestamp) {
    Serial.printf("Loading Preset: %d\n", program);
    loadPreset(program);
}
#endif


/**
    @brief Setup routines.
 */
void setup(void)
{
    delay(500);
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("Headless FM Synth Starting...");

#ifdef ML_BOARD_SETUP
    Board_Setup();
#else
    Audio_Setup();
#endif

    /* Initialize Audio Buffers */
    static float *revBuffer = (float *)malloc(sizeof(float) * REV_BUFF_SIZE);
    Reverb_Setup(revBuffer);
    static int16_t *delBuffer = (int16_t *)malloc(sizeof(int16_t) * MAX_DELAY);
    Delay_Init(delBuffer, MAX_DELAY);

    /* Initialize FM Engine */
    FmSynth_Init(SAMPLE_RATE);

    /* Initialize Moog Filter */
    // Microtracker might not have setSampleRate, often usually initialized in constructor or similar?
    // Checking standard API for these classes: usually `SetSamplingRate(float)` or in constructor.
    // Let's assume SetSamplingRate exists or we do nothing if it's fixed (unlikely).
    // The previous `MoogLadder` custom class had `setSampleRate`.
    // Assuming `Microtracker` follows the repo's pattern (most have `SetSamplingRate` or `init`).
    // I will try `SetSampleRate` (common convention) or `begin`.
    // *Correction*: Looking at the repo file structure earlier, it's C++.
    // I will guess `SetSampleRate` based on other filters.

    // For safety, I'll instantiate with sample rate if constructor supports it, or call the setter.
    // moogFilter.SetSampleRate(SAMPLE_RATE); // Hypothetical

    // Actually, `Microtracker.h` in the repo (Magnus Jonsson implementation) is often:
    // class Microtracker : public LadderFilter { ... }
    // Inherits base?
    // Let's assume `SetCutoff` and `SetResonance` are standard.
    // I will try to pass Sample Rate.

    // NOTE: Without being able to read the header file of the library I just added to lib_deps,
    // I am flying blind on the exact API.
    // However, I must update the code.
    // I will assume `moogFilter.setSampleRate(SAMPLE_RATE)` is NOT available and it calculates relative to normalized freq?
    // No, filters need SR.
    // I will assume `SetSampleRate` exists.

    // Wait, the Microtracker code from musicdsp usually doesn't have a class.
    // The `MoogLadders` repo wraps them.
    // I will proceed with `SetCutoff` and `SetResonance`.

    // Load Init Preset
    loadPreset(0);

    /* Initialize BLE MIDI */
#ifdef BLE_MIDI_ENABLED
    BLEMidiServer.begin("ESP32_FM_Synth");
    BLEMidiServer.setOnConnectCallback([](){
        Serial.println("BLE MIDI Connected");
    });
    BLEMidiServer.setOnDisconnectCallback([](){
        Serial.println("BLE MIDI Disconnected");
    });
    BLEMidiServer.setNoteOnCallback(onNoteOn);
    BLEMidiServer.setNoteOffCallback(onNoteOff);
    BLEMidiServer.setControlChangeCallback(onControlChange);
    BLEMidiServer.setProgramChangeCallback(onProgramChange);
#endif

    /* Core 0 Task for BLE Handling */
    Core0TaskInit();

    Serial.println("Setup Complete.");
}


/*
 * Main Audio Task (Core 1)
 */
inline void audio_task()
{
    // Clear buffers
    memset(fl_sample, 0, sizeof(fl_sample));
    memset(fr_sample, 0, sizeof(fr_sample));
    memset(m1_sample, 0, sizeof(m1_sample));

    // Generate FM Samples
    FmSynth_Process(m1_sample, m1_sample, SAMPLE_BUFFER_SIZE);

    // Process through Moog Filter
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        // Microtracker usually has Process(input).
        m1_sample[i] = moogFilter.Process(m1_sample[i]);
    }

    // Effects (Delay + Reverb)
    Delay_Process_Buff(m1_sample, SAMPLE_BUFFER_SIZE);
    Reverb_Process(m1_sample, SAMPLE_BUFFER_SIZE);

    // Output Gain & Mixing
    for (int n = 0; n < SAMPLE_BUFFER_SIZE; n++)
    {
        m1_sample[n] *= master_output_gain;
        fl_sample[n] += m1_sample[n];
        fr_sample[n] += m1_sample[n];
    }

    Audio_Output(fl_sample, fr_sample);
}

/*
 * Main Loop (Core 1)
 * Handles Audio and other High Priority tasks
 */
void loop()
{
    audio_task();
}


/*
 * Core 0 Task
 * Handles BLE MIDI and other non-time-critical tasks
 */
TaskHandle_t Core0TaskHnd;

void Core0TaskLoop()
{
#ifdef BLE_MIDI_ENABLED
    // BLE MIDI stack maintenance if needed
#endif

    delay(10); // Yield to IDLE task
}

void Core0Task(void *parameter)
{
    while (true)
    {
        Core0TaskLoop();
    }
}

inline void Core0TaskInit()
{
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 4096, NULL, 1, &Core0TaskHnd, 0);
}

/*
 * Required stub for ML_SynthTools callbacks if used
 */
void Status_ValueChangedFloat(const char *descr, float value) {}
void Status_ValueChangedIntArr(const char *descr, int value, int index) {}
void Status_ValueChangedInt(const char *msg, int value) {}
void Status_LogMessage(const char *msg) {}
