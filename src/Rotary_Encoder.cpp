#include <Arduino.h>
#include "Rotary_Encoder.h"

Rotary_Encoder::Rotary_Encoder(uint8_t clk_pin, uint8_t dt_pin, uint8_t sw_pin) {
    this->clk_pin = clk_pin;
    this->dt_pin = dt_pin;
    this->sw_pin = sw_pin;
    this->last_clk_state = HIGH;
    this->last_button_time = 0;
}

void Rotary_Encoder::init() {
    pinMode(clk_pin, INPUT_PULLUP);
    pinMode(dt_pin, INPUT_PULLUP);
    pinMode(sw_pin, INPUT_PULLUP);
    last_clk_state = digitalRead(clk_pin);
}

// Returns:
//  1  -> clockwise
// -1  -> counterclockwise
//  0  -> no movement
int8_t Rotary_Encoder::read_encoder() {
    int clk_state = digitalRead(clk_pin);
    int dt_state = digitalRead(dt_pin);
    int result = 0;

    if (clk_state != last_clk_state && clk_state == LOW) {
        result = (dt_state != clk_state) ? 1 : -1;
    }

    last_clk_state = clk_state;
    return result;
}