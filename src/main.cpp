#include <Arduino.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- Custom libraries ---
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "Touch_Sensor.h"

// --- Constants ---
#define MAX_SONGS 100
#define TRACKS_DIR "/"

// --- Variables ---
String tracklist[MAX_SONGS];
uint16_t track_count = 0;
size_t bytes_written;
uint16_t selected_track_ind = 0;

Channel* output;
Touch_Sensor* touch1;
Touch_Sensor* touch2;
Touch_Sensor* touch3;
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C* u8g2;

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

    // Creating touch
    touch1 = new Touch_Sensor(1, 38000);
    touch2 = new Touch_Sensor(2, 38000);
    touch3 = new Touch_Sensor(3, 38000);

    // Creating display
    u8g2 = new U8G2_SSD1309_128X64_NONAME0_F_HW_I2C(U8G2_R0, OLED_RESET);
    Wire.begin(OLED_SDA, OLED_SCL);
    delay(100);
    u8g2->begin();
}

void loop() {
    // Nothing here since everything is handled by tasks
    touch1->update();
    touch2->update();
    touch3->update();

  char line[64];

  snprintf(line, sizeof(line),
          "T1:%s  T2:%s  T3:%s",
          touch1->pressed() ? "ON" : "OFF",
          touch2->pressed() ? "ON" : "OFF",
          touch3->pressed() ? "ON" : "OFF");

  u8g2->firstPage();
  do {
    u8g2->setFont(u8g2_font_6x10_tf);
    u8g2->drawStr(0, 12, line);
  } while (u8g2->nextPage());

  delay(50);
}