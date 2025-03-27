#ifndef HP_ESP32SERVO_H
#define HP_ESP32SERVO_H

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

class Servo
{
    public:

        Servo(gpio_num_t gpio, ledc_timer_t timerNum = LEDC_TIMER_0);
        void write(float angle);
        void    setFadingTimeMS(int value) { fFadingTimeMS = value; }

    private:

        ledc_channel_t  fChannel = LEDC_CHANNEL_MAX;
        ledc_timer_t    fTimer = LEDC_TIMER_MAX;
        gpio_num_t      fGpio = GPIO_NUM_NC;

        float           fAngle = 0.0f;
        int             fFadingTimeMS = 0;
};

#endif