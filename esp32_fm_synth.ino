/*
 * Fully Headless FM Synthesizer with BLE MIDI and Moog Ladder Filter
 * Refactored by Jules
 */

#include "config.h"
#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>

#ifdef BLE_MIDI_ENABLED
#include <BLEMidi.h>
#endif

/* Moog Filter */
#include "MoogLadder.h"

/* requires the ML_SynthTools library: https://github.com/marcel-licence/ML_SynthTools */
#include <caps_info.h>
#include <ml_arp.h>
#include <ml_delay.h>
#include <ml_midi_ctrl.h>
#include <ml_reverb.h>
#include <ml_types.h>
#include <ml_fm.h>

#define ML_SYNTH_INLINE_DECLARATION
#include <ml_inline.h>
#undef ML_SYNTH_INLINE_DECLARATION

/* Audio Objects */
static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];
static float m1_sample[SAMPLE_BUFFER_SIZE];

/* Moog Filter Instance */
MoogLadder moogFilter;

/* Global Parameters */
static float master_output_gain = 0.5f;

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

        moogFilter.setFrequency(cutoff);
    }
    // Moog Filter Resonance (CC 71)
    else if (controller == 71) {
        // Map 0-127 to 0.0 - 4.2 (allowing self-oscillation k >= 4.0)
        float res = normalized * 4.2f;
        moogFilter.setResonance(res);
    }
    // FM Parameters (Based on z_config.ino mappings)
    else {
        // Map MIDI CCs to FM Synth functions found in z_config.ino
        switch (controller) {
            case 16: // CC 16: Ratio (ChangeParam 0)
                 FmSynth_ChangeParam(0, normalized);
                 break;
            case 17: // CC 17: Mod Index (ChangeParam 1) - guessing usually param 1 or Feedback?
                 // z_config.ino shows:
                 // S1 (CC 0x11? no, slider) -> FmSynth_ChangeParam, 0
                 // S2 -> FmSynth_ChangeParam, 1
                 // R4 -> FmSynth_Feedback, 3

                 // Let's use FmSynth_ChangeParam for general FM mods if that controls ratio/index
                 // Or we can try to use specific functions if they exist.
                 // z_config.ino uses 'FmSynth_ChangeParam' for 'S1' to 'S4'.
                 FmSynth_ChangeParam(1, normalized);
                 break;
            case 18: // CC 18: Attack
                 // z_config.ino: FmSynth_Attack, 4
                 FmSynth_Attack(4, normalized);
                 break;
            case 19: // CC 19: Decay
                 // z_config.ino: FmSynth_Decay1, 5
                 FmSynth_Decay1(5, normalized);
                 break;
            case 20: // CC 20: Sustain
                 // z_config.ino: FmSynth_DecayL, 6 (Sustain Level?)
                 FmSynth_DecayL(6, normalized);
                 break;
            case 21: // CC 21: Release
                 // z_config.ino: FmSynth_Release, 8
                 FmSynth_Release(8, normalized);
                 break;
            case 22: // Feedback
                 // z_config.ino: FmSynth_Feedback, 3
                 FmSynth_Feedback(3, normalized);
                 break;
        }
    }
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
    moogFilter.setSampleRate(SAMPLE_RATE);
    moogFilter.setFrequency(1000.0f); // Default cutoff
    moogFilter.setResonance(0.0f);

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
        m1_sample[i] = moogFilter.process(m1_sample[i]);
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
