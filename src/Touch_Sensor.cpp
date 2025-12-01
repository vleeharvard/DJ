#include <Arduino.h>
#include "Touch_Sensor.h"

Touch_Sensor::Touch_Sensor(int pin, uint16_t sensitivity, uint16_t debounce_ms) {
    this->pin = pin;
    this->sens = sensitivity;
    this->deb_ms = debounce_ms;
}

void Touch_Sensor::init() {
    baseline = touchRead(pin);
    last_change = millis();
    state = false;
    stable_state = false;
}

void Touch_Sensor::update() {
    uint16_t reading = touchRead(pin);

    baseline = (baseline * 0.995f) + (reading * 0.005f);

    bool new_state = (reading < baseline - sens);

    if (new_state != state) {
        state = new_state;
        last_change = millis();
    }

    if (millis() - last_change > deb_ms) {
        stable_state = state;
    }
}

bool Touch_Sensor::pressed() {
    return stable_state;
}

bool Touch_Sensor::released() {
    return !stable_state;
}