#include <Arduino.h>
#include <SD.h>
#include "Wav_File.h"

Wav_File::Wav_File() {
    this->is_empty = true;
}

void Wav_File::load(const String& file_name) {
    wav_file = SD.open(AUDIO_DIR + file_name);
    if (!wav_file) {
        is_empty = true;
        assert(0);
        return;
    }
    parse_header();
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

int16_t Wav_File::read_sample() {
    int16_t sample = 0;

    if (wav_file.available() < 2) {
        return -1;
    }

    uint8_t low = wav_file.read();
    uint8_t high = wav_file.read();

    sample = (int16_t)(high << 8 | low);  // Samples are little-endian
    // Important to recognize that this, like File.read(), will also increment the file cursor
    return sample;
}

void Wav_File::seek_sample(uint32_t sample_pos) {
    wav_file.seek(
        sizeof(WAV_HEADER) + 2 * sample_pos
    );
}

int16_t* Wav_File::get_window() {
    uint32_t saved_file_pos = wav_file.position();
    int32_t file_pos = (int32_t) wav_file.position() - AUDIO_WINDOW_SZ / 4;
    file_pos = max(file_pos, static_cast<int32_t>(sizeof(WAV_HEADER))); // Clamp to above the .wav header

    wav_file.seek(file_pos);
    for (int i_window = 0; i_window < AUDIO_WINDOW_SZ; i_window++) {
        window[i_window] = read_sample(); // Simultaneously increments the file cursor
    }
    wav_file.seek(saved_file_pos); // get_window() should preserve position of file cursor
    return window;
}

uint32_t Wav_File::get_position() {
    return wav_file.position();
}