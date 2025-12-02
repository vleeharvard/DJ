#include <Arduino.h>

#ifndef TOUCH_SENSOR_FILE_H
#define TOUCH_SENSOR_FILE_H

class Touch_Sensor {
    private:
    uint8_t pin;
    uint16_t sens;

    float baseline = 0;
    bool state = false; // Instantaneous state

    public:
    explicit Touch_Sensor(int pin, uint16_t sensitivity);
    void init();
    uint16_t update();
    bool pressed();
    bool released();
};

#endif // CHANNEL_FILE_H
