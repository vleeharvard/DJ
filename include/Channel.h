#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "driver/i2s.h"

#include "Wav_File.h"

#ifndef CHANNEL_FILE_H
#define CHANNEL_FILE_H

class Channel {
    private:
    String* tracklist;
    uint16_t num_tracks;

    TaskHandle_t i2s_writer_task_handle;
    QueueHandle_t i2s_queue;
    i2s_port_t i2s_port;

    public:
    Wav_File* track;
    int32_t track_index;

    Channel(String* tracklist, u_int16_t track_count);

    void select_track(int32_t track_num);
    void start(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pin_config);
    friend void i2s_writer_task(void *param);
};

#endif // CHANNEL_FILE_H