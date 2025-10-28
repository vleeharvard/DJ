#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "driver/i2s.h"

/*
Functionality of each channel in the DJ Mixer:
- Pull music from tracklist
  - Each channel should be able to independently access the tracklist and SD card
- Play music from the queue
  - Always keep track of a "selected" song.
  - Pressing a button will add this selection to the queue.
  - If the queue is empty, play the "selected" song 
- On-the-fly high, low, medium band pass filter
- Should be able to handle 8-bit PCM or 16-bit PCM .wav files
  - Adjust the amplitude of the waves displayed accordingly
  - Calculate amplitude of the sample differently depending on the bits per sample
  - "Resolution" is another way to say bits per sample
  - However, each song needs to have the same sample rate in order for simultaneous
    audio to work.

*/

// --- Constants ---
#define MAX_SONGS 100
#define SD_CS_PIN 16
#define TRACKS_DIR "/"
#define AUDIO_WINDOW_SZ 1024 // In samples
#define MAX_WAVE_AMP 100

// The first 44 bytes of a .wav file are a header file that stores metada about the file.
//    This includes sample_rate, which the playback rate must match, and bits_per_sample
//    which tells us how many bits are in each sample. Note that bits_per_sample is
//    crucial for both playback and displaying the the .wav file as a waveform since it
//    tells us the range of values a given sample can have.
struct wav_header {
  char chunk_ID[4];
  uint32_t chunk_size;
  char format[4];
  char subchunk_1_ID[4];
  uint32_t subchunk_1_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  char subchunk_2_ID[4];
  uint32_t subchunk_2_size;
};

// Global timer shared by all channels
hw_timer_t* global_timer = nullptr;
portMUX_TYPE global_timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool global_update_flag = false;

void IRAM_ATTR on_global_timer() {
  portENTER_CRITICAL_ISR(&global_timerMux);
  global_update_flag = true;
  portEXIT_CRITICAL_ISR(&global_timerMux);
}

void setup_global_timer(uint32_t sample_rate) {
    if (global_timer != nullptr) { timerEnd(global_timer); }

    uint16_t prescaler = 80000000 / sample_rate;  // APB freq = 80 MHz
    global_timer = timerBegin(0, prescaler, true);
    timerAttachInterrupt(global_timer, &on_global_timer, true);
    timerAlarmWrite(global_timer, 1, true);
    timerAlarmEnable(global_timer);

    Serial.printf("Global timer started at %d Hz\n", sample_rate);
}

class Channel {
  private:
    String* tracklist;
    int track_index = 0;
    // LinkedList<String> queue;
    String current_track_name;

    File current_wav;
    wav_header header;
    // TO DO: Use a queue instead of an array to increase performance
    int16_t wave_window[AUDIO_WINDOW_SZ];
    int sample_index;
    volatile bool playing = false;

    hw_timer_t *timer = nullptr;  

  public:
    Channel() {
      this->tracklist = nullptr;
    }

    Channel(String* tracklist) {
      this->tracklist = tracklist;
    }

    void load_next_song() {
      assert(tracklist != nullptr);

      if (queue.size() != 0) {
        current_track_name = queue.shift();
      } else {
        current_track_name = tracklist[track_index];
        track_index++;
        if (track_index >= MAX_SONGS) { track_index = 0; } // Loop back
      }

      if (current_wav) { current_wav.close(); }
      current_wav = SD.open("/" + current_track_name);

      if (!current_wav) {
        Serial.println("Failed to open " + current_track_name);
        return;
      }

      current_wav.read((uint8_t*)&header, sizeof(header));
      Serial.printf("Playing %s (%d channels, %d Hz)\n", current_track_name.c_str(),
                    header.num_channels, header.sample_rate);
      playing = true;
      sample_index = 0;
    }

    // Reads a sample at a designated position of the current .wav file. Depending the
    //    on the number of bits per sample, or the resolution of the .wav, adjust the 
    //    number of bytes sampled (2 bytes for 16-bit PCM and 1 byte for 8-bit PCM) to 
    //    ensure the correct amplitude. Returns amplitude of sample.
    int16_t read_sample(int sample_pos) {
      current_wav.seek(sample_pos);
      if (!current_wav.available()) { return 0; }
      int16_t sample = 0;
      if (header.bits_per_sample == 16) {
          uint8_t low = current_wav.read();
          uint8_t high = current_wav.read();
          sample = (int16_t)(high << 8 | low);  // little-endian
      } else if (header.bits_per_sample == 8) {
          sample = (int16_t)(current_wav.read() - 128) << 8; // 8-bit PCM unsigned
      }
      return sample;
    }

    void fill_window() {
      if (!current_wav.available()) { return; }
      int16_t sample = 0;
      for (int i = 0; i < AUDIO_WINDOW_SZ; i++) {
        int sample_read_index = sample_index - AUDIO_WINDOW_SZ / 2 + i;
        if (sample_read_index >= 0) {
          sample = read_sample(sample_read_index);
        }

        // TO DO: Normalize amplitude of sample depending on bit rate
        wave_window[i] = sample;
      }
    }

    uint32_t get_sample_rate() {
      return header.sample_rate;
    }

    // Playback controls
    void play() { playing = true; }
    void stop() { playing = false; }
    void seek(uint32_t pos_in_samples) {

    }

    void update() {
      if (!playing) { return; }
      int16_t cur_sample = read_sample(sample_index);
      // TO DO: Play this sample through one of the DAC pins
      Serial.println(cur_sample);
      sample_index++;
      fill_window();
    }

};

// --- Load tracks from SD card
String tracklist[MAX_SONGS];
int track_count = 0;

void load_tracklist_from_file(File directory) {
  while (true) {
    File entry = directory.openNextFile();
    if (!entry) { break; }
    if (!entry.isDirectory()) {
      String track_name = String(entry.name());
      String track_name_lower = track_name;
      track_name_lower.toLowerCase();
      if (track_name_lower.endsWith(".wav") && track_count < MAX_SONGS) {
        tracklist[track_count++] = String(entry.name());
      }
    }
    entry.close();
  }
}

Channel channel_1;

void setup() {
  Serial.begin(9600);
  delay(1000);

  // Read SD card and load tracks
  Serial.println("Initializing SD card...");

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed.");
    while (1);
  }

  File tracks_dir = SD.open(TRACKS_DIR);
  load_tracklist_from_file(tracks_dir);

  Serial.printf("Found %d files:\n", track_count);
  for (int i = 0; i < track_count; i++) {
    Serial.println(tracklist[i]);
  }

  // Initialize channel(s)
  channel_1 = Channel(tracklist);
  channel_1.load_next_song();

  // Initialize global timer
  setup_global_timer(channel_1.get_sample_rate());

}

void loop() {
  if (global_update_flag) {
    portENTER_CRITICAL(&global_timerMux);
    global_update_flag = false;
    portEXIT_CRITICAL(&global_timerMux);

    channel_1.update();
  }
  
}