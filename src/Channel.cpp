#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "driver/i2s.h"

Channel::Channel(playlist_t** tracklist, uint16_t num_playlists) {
    this->tracklist = tracklist; // Passed by pointer
    this->num_playlists = num_playlists;
    
    this->track = new Wav_File();

    this->track_index = -1;
    this->playlist_index = -1;
    this->paused = true;
}

void Channel::select_track(int32_t playlist_i, int32_t track_i) {
    playlist_index = constrain(playlist_i, 0, num_playlists - 1);
    track_index = constrain(track_i, 0, tracklist[playlist_index]->num_songs - 1); // TO DO: CHECK FOR -1

    track->load(tracklist[playlist_index]->tracks[track_index]->filepath);

    if (!track->is_empty) {
    }
}

void i2s_writer_task(void *param) {
    Channel *output = (Channel*)param;

    Frame_t *frames = (Frame_t*)malloc(sizeof(Frame_t) * NUM_FRAMES_TO_SEND);

    while (true) {

        if (output->paused) {
            vTaskDelay(5);
            continue;
        }

        size_t frames_read = output->track->read_frames(frames, NUM_FRAMES_TO_SEND);

        if (frames_read == 0) { // END OF SONG
            playlist_t *cur_playlist = output->tracklist[output->playlist_index];

            if (output->track_index >= cur_playlist->num_songs) {
                output->paused = true;
                output->track_index = 0;
                vTaskDelay(1);
                continue;
            }

            output->track_index = constrain(output->track_index + 1, 0, cur_playlist->num_songs - 1);
            output->select_track(output->playlist_index, output->track_index);
            continue;
        }

        size_t bytes_written;
        i2s_write(output->i2s_port,
                  (const char*)frames,
                  frames_read * sizeof(Frame_t),
                  &bytes_written,
                  portMAX_DELAY);

        vTaskDelay(1);
    }
}

void Channel::start(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pin_config) {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = track->sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false, 
        .tx_desc_auto_clear = true,
    };

    this->i2s_port = i2s_port;
    i2s_driver_install(i2s_port, &i2s_config, 4, &i2s_queue);
    i2s_set_pin(i2s_port, &i2s_pin_config);
    i2s_zero_dma_buffer(i2s_port);
    TaskHandle_t i2s_writer_task_handle;
    xTaskCreate(i2s_writer_task, "i2s Writer Task", 8192, this, 1, &i2s_writer_task_handle);
}