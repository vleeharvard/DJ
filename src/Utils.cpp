#include <Arduino.h>
#include <SD.h>
#include <Utils.h>

void generate_track(String dir, String track_name, track_t* track) {
    int semicolon_index = track_name.indexOf(';');
    track->artist = track_name.substring(0, semicolon_index);
    track->artist.replace("_", " ");
    track->song = track_name.substring(semicolon_index + 1, track_name.length() - 4);
    track->song.replace("_", " ");
    track->filepath = dir + "/" + track_name;
}

playlist_t* load_playlist_from_dir(String filepath) {
    File dir = SD.open(filepath);
    playlist_t* playlist = new playlist_t; 
    
    uint16_t track_count = 0;
    while (true) {
        File track_entry = dir.openNextFile();

        if (!track_entry) { break; }

        String track_name = String(track_entry.name());
        if (track_name.endsWith(".wav") && track_count < MAX_SONGS_PER_PLAYLIST) {
            track_t* track = new track_t;
            generate_track(filepath, track_name, track);
            playlist->tracks[track_count++] = track;
        }
    }

    playlist->name = String(dir.name());
    playlist->name.replace("_", " ");
    playlist->num_songs = track_count;
    return playlist;
}

uint16_t load_tracklist_from_sd(String root_filepath, playlist_t** tracklist) {
    File dir = SD.open(root_filepath);
    uint16_t num_playlists = 0;

    playlist_t* playlistless_playlist = new playlist_t;
    playlistless_playlist->name = "No Playlist";
    playlistless_playlist->num_songs = 0;
    tracklist[num_playlists++] = playlistless_playlist;

    while (true) {
        File entry = dir.openNextFile();
        if (!entry) { break; }

        if (entry.isDirectory() && num_playlists <= MAX_PLAYLISTS && !String(entry.name()).startsWith(".")) {
            tracklist[num_playlists++] = load_playlist_from_dir(root_filepath + String(entry.name()));
        } else { // Entry is a just a playlist-less song, put it in the filler playlist

            if (String(entry.name()).endsWith(".wav") && playlistless_playlist->num_songs < MAX_SONGS_PER_PLAYLIST) {
                track_t* general_track = new track_t;
                generate_track("", String(entry.name()), general_track);
                playlistless_playlist->tracks[playlistless_playlist->num_songs++] = general_track;
            }

        }
        entry.close();
    }

    return num_playlists;
}

void print_tracklist(playlist_t* tracklist[], int num_playlists) {
    for (int i = 0; i < num_playlists; i++) {
        playlist_t* pl = tracklist[i];
        if (pl == nullptr) continue; // skip empty entries

        Serial.printf("playlist %d: %s\n", i + 1, pl->name.c_str());

        for (int j = 0; j < pl->num_songs; j++) {
            track_t* t = pl->tracks[j];
            if (t == nullptr) continue; // skip empty song slots

            Serial.printf("%d) %s, %s, %s\n",
                j + 1,
                t->artist.c_str(),
                t->song.c_str(),
                t->filepath.c_str()
            );
        }

        Serial.println();  // blank line between playlists
    }
}