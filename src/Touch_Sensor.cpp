#include <Arduino.h>
#include "Touch_Sensor.h"

Touch_Sensor::Touch_Sensor(int pin, uint16_t sensitivity) {
    this->pin = pin;
    this->sens = sensitivity;
}

void Touch_Sensor::init() {
    baseline = touchRead(pin);
    state = false;
}

uint16_t Touch_Sensor::update() {
    uint16_t reading = touchRead(pin);
    state = (reading > sens);

    return reading;
}

bool Touch_Sensor::pressed() {
    return state;
}

bool Touch_Sensor::released() {
    return !state;
}