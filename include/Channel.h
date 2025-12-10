#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "driver/i2s.h"

#include "Wav_File.h"

#ifndef CHANNEL_FILE_H
#define CHANNEL_FILE_H

class Channel {
    private:
    playlist_t** tracklist;
    uint16_t num_playlists;

    TaskHandle_t i2s_writer_task_handle;
    QueueHandle_t i2s_queue;
    i2s_port_t i2s_port;

    public:
    Wav_File* track;
    int16_t playlist_index;
    int16_t track_index;

    bool paused;

    Channel(playlist_t** tracklist, uint16_t num_playlists);

    void select_track(int32_t playlist, int32_t track);
    void start(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pin_config);
    friend void i2s_writer_task(void *param);
};

#endif // CHANNEL_FILE_H