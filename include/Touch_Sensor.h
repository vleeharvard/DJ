#include <Arduino.h>

#ifndef TOUCH_SENSOR_FILE_H
#define TOUCH_SENSOR_FILE_H

class Touch_Sensor {
    private:
    uint8_t pin;
    uint16_t sens;
    uint16_t deb_ms;

    uint32_t last_change;
    float baseline = 0;
    bool state = false; // Instantaneous state
    bool stable_state = false; // Debounced state

    public:
    explicit Touch_Sensor(int pin, uint16_t sensitivity, uint16_t debounce_ms);
    void init();
    void update();
    bool pressed();
    bool released();
};

#endif // CHANNEL_FILE_H
