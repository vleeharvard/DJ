#include <Arduino.h>
#include <TFT_eSPI.h>
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
#define SD_CS_PIN 16
#define TRACKS_DIR "/"

// --- Variables ---
String tracklist[MAX_SONGS];
uint16_t track_count = 0;
TFT_eSPI tft = TFT_eSPI();
size_t bytes_written;
uint16_t selected_track_ind = 0;

Channel* channel_1 = nullptr; 
Rotary_Encoder song_select_1 = Rotary_Encoder(SS1_CLK_PIN, SS1_DT_PIN, SS1_SW_PIN);

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

    // Setup display
    Serial.printf("Initializing display\n");
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextSize(TEXT_SIZE);

    // Setup I2S for audio
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DIN_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM_0, I2S_SAMPLE_RATE);

    // Setup rotary encoder
    song_select_1.init();

    // Creating channels
    channel_1 = new Channel(tracklist, &tft, track_count, ACCENT_COLOR_1);
    channel_1->select_track(0);
    channel_1->playing = true;

    // Display tracklist
    display_tracklist(
        &tft,
        tracklist,
        200, 
        5, 
        track_count, 
        selected_track_ind,
        channel_1->track_index
    );
}

void loop() {
    int8_t ss1_delta = song_select_1.read_encoder();
    bool selection_changed = false;

    if (ss1_delta != 0) {
        selected_track_ind += ss1_delta;
        if (selected_track_ind >= track_count) { selected_track_ind = track_count - 1; }
        if (selected_track_ind < 0) { selected_track_ind = 0; }

        selection_changed = true;
    }

    if (song_select_1.read_button()) {
        channel_1->select_track(selected_track_ind);
        channel_1->playing = true;
        selection_changed = true;
    }

    if (selection_changed) {
        display_tracklist(
            &tft,
            tracklist,
            200, 
            5, 
            track_count, 
            selected_track_ind,
            channel_1->track_index
        );
    }

    channel_1->update();
    // channel_1->display_waveform(0, 0, 200);
}