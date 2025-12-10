#include <Arduino.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- Custom libraries ---
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "Touch_Sensor.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "Touch_Sensor.h"

Touch_Sensor* touch_sensors[13];

void setup() {
    Serial.begin(57600);
    Serial.printf("\n\n\nStarting Capacitive Wheel...\n");

    for (int i = 1; i <= 13; i++) {
        if (i == 1) {
            touch_sensors[i-1] = new Touch_Sensor(i, 30000, 60000);
        } else if (i <= 9) {
            touch_sensors[i-1] = new Touch_Sensor(i, 30000, 60000);
        } else {
            touch_sensors[i-1] = new Touch_Sensor(i, 20000, 120000);
        }
        touch_sensors[i-1]->init();
    }
}

void loop() {
    char pressedButtons[640];
    int pos = 0;
    pressedButtons[0] = '\0';

    for (int i = 0; i < 13; i++) {
        int a = touch_sensors[i]->update();
        if (touch_sensors[i]->pressed()) {          
          if (i < 9) {
                pressedButtons[pos++] = '1' + i;       // 1-9
            } else {
                pressedButtons[pos++] = '1';           // 10-13 tens digit
                pressedButtons[pos++] = '0' + (i + 1 - 10); // ones digit
            }
        }
    }
    pressedButtons[pos] = '\0';

    Serial.println(pressedButtons);
}
