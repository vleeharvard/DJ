#include <Arduino.h>

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

class Rotary_Encoder {
    public:
    explicit Rotary_Encoder(uint8_t clk_pin, uint8_t dt_pin, uint8_t sw_pin);

    void init();
    int8_t read_encoder();
    bool read_button();

    private:
    uint8_t clk_pin;
    uint8_t dt_pin;
    uint8_t sw_pin;

    int8_t last_clk_state = HIGH;
    uint64_t last_button_time = 0;
};


#endif // ROTARY_ENCODER_H