#include <Arduino.h>
#include <SD.h>
#include "Wav_File.h"

Wav_File::Wav_File() {
    this->is_empty = true;
}

void Wav_File::load(const String& file_name) {
    if (wav_file) { wav_file.close(); }
    wav_file = SD.open(file_name);
    if (!wav_file) {
        is_empty = true;
        assert(0);
        return;
    }
    parse_header();
    is_empty = false;
}

Wav_File::Wav_File(const String& file_name) {
    wav_file.close();
    load(file_name);
}

Wav_File::~Wav_File() {
    wav_file.close();
}

void Wav_File::parse_header() {

    wav_file.read((uint8_t*)&header, sizeof(wav_header_t));

    if (memcmp(header.RIFF, "RIFF", 4) != 0 || memcmp(header.WAVE, "WAVE", 4) != 0) { // Verify integrity of file
        Serial.printf("File %s is not .wav\n", wav_file.name());
        wav_file.close();
        is_empty = true;
        return;
    }

    num_of_chan = header.num_of_chan;
    sample_rate = header.sample_rate;
    data_size = header.data_size;
    bits_per_sample = header.bits_per_sample;
    data_start = sizeof(wav_header_t);

    wav_file.seek(data_start); // Move cursor to beginning of data chunk
}

size_t Wav_File::read_frames(Frame_t *frames, size_t num_frames) {
    size_t frames_read = 0;
    for (uint32_t i = 0; i < num_frames; i++) {
        if (wav_file.available() == 0) {
            is_empty = true;
        }

        if (cache.pos_tag + 1 >= cache.end_tag) {
            fill_cache();
        }
        if (cache.pos_tag + 1 >= cache.end_tag) {
            break;
        }

        uint8_t low = cache.cbuf[cache.pos_tag - cache.start_tag];
        uint8_t high = cache.cbuf[cache.pos_tag - cache.start_tag + 1];
        frames[i].left = (int16_t)(high << 8 | low) * (constrain(0, volume, 100) / 100.0f);
        cache.pos_tag += 2;

        if (num_of_chan == 1) {
            frames[i].right = frames[i].left;
        } else {
            low = cache.cbuf[cache.pos_tag - cache.start_tag];
            high = cache.cbuf[cache.pos_tag - cache.start_tag + 1];
            frames[i].right = (int16_t)(high << 8 | low) * (constrain(0, volume, 100) / 100.0f);
            cache.pos_tag += 2;
        }
        frames_read += 1;
    }

    return frames_read;
}

void Wav_File::empty_cache() {
    cache.start_tag = cache.pos_tag = cache.end_tag = sizeof(wav_header_t); // Cache begins at data chunk
}

void Wav_File::fill_cache() {
    cache.start_tag = cache.end_tag;
    ssize_t n = wav_file.read(cache.cbuf, WAV_CACHE_SZ);
    
    cache.end_tag = cache.start_tag + n;
    cache.pos_tag = cache.start_tag;
}