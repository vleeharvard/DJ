#include <Arduino.h>
#include <SD.h>
#include "Wav_File.h"

Wav_File::Wav_File() {
    this->is_empty = true;
    this->sample_index = 0;
}

void Wav_File::load(const String& file_name) {
    wav_file = SD.open(AUDIO_DIR + file_name);
    if (!wav_file) {
        is_empty = true;
        assert(0);
        return;
    }
    parse_header();
    empty_cache();
    is_empty = false;
}

Wav_File::Wav_File(const String& file_name) {
    load(file_name);
}

void Wav_File::parse_header() {

    wav_file.read((uint8_t*)&header, sizeof(WAV_HEADER));

    if (memcmp(header.RIFF, "RIFF", 4) != 0 || memcmp(header.WAVE, "WAVE", 4) != 0) { // Verify integrity of file
        Serial.printf("File %s is not .wav\n", wav_file.name());
        wav_file.close();
        is_empty = true;
        return;
    }

    num_of_chan = header.num_of_chan;
    sample_rate = header.sample_rate;
    data_size = header.subchunk_2_size;

    // Serial.printf("RIFF[4] = %c%c%c%c\n", header.RIFF[0], header.RIFF[1], header.RIFF[2], header.RIFF[3]);
    // Serial.printf("WAVE[4] = %c%c%c%c\n", header.WAVE[0], header.WAVE[1], header.WAVE[2], header.WAVE[3]);

    wav_file.seek(sizeof(WAV_HEADER)); // Move cursor to beginning of data chunk
}

void Wav_File::empty_cache() {
    cache.start_tag = cache.pos_tag = cache.end_tag = sizeof(WAV_HEADER); // Cache begins at data chunk
}

void Wav_File::fill_cache() {
    cache.start_tag = max(cache.end_tag - AUDIO_WINDOW_RES, (long) 0);
    ssize_t n = wav_file.read(cache.cbuf, WAV_CACHE_SZ);

    if (n <= 0) {
        assert(0);
    }
    
    cache.end_tag = cache.start_tag + n;
    cache.pos_tag = cache.start_tag;
}

off_t Wav_File::get_cache_pos() {
    return cache.pos_tag;
}

ssize_t Wav_File::read_samples(int16_t* buf, size_t sz) {

    size_t pos = 0;
    while (pos < sz) {
        if (cache.pos_tag == cache.end_tag) {
            fill_cache();
        }
        if (cache.pos_tag == cache.end_tag) {
            break;
        }

        uint8_t low = cache.cbuf[cache.pos_tag - cache.start_tag];
        uint8_t high = cache.cbuf[cache.pos_tag - cache.start_tag + 1];
        int16_t sample = (int16_t)(high << 8 | low); // Samples are little-endian
        memcpy(&buf[pos], &sample, 2); // buf is an array of samples
        cache.pos_tag += 2; // pos_tag is in bytes
        pos += 1; // pos (and buf) are in samples
    }

    return pos;
}

void Wav_File::seek_sample(uint32_t sample_pos) {
    // TO DO: Clear cache whenever we seek (DONE?)
    cache.end_tag = sizeof(WAV_HEADER) + sample_pos * 2;
    empty_cache();
    fill_cache();

    wav_file.seek(
        sizeof(WAV_HEADER) + 2 * sample_pos
    );
}

int16_t* Wav_File::get_window(uint64_t cen_pos) {
    /*
    New get_window should return an array of sz representing a
    strided selection of samples based on AUDIO_WINDOW_RES. It
    should NOT need call read_samples (so we don't increment)
    and should just pull straight from the cache.

    Ongoing issue with the I2S playback updating faster than
    the window can keep up. I2S playback will constantly push
    the cache forward, so the window can't keep up.

    */

    static int16_t window[AUDIO_WINDOW_SZ];
    off_t window_start = static_cast<int64_t>(cen_pos * 2)  - AUDIO_WINDOW_RES / 2; // In bytes

    for (uint16_t i = 0; i < AUDIO_WINDOW_SZ; i++) {
        uint16_t stride = AUDIO_WINDOW_RES / AUDIO_WINDOW_SZ;
        off_t sample_pos = window_start + i * stride; // In bytes

        if (sample_pos > cache.end_tag) {
            fill_cache();
        }
        
        if (sample_pos < sizeof(WAV_HEADER)) {
            window[i] = 0;
        } else {
            uint8_t low = cache.cbuf[sample_pos - cache.start_tag];
            uint8_t high = cache.cbuf[sample_pos - cache.start_tag + 1];
            int16_t sample = (int16_t)(high << 8 | low);
            window[i] = sample;
        }
    }

    return window;
}

uint32_t Wav_File::get_position() {
    return wav_file.position();
}