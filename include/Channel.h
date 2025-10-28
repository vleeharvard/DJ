#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Wav_File.h"

#ifndef CHANNEL_FILE_H
#define CHANNEL_FILE_H

class Channel {
    private:
    String* tracklist;
    Wav_File track;
    TFT_eSPI* tft;
    uint16_t num_tracks;

    static const int I2S_BUF_FRAMES = 256;
    int16_t i2s_buffer[I2S_BUF_FRAMES * 2]; // stereo interleaved
    int buf_index = 0;                       // current fill position

    public:
    int32_t track_index;
    int16_t cur_sample;
    bool playing;

    // Constructor
    explicit Channel(String* tracklist, TFT_eSPI* tft, u_int16_t track_count);

    // Methods
    void display_waveform(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void display_tracklist(uint16_t y, uint16_t num_tracks_to_display);
    void select_track(int32_t track_num);
    void update();
};

#endif // CHANNEL_FILE_H