// Helper functions and useful macros

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <SD.h>

// --- Pins ---
#define SD_CS_PIN 39
#define MOSI_PIN 35
#define SCK_PIN 36
#define MISO_PIN 37

#define I2S_DIN_PIN 42 // DIN on the amp module, but an output on our ESP32
#define I2S_BCLK_PIN 41
#define I2S_LRC_PIN 40

#define OLED_SCL 18 
#define OLED_SDA 17
#define OLED_RESET U8X8_PIN_NONE

#define BATLVL 16

// --- Audio ---
typedef struct
{
    int16_t left;
    int16_t right;
} Frame_t;
#define AUDIO_DIR "/"
#define VOLUME (50)
#define DMA_BUF_COUNT 64
#define DMA_BUF_LEN 1024
#define WAV_CACHE_SZ 4096

#define NUM_FRAMES_TO_SEND 512

// --- User Interface ---


uint16_t load_tracklist_from_file(File directory, String* tracklist, uint32_t max_songs);
// void display_tracklist(TFT_eSPI* tft, String* tracklist, uint16_t y, uint16_t num_tracks_to_display, uint16_t num_tracks, uint16_t hovered_track_ind, uint16_t channel_1_ind);

#endif // UTILS_H