#ifndef HP_ESP32SERVO_H
#define HP_ESP32SERVO_H

#define MIN   26214
#define MAX  131072

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

class Servo
{
    public:

        Servo(gpio_num_t gpio, ledc_timer_t timerNum = LEDC_TIMER_0);
        void write(float angle, bool force = false);
        void    setSpeed(float speed);
        void    fadingCallback() const;
        void    setFadingCallback(void (*callback)());
        float   getAngle() const { return fAngle; }

    private:

        ledc_channel_t  fChannel = LEDC_CHANNEL_MAX;
        ledc_timer_t    fTimer = LEDC_TIMER_MAX;
        gpio_num_t      fGpio = GPIO_NUM_NC;

        float           fAngle = 0.0f; // degrees
        float           fSpeed = 5.0f; // degrees / seconds
        void            (*fFadingCallback)() = nullptr;

        uint32_t        fCurrentDutyCycle = MIN;
};

#endif