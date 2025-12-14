#include <Arduino.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "Touch_Sensor.h"
#include "Touch_Wheel.h"

uint16_t volume = 50;

playlist_t* tracklist[10];
uint16_t num_playlists;

Touch_Wheel* touch_wheel;
Touch_Sensor* touch_sensors[NUM_SENSORS];
int8_t wheel_inc = 0;
char swipe_dir = '\0';
bool btn_pressed = false;

bool btn_last = false;
uint32_t btn_time = 0;

char swipe_last = '\0';
uint32_t swipe_time = 0;

uint16_t selected_track_ind = 0;

Channel* output;
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C* u8g2;

enum State_t { PLAYING, SONG_SELECT };
State_t current_state = SONG_SELECT;
uint16_t hovered_i = 0;

void setup() {
    Serial.begin(57600);
    delay(1000);
    Serial.printf("\n\n\nStarting DJ Mixer...\n");

    // Capacitive touch wheel
    for (int i = 1; i <= 13; i++) {
        if (i == 1) { // Button [1]
            touch_sensors[i-1] = new Touch_Sensor(i, 10000, 60000);
        } else if (i <= 9) { // Wheel [2, 9]
            touch_sensors[i-1] = new Touch_Sensor(i, 10000, 30000);
        } else { // Ring [10, 13]
            touch_sensors[i-1] = new Touch_Sensor(i, 10000, 50000);
        }
        touch_sensors[i-1]->init();
    }
    touch_wheel = new Touch_Wheel(touch_sensors);

    // Read SD card and load tracks
    Serial.printf("Initializing SD card");
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
    while (!SD.begin(SD_CS_PIN)) {
        Serial.printf(".");
    }
    Serial.printf("\n");

    num_playlists = load_tracklist_from_sd(TRACKS_DIR, tracklist);
    // print_tracklist(tracklist, num_playlists);
    Serial.printf("Num playlists: %d\n", num_playlists);

    // Select I2S pins
    i2s_pin_config_t i2s_pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DIN_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // Create channel
    Serial.println("Starting I2S Output");
    output = new Channel(tracklist, num_playlists);
    output->select_track(1, 0);
    output->start(I2S_NUM_1, i2s_pin_config);

    // Initialize display
    u8g2 = new U8G2_SSD1309_128X64_NONAME0_F_HW_I2C(U8G2_R2, OLED_RESET_PIN);
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    delay(100);
    u8g2->begin();
}

void loop() {
    // Audio streaming is handled by a task and runs asynchronously

    uint32_t now = millis();

    playlist_t* cur_playlist = tracklist[
        constrain(output->playlist_index, 0, num_playlists - 1)
    ];
    track_t* cur_track = cur_playlist->tracks[
        constrain(output->track_index, 0, cur_playlist->num_songs - 1)
    ];

    // Touch wheel
    wheel_inc = touch_wheel->wheel();
    swipe_dir = touch_wheel->swipe();
    btn_pressed = touch_wheel->button();

    // Display
    u8g2->clearBuffer();

    switch (current_state)
    {
    case PLAYING: {
        // Song title
        u8g2->setFont(u8g2_font_profont17_mr);
        u8g2->drawStr(0, 20, cur_track->song.c_str());

        // Song artist and playlist
        u8g2->setFont(u8g2_font_smallsimple_tr);
        u8g2->drawStr(0, 40, ("by " + cur_track->artist).c_str());
        u8g2->drawStr(0, 50, ("in " + cur_playlist->name).c_str());
        u8g2->drawStr(0, 60, (String(volume / 2) + " db").c_str());

        // Icon
        u8g2->setFont(u8g2_font_streamline_all_t);
        char icon = 147;
        u8g2->drawStr(108, 61, &icon);

        // WAV statistics
        u8g2->setFont(u8g2_font_tiny5_te);
        u8g2->drawStr(90, 47, ".wav");
        u8g2->drawStr(83, 54, (String(output->track->bits_per_sample) + " bits").c_str());
        u8g2->drawStr(76, 61, (String(output->track->sample_rate ) + " hz").c_str());

        // Battery level;
        // u8g2->setFont(u8g2_font_tiny5_te);
        // u8g2->drawStr(110, 8, (String(get_battery(BAT_LVL_PIN)) + "%").c_str());

        // Pause indicator
        if (output->paused) {
            u8g2->setFont(u8g2_font_tiny5_te);
            u8g2->drawStr(49, 30, "PAUSED");
        }

        hovered_i = 0;

        volume = constrain(volume + wheel_inc, 0, 100);

        // Next/Prev Song
        if (swipe_dir != swipe_last && (now - swipe_time) > SWP_DEBOUNCE && wheel_inc == 0) {
            swipe_last = swipe_dir;
            swipe_time = now;

            switch (swipe_dir)
            {
            case 'U':
                current_state = SONG_SELECT;
                break;
            
            case 'L':
                output->track_index = constrain(output->track_index - 1, 0, cur_playlist->num_songs - 1);
                output->select_track(output->playlist_index, output->track_index);
                break;

            case 'R':
                output->track_index = constrain(output->track_index + 1, 0, cur_playlist->num_songs - 1);
                output->select_track(output->playlist_index, output->track_index);
                break;

            case 'D':
                current_state = SONG_SELECT;
                break;
            }
        }


        // Pause
        if (btn_pressed != btn_last && (now - btn_time) > BTN_DEBOUNCE) {
            btn_last = btn_pressed;
            btn_time = now;

            if (btn_pressed) {
                output->paused = !output->paused;
            }
        }        

        break;
    }
           
    case SONG_SELECT: {
        int16_t half_window_sz = SS_NUM_DISPLAYED_TRACKS / 2;
        int16_t start_i = 0;

        // Switch playlist
        if (swipe_dir != swipe_last && (now - swipe_time) > SWP_DEBOUNCE  && wheel_inc == 0) {
            swipe_last = swipe_dir;
            swipe_time = now;

            switch (swipe_dir)
            {
            case 'U':
                current_state = PLAYING;
                break;
            
            case 'L':
                output->playlist_index = constrain(output->playlist_index - 1, 0, num_playlists - 1);
                hovered_i = 0;
                break;

            case 'R':
                output->playlist_index = constrain(output->playlist_index + 1, 0, num_playlists - 1);
                hovered_i = 0;
                break;

            case 'D':
                current_state = PLAYING;
                break;
            }
        }

        u8g2->setFont(u8g2_font_smallsimple_tr);
        u8g2->drawStr(1, 10, (String(output->playlist_index) + ". " + cur_playlist->name).c_str());

        hovered_i = constrain(hovered_i + wheel_inc, 0, cur_playlist->num_songs - 1);

        if (hovered_i > half_window_sz) {
            start_i = hovered_i - half_window_sz;
        }

        if (start_i + SS_NUM_DISPLAYED_TRACKS > cur_playlist->num_songs) {
            start_i = max(0, cur_playlist->num_songs - SS_NUM_DISPLAYED_TRACKS);
        }

        u8g2->setFont(u8g2_font_tiny5_te);

        for (uint16_t i = 0; 
            i < SS_NUM_DISPLAYED_TRACKS && 
            (start_i + i) < cur_playlist->num_songs; 
            i++) 
        {
            uint16_t y_pos = (i + 1) * 10 + 10;

            track_t* t = cur_playlist->tracks[start_i + i];
            String label = t->artist + " - " + t->song;

            u8g2->drawStr(10, y_pos, label.c_str());

            if (start_i + i == hovered_i) {
                u8g2->drawStr(0, y_pos, ">");
            }
        }

        // Select song
        if (btn_pressed != btn_last && (now - btn_time) > BTN_DEBOUNCE && wheel_inc == 0 && swipe_dir == '\0') {
            btn_last = btn_pressed;
            btn_time = now;

            if (btn_pressed) {
                output->select_track(output->playlist_index, hovered_i);
                output->paused = false;
                current_state = PLAYING;
                return;
            }
        }

        break;
    }  
    }

    u8g2->sendBuffer();
}

// Cool fonts
// u8g2->setFont(u8g2_font_Pixellari_te);
// u8g2->setFont(u8g2_font_chargen_92_me);
// u8g2->setFont(u8g2_font_commodore64_tr);
// u8g2->setFont(u8g2_font_crox4hb_tr);
// u8g2->setFont(u8g2_font_maniac_te);
// u8g2->setFont(u8g2_font_pixzillav1_tf);


// u8g2->setFont(u8g2_font_5x8_mf);
// u8g2->setFont(u8g2_font_crox1h_tr);
// u8g2->setFont(u8g2_font_ordinarybasis_tr);
// u8g2->setFont(u8g2_font_nine_by_five_nbp_t_all);