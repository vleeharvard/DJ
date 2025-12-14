#include <Arduino.h>
#include "Touch_Wheel.h"
#include "Touch_Sensor.h"
#include "Utils.h"

Touch_Wheel::Touch_Wheel(Touch_Sensor** sensor_arr) {
    this->sensors = sensor_arr;
}


int16_t Touch_Wheel::wheel() {
    int wheel_state = 0;
    for (int i = 2; i <= 9; i++) {
        sensors[i-1]->update();
        if (sensors[i-1]->pressed()) {
            wheel_state = i;
            break;
        }
    }

    if (millis() - last_wheel_pressed_time > STORE_MS) {
        last_wheel_state = 0;
    }

    // Serial.printf("current_wheel_state: %d, last_wheel_state: %d              ", wheel_state, last_wheel_state);

    if (wheel_state == 0) return 0; // no current press

    int prev1, prev2, next1, next2;

    switch (wheel_state) {
        case 2: prev1 = 8; prev2 = 9; next1 = 3; next2 = 4; break;
        case 3: prev1 = 2; prev2 = 2; next1 = 4; next2 = 5; break;
        case 4: prev1 = 2; prev2 = 3; next1 = 5; next2 = 6; break;
        case 5: prev1 = 3; prev2 = 4; next1 = 6; next2 = 7; break;
        case 6: prev1 = 4; prev2 = 5; next1 = 7; next2 = 8; break;
        case 7: prev1 = 5; prev2 = 6; next1 = 8; next2 = 9; break;
        case 8: prev1 = 6; prev2 = 7; next1 = 9; next2 = 2; break;
        case 9: prev1 = 7; prev2 = 8; next1 = 2; next2 = 3; break;
        default: return 0;
    }

    if (last_wheel_state == prev1 || last_wheel_state == prev2) return 1;
    if (last_wheel_state == next1 || last_wheel_state == next2) return -1;

    if (wheel_state != last_wheel_state) {
        last_wheel_pressed_time = millis();
        last_wheel_state = wheel_state;
    }

    return 0;
}

char Touch_Wheel::swipe() {
    int swipe_state = 0;
    bool last_button_swipe_state = false;

    for (int i = 10; i <= 13; i++) {
        sensors[i-1]->update();
        if (sensors[i-1]->pressed()) {
            swipe_state = i;
            break;
        }
    }

    // Serial.printf("button pressed within HOLD_MS: %d, swipe_state: %d              ", sensors[0]->pressed() && millis() - last_button_pressed_time < SWIPE_MS, swipe_state);

    if (millis() - last_button_pressed_time < SWIPE_MS) {
        switch (swipe_state)
        {
        case 10:
            return 'U';
        case 11:
            return 'R';
        case 12:
            return 'D';
        case 13:
            return 'L';
        }
    }
    return '\0';
}

bool Touch_Wheel::button() {
    sensors[0]->update();
    bool current_state = sensors[0]->pressed();
    unsigned long current = millis();

    // Serial.printf("button state: %d, time_in_state: %d              ", current_state, current - last_button_pressed_time);

    if (current_state) {
        if (!last_button_state) {
            // Button just pressed
            last_button_pressed_time = current;
        } else if (current - last_button_pressed_time >= HOLD_MS) {
            return true;
        }
        last_button_state = true;
    } else {
        last_button_state = false;
    }

    return false;
}