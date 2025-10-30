// Helper functions and common macros

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <SD.h>
#include <TFT_eSPI.h>

#define SD_CS_PIN 16
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23
#define SS1_CLK_PIN 12
#define SS1_DT_PIN 3
#define SS1_SW_PIN 14
#define I2S_DIN_PIN 22 // DIN on the amp module, but an output on our ESP32
#define I2S_BCLK_PIN 26
#define I2S_LRC_PIN 25
#define I2S_SAMPLE_RATE 44100 // All .wav files have been given a 44100 sample rate upon creation

#define AUDIO_DIR "/"
#define AUDIO_WINDOW_RES 1000 // In bytes
#define AUDIO_WINDOW_SZ 320 // In samples
#define VOLUME 0.3
#define I2S_BUF_FRAMES 3000
#define I2S_STEREO_MULT 2

#define WAV_CACHE_SZ 12000 // In bytes

// --- User Interface ---
#define BACKGROUND_COLOR 0x0000
#define PRIMARY_COLOR 0xFFFF
#define ACCENT_COLOR_1 0x7DFF
#define TEXT_PADDING 3
#define TEXT_SIZE 1
#define LINE_HEIGHT TEXT_SIZE * 10 
#define BORDER_RADIUS 4

#define BAD_FILE 0
#define OK 1

uint16_t load_tracklist_from_file(File directory, String* tracklist, uint32_t max_songs);
void display_tracklist(TFT_eSPI* tft, String* tracklist, uint16_t y, uint16_t num_tracks_to_display, uint16_t num_tracks, uint16_t hovered_track_ind, uint16_t channel_1_ind);

#endif // UTILS_H