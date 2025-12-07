#ifndef MOOG_LADDER_H
#define MOOG_LADDER_H

#include <math.h>

class MoogLadder {
public:
    MoogLadder() {
        reset();
        setSampleRate(44100.0f);
    }

    void setSampleRate(float sampleRate) {
        sampleRate_ = sampleRate;
    }

    void reset() {
        for (int i = 0; i < 4; i++) {
            state[i] = 0.0f;
        }
        cutoff_ = 1000.0f;
        resonance_ = 0.0f;
        calculateCoefficients();
    }

    // Set Cutoff Frequency in Hz
    void setFrequency(float cutoff) {
        cutoff_ = cutoff;
        calculateCoefficients();
    }

    // Set Resonance
    // Standard Moog resonance k goes from 0 to 4 for self oscillation.
    // We expect the input here to be the internal coefficient value.
    // The main code maps 0-127 CC to 0.0-4.2 for this method.
    void setResonance(float resonance) {
        resonance_ = resonance;
        calculateCoefficients();
    }

    float process(float input) {
        // Moog Ladder Filter Implementation
        // Based on the difference equation model (Stilson/Moog)

        // Negative Feedback loop
        // The feedback is usually 4.0 * resonance * last_stage_output
        // But here we use resonance_ directly as the feedback amount k (0..4)

        float val = input - resonance_ * state[3];

        // 4 one-pole stages with tanh saturation for non-linearity

        // Stage 1
        state[0] += p_ * (tanh(val) - state[0]);

        // Stage 2
        state[1] += p_ * (state[0] - state[1]);

        // Stage 3
        state[2] += p_ * (state[1] - state[2]);

        // Stage 4
        state[3] += p_ * (state[2] - state[3]);

        return state[3];
    }

private:
    float state[4];
    float cutoff_;
    float resonance_;
    float sampleRate_;
    float p_; // Cutoff coefficient

    void calculateCoefficients() {
        // Cutoff coefficient p calculation
        // p = 1 - exp(-2 * PI * cutoff / sampleRate)
        // Works well for simple 1-pole sections

        float fc = cutoff_ / sampleRate_;

        // Clamp fc to avoid instability
        if (fc > 0.45f) fc = 0.45f;

        p_ = 1.0f - expf(-2.0f * M_PI * fc);
    }
};

#endif
