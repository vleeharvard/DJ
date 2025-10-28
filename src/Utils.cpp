#include <Arduino.h>
#include <SD.h>

uint16_t load_tracklist_from_file(File directory, String* tracklist, uint32_t max_songs) {
    uint16_t track_count = 0;
    while (true) {
    File entry = directory.openNextFile();
    if (!entry) { break; }
    if (!entry.isDirectory()) {
        String track_name = String(entry.name());
        String track_name_lower = track_name;
        track_name_lower.toLowerCase();
        if (track_name_lower.endsWith(".wav") && track_count < max_songs) {
            tracklist[track_count++] = String(entry.name());
        }
    }
    entry.close();
    }
    return track_count;
}

