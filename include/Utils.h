// Helper functions and useful macros

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <SD.h>

// --- Song storage ---
extern uint16_t volume;

#define MAX_SONGS_PER_PLAYLIST 10
#define MAX_PLAYLISTS 10
#define TRACKS_DIR "/"

struct track_t {
    String artist;
    String song;
    String filepath;
};

struct playlist_t {
    String name;
    uint16_t num_songs;
    track_t* tracks[MAX_SONGS_PER_PLAYLIST];
};

uint16_t load_tracklist_from_sd(String root_filepath, playlist_t** tracklist);
void print_tracklist(playlist_t* tracklist[], int num_playlists);

// Touch sensors
#define NUM_SENSORS 13
#define STORE_MS 70 // How long to store wheel state
#define HOLD_MS 300 // How long to register button press
#define SWIPE_MS 300 // How long to register button swipe

#define BTN_DEBOUNCE 50

// --- Pins ---
#define SD_CS_PIN 39
#define MOSI_PIN 35
#define SCK_PIN 36
#define MISO_PIN 37

#define I2S_DIN_PIN 40 // DIN on the amp module, but an output on our ESP32
#define I2S_BCLK_PIN 41
#define I2S_LRC_PIN 42

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
#define DMA_BUF_COUNT 64
#define DMA_BUF_LEN 1024
#define WAV_CACHE_SZ 4096

#define NUM_FRAMES_TO_SEND 1024

// --- User Interface ---

#define SS_NUM_DISPLAYED_TRACKS 5


#endif // UTILS_H