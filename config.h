/*
 * Copyright (c) 2022 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __CDT_PARSER__
#include <cdt.h>
#endif

/*
 * BLE MIDI Configuration
 */
#define BLE_MIDI_ENABLED

/*
 * Core Audio Settings
 */
//#define I2S_USE_APLL
//#define OUTPUT_SAW_TEST

#define SERIAL_BAUDRATE 115200

// Board Selection
#define BOARD_ML_V1
//#define BOARD_ESP32_AUDIO_KIT_AC101
//#define BOARD_ESP32_AUDIO_KIT_ES8388
//#define BOARD_ESP32_DOIT

/* Audio Processing */
//#define AUDIO_PASS_THROUGH
#define SAMPLE_BUFFER_SIZE 48
#define MAX_DELAY   (SAMPLE_RATE/3) /* 1s -> @ 44100 samples */

/* MIDI Configuration */
// Disable Serial/USB MIDI if using BLE to avoid conflict/overhead,
// though BLEMidi can coexist. We focus on BLE.
//#define MIDI_RECV_FROM_SERIAL
//#define MIDI_VIA_USB_ENABLED

#define MIDI_USE_CONST_VELOCITY

/* Other modules */
//#define OLED_OSC_DISP_ENABLED
//#define PRESSURE_SENSOR_ENABLED
//#define ADC_TO_MIDI_ENABLED
#define ADC_TO_MIDI_LOOKUP_SIZE 8

//#define ARP_MODULE_ENABLED
#define MIDI_SYNC_MASTER

/*
 * include the board configuration
 */
#include <ml_boards.h>

#ifdef BOARD_ML_V1
#elif (defined BOARD_ESP32_AUDIO_KIT_AC101)
#elif (defined BOARD_ESP32_AUDIO_KIT_ES8388)
#elif (defined BOARD_ESP32_DOIT)
#else
#define MIDI_PORT2_ACTIVE
#define MIDI_RX2_PIN 16
#define MIDI_TX2_PIN 17
#endif

#ifdef ESP32_AUDIO_KIT
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT
#else
#define SAMPLE_RATE 48000
#define SAMPLE_SIZE_16BIT
#endif

#endif /* CONFIG_H_ */
