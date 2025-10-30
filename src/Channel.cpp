#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "driver/i2s.h"

Channel::Channel(String* tracklist, TFT_eSPI* tft, uint16_t track_count, uint16_t color) {
    this->tracklist = tracklist; // Passed by pointer
    this->track = Wav_File();
    this->tft = tft; // Passed by pointer
    this->track_index = -1;
    this->num_tracks = track_count;
    this->playing = false;
    this->total_samples_played = 0;
    this->color = color;
}

void Channel::select_track(int32_t track_num) {
    track_index = track_num;
    track_index = max(track_index, 0);
    track_index = min(track_index, num_tracks - 1);
    track.load(tracklist[track_index]);

    if (!track.is_empty) {
        track.empty_cache();
        buf_index = 0;
        total_samples_played = 0;
        track.sample_index = 0;
        fill_i2s_buf();
        playing = true;
    }
}

void Channel::display_waveform(uint16_t x, uint16_t y, uint16_t height) {

    tft->fillRect(x, y, AUDIO_WINDOW_SZ, height, BACKGROUND_COLOR);

    if (track.is_empty) {
        tft->drawLine(x, y + height / 2, x + AUDIO_WINDOW_SZ, y + height / 2, color);
        return;
    }

    int16_t* window = track.get_window(total_samples_played);

    for(uint16_t i = 0; i < AUDIO_WINDOW_SZ; i++) {
        int16_t nor_sample = window[i] * (height / 2) / INT16_MAX;
        int16_t abs_sample = abs(nor_sample);
        int16_t sample_y = y + (height) / 2;

        tft->fillRect(x + i, sample_y - abs_sample, 1, abs_sample * 2, color);
    }
}

void Channel::update() {
    track.sample_index++;

    uint64_t cache_sample_pos = (track.get_cache_pos() - 44, static_cast<long>(0)) / 2; // Don't update cache if we haven't gotten that far into our window
    if (track.is_empty || !playing || track.sample_index < cache_sample_pos) { 
        return; 
    } else {
        fill_i2s_buf();
    }

}

void Channel::fill_i2s_buf() {
    int16_t sample_buf[I2S_BUF_FRAMES];
    ssize_t samples_read = track.read_samples(sample_buf, I2S_BUF_FRAMES);

    if (samples_read <= 0) {
        playing = false;
        return;
    }

    for (ssize_t i = 0; i < samples_read; ++i) {
        int16_t sample = static_cast<int16_t>(sample_buf[i] * VOLUME);
        i2s_buffer[2 * buf_index]     = sample;
        i2s_buffer[2 * buf_index + 1] = sample;
        buf_index++;

        if (buf_index >= I2S_BUF_FRAMES) {
            size_t bytes_written;
            i2s_write(I2S_NUM_0, i2s_buffer,
                      I2S_BUF_FRAMES * 2 * sizeof(int16_t),
                      &bytes_written,
                      portMAX_DELAY);
            buf_index = 0;
            total_samples_played += (bytes_written / (2 * sizeof(int16_t)));
        }
    }
}