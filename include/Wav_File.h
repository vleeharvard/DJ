// Custom library that for .wav files with extra functionality relevant to
//  wav_files and my project.

#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <Arduino.h>
#include <SD.h>
#include "Utils.h"

struct wav_header_t {
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

    uint8_t   data_ID[4];     // "data"
    uint32_t  data_size;      // Number of bytes of actual audio data
} __attribute__((packed));

struct cache_t {
    off_t start_tag; // File offset of first byte of cached data (0 when file is opened).
    off_t end_tag; // File offset one past the last byte of cached data (0 when file is opened).
    off_t pos_tag; // `pos_tag`: Cache position: file offset of the cache.
    uint8_t cbuf[WAV_CACHE_SZ]; // Cache is in bytes, not samples
};

class Wav_File {
    private:
    File wav_file;
    wav_header_t header;
    cache_t cache;

    void refill_cache();

    public:
    uint16_t num_of_chan = 0;
    uint32_t sample_rate = 0;
    uint32_t data_size = 0;
    uint32_t bits_per_sample = 0;
    uint32_t data_start = 0;
    bool playing = false;
    bool is_empty = false;

    Wav_File();
    Wav_File(const String& file_name);
    ~Wav_File();

    void load(const String& file_name);
    void parse_header();
    size_t read_frames(Frame_t *frames, size_t num_frames = 1);

    void empty_cache();
    void fill_cache();
};

#endif // WAV_FILE_H