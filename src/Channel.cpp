#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "driver/i2s.h"

Channel::Channel(String* tracklist, TFT_eSPI* tft, uint16_t track_count) {
    this->tracklist = tracklist; // Passed by pointer
    this->track = Wav_File();
    this->tft = tft; // Passed by pointer
    this->track_index = -1;
    this->num_tracks = track_count;
    this->playing = false;
    this->cur_sample = 0;
}

void Channel::display_waveform(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    // Width must be divisible by buffer size 

    tft->fillRect(x, y, width, height, BACKGROUND_COLOR);

    if (track.is_empty) {
        tft->drawLine(x, y + height / 2, x + width, y + height / 2, PRIMARY_COLOR);
        return;
    }

    int16_t* window = track.get_window();
    int16_t stride = AUDIO_WINDOW_SZ / width;

    for(uint16_t sample_x = 0; sample_x < width; sample_x++) {
        int16_t sample = window[sample_x * stride];
        int16_t nor_sample = (int32_t) sample * (height / 2) / INT16_MAX;
        int16_t abs_sample = abs(nor_sample);
        int16_t sample_y = (height) / 2;
        
        tft->fillRect(x + sample_x, sample_y - abs_sample, 1, abs_sample * 2, PRIMARY_COLOR);

        // Serial.printf("%d\n", abs_sample);
    }
}

void Channel::select_track(int32_t track_num) {
    track_index = track_num;
    track_index = max(track_index, 0);
    track_index = min(track_index, num_tracks - 1);
    track.load(tracklist[track_index]);
}

void Channel::display_tracklist(uint16_t y, uint16_t num_tracks_to_display) {
    if (num_tracks == 0) { return; }

    int16_t half_window = num_tracks_to_display / 2;
    int16_t start_index = track_index > half_window ? track_index - half_window : 0;

    if (start_index + num_tracks_to_display > num_tracks) {
        start_index = max(0, num_tracks - num_tracks_to_display);
    }

    tft->fillRect(0, y, TFT_WIDTH, num_tracks_to_display * (LINE_HEIGHT + TEXT_PADDING * 2), BACKGROUND_COLOR);

    for (uint16_t i = 0; i < num_tracks_to_display && (start_index + i) < num_tracks; i++) {
        uint16_t track_num = start_index + i;
        uint16_t track_y = y + i * (LINE_HEIGHT + TEXT_PADDING * 2);

        bool is_selected = (track_num == track_index);

        if (is_selected) {
            tft->fillRoundRect(
                0,
                track_y,
                TFT_WIDTH,
                LINE_HEIGHT + 2 * TEXT_PADDING,
                BORDER_RADIUS,
                PRIMARY_COLOR
            );
            tft->setTextColor(BACKGROUND_COLOR, PRIMARY_COLOR);
        } else {
            tft->setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
        }

        tft->drawString(tracklist[track_num], TEXT_PADDING, track_y + LINE_HEIGHT / 2);
    }
}

// void Channel::update() {
//     if (!track.is_empty && playing) {
//         cur_sample = track.read_sample();
//     }
// }

void Channel::update() {
    if (track.is_empty || !playing) return;

    const int BUF_FRAMES = 256;

    while (buf_index < BUF_FRAMES) {
        int16_t sample = track.read_sample() * VOLUME;

        // Stop filling only at real EOF
        if (sample == -1) {
            playing = false;
            break;
        }

        // Duplicate mono sample for stereo
        i2s_buffer[2 * buf_index]     = sample; // left
        i2s_buffer[2 * buf_index + 1] = sample; // right
        buf_index++;
    }

    // Flush buffer when full
    if (buf_index >= BUF_FRAMES) {
        size_t bytes_written;
        i2s_write(I2S_NUM_0, i2s_buffer, BUF_FRAMES * 2 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        buf_index = 0; // reset for next batch
    }
}
