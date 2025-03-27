/*#include "HP_ESP32Servo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Servo DM996(GPIO_NUM_27);
Servo DS3218(GPIO_NUM_12);


extern "C" void app_main()
{
    DM996.write(0.0f);
    DS3218.write(0.0f);

    vTaskDelay(pdMS_TO_TICKS(1000));

    for (float i = 0.0f; i <= 180.0f; i += 0.1f)
    {
        DM996.write(i);
        DS3218.write(i);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    DM996.write(0.0f);
    DS3218.write(0.0f);

    vTaskDelay(pdMS_TO_TICKS(1000));

    for (float i = 0.0f; i <= 180.0f; i += 0.05f)
    {
        DM996.write(i);
        DS3218.write(i);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}*/