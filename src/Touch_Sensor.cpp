#include <Arduino.h>
#include "Touch_Sensor.h"

Touch_Sensor::Touch_Sensor(int pin, uint32_t lower, uint32_t upper) {
    this->pin = pin;
    this->lower = lower;
    this->upper = upper;
}

void Touch_Sensor::init() {
    state = false;
}

uint32_t Touch_Sensor::update() {
    uint32_t reading = touchRead(pin);
    state = (reading < lower || reading > upper);

    return reading;
}

bool Touch_Sensor::pressed() {
    return state;
}

bool Touch_Sensor::released() {
    return !state;
}