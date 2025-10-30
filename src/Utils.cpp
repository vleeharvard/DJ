#include <Arduino.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <Utils.h>

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

void display_tracklist(
    TFT_eSPI* tft,
    String* tracklist,
    uint16_t y, 
    uint16_t num_tracks_to_display, 
    uint16_t num_tracks, 
    uint16_t hovered_track_ind,
    uint16_t channel_1_ind
) {
    if (num_tracks == 0) { return; }

    int16_t half_window = num_tracks_to_display / 2;
    int16_t start_index = 0;
    if (hovered_track_ind > half_window) {
        start_index = hovered_track_ind - half_window;
    }

    if (start_index + num_tracks_to_display > num_tracks) {
        start_index = max(0, num_tracks - num_tracks_to_display);
    }

    tft->fillRect(0, y, TFT_WIDTH, num_tracks_to_display * (LINE_HEIGHT + TEXT_PADDING * 2), BACKGROUND_COLOR);

    for (uint16_t i = 0; i < num_tracks_to_display && (start_index + i) < num_tracks; i++) {
        uint16_t track_num = start_index + i;
        uint16_t track_y = y + i * (LINE_HEIGHT + TEXT_PADDING * 2);

        bool is_selected = (track_num == hovered_track_ind);
        bool channel_1_now_playing = (track_num == channel_1_ind);

        if (is_selected) {
            tft->fillRoundRect(
                0,
                track_y,
                TFT_WIDTH,
                LINE_HEIGHT + 2 * TEXT_PADDING,
                BORDER_RADIUS,
                PRIMARY_COLOR
            );
            tft->setTextColor(BACKGROUND_COLOR, PRIMARY_COLOR);
        } else if (channel_1_now_playing) {
            tft->fillRoundRect(
                0,
                track_y,
                TFT_WIDTH,
                LINE_HEIGHT + 2 * TEXT_PADDING,
                BORDER_RADIUS,
                ACCENT_COLOR_1
            );
            tft->setTextColor(BACKGROUND_COLOR, ACCENT_COLOR_1);
        } else {
            tft->setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
        }

        tft->drawString(tracklist[track_num], TEXT_PADDING, track_y + LINE_HEIGHT / 2);
    }
}

