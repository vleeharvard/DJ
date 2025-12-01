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

Wav_File::~Wav_File()
{
    wav_file.close();
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
    data_size = header.data_size;
    bits_per_sample = header.bits_per_sample;
    data_start = sizeof(WAV_HEADER);

    wav_file.seek(sizeof(data_start)); // Move cursor to beginning of data chunk
}

void Wav_File::read_frames(Frame_t *frames, size_t num_frames) {
    for (uint32_t i = 0; i < num_frames; i++) {
        if (wav_file.available() == 0) {
            is_empty = true;
        }

        wav_file.read((uint8_t*)(&frames[i].left), sizeof(int16_t));
        if (num_of_chan == 1) {
            frames[i].right = frames[i].left;
        } else {
            wav_file.read((uint8_t*)(&frames[i].left), sizeof(int16_t));
        }
    }
}
