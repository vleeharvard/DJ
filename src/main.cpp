#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>

// --- Custom libraries ---
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"

// --- Constants ---
#define MAX_SONGS 100
#define SD_CS_PIN 16
#define TRACKS_DIR "/"

// --- Variables ---
String tracklist[MAX_SONGS];
uint16_t track_count = 0;
Wav_File* test_track = nullptr;
TFT_eSPI tft = TFT_eSPI();
size_t bytes_written;

Channel* channel_1 = nullptr; 

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
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
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

    // Creating channels
    test_track = new Wav_File(tracklist[0]);

    channel_1 = new Channel(tracklist, &tft, track_count);
    channel_1->select_track(2);
    channel_1->display_tracklist(100, 5);
}

void loop() {
    channel_1->display_waveform(0, 0, TFT_WIDTH, 100);
    channel_1->playing = true;
    channel_1->update();
    // int16_t sample = channel_1->cur_sample;
    // Serial.printf("%d\n", sample);
}