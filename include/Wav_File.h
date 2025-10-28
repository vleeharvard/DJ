// Custom library that for .wav files with extra functionality relevant to
//  wav_files and my project.

#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <Arduino.h>
#include <SD.h>
#include "Utils.h"

struct WAV_HEADER {
    uint8_t   RIFF[4];              // Should be "RIFF"
    uint32_t  chunk_size;           // Overall file size minus 8 bytes
    uint8_t   WAVE[4];              // Shoould be "WAVE"

    uint8_t   fmt[4];               // Should be "fmt "
    uint32_t  subchunk_1_size;      // Size of the fmt chunk (16 for PCM)
    uint16_t  audio_format;         // 1 = PCM
    uint16_t  num_of_chan;          // 1 = mono, 2 = stereo
    uint32_t  sample_rate;          // Sample rate (e.g. 44100)
    uint32_t  byte_rate;            // SampleRate * NumChannels * BitsPerSample/8
    uint16_t  block_align;          // NumChannels * BitsPerSample/8
    uint16_t  bits_per_sample;      // Bits per sample (e.g. 16)

    uint8_t   subchunk_2_ID[4];     // "data"
    uint32_t  subchunk_2_size;      // Number of bytes of actual audio data
};

class Wav_File {
  private:
    File wav_file;
    WAV_HEADER header;
    int16_t window[AUDIO_WINDOW_SZ];

  public:
    uint16_t num_of_chan = 0;
    uint32_t sample_rate = 0;
    uint32_t data_size = 0;
    bool playing = false;
    bool is_empty = false;

    // Constructor
    explicit Wav_File();
    explicit Wav_File(const String& file_name);

    // Methods
    void load(const String& file_name);
    void parse_header();
    int16_t read_sample();
    void seek_sample(uint32_t sample_pos);
    int16_t* get_window();
    uint32_t get_position();
};

#endif // WAV_FILE_H