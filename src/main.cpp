#include <Arduino.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>

// --- Custom libraries ---
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "Rotary_Encoder.h"

// --- Constants ---
#define MAX_SONGS 100
#define TRACKS_DIR "/"

// --- Variables ---
String tracklist[MAX_SONGS];
uint16_t track_count = 0;
size_t bytes_written;
uint16_t selected_track_ind = 0;

Channel* output;

void setup() {
    Serial.begin(57600);
    delay(1000);
    Serial.printf("\n\n\nStarting DJ Mixer...\n");

    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // Manually start

    // Read SD card and load tracks
    Serial.printf("Initializing SD card");

    while (!SD.begin(SD_CS_PIN)) {
        Serial.printf(".");
    }
    Serial.printf("\n");

    File tracks_dir = SD.open(TRACKS_DIR);
    track_count = load_tracklist_from_file(tracks_dir, tracklist, MAX_SONGS);

    Serial.printf("Found %d files:\n", track_count);
    for (int i = 0; i < track_count; i++) {
        Serial.printf("%d) %s\n", i, tracklist[i].c_str());
    }

    // Select I2S pins
    i2s_pin_config_t i2s_pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DIN_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // Creating channel
    Serial.println("Starting I2S Output");
    output = new Channel(tracklist, track_count);
    output->select_track(0);
    output->start(I2S_NUM_1, i2s_pin_config);
}

void loop() {
    // Nothing here since everything is handled by tasks
}