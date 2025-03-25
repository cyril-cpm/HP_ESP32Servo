#include "HP_ESP32Servo.h"
#include "esp_log.h"

#define MIN   26214
#define MAX  131072

static uint8_t initialisedTimer = 0b0000;

static ledc_channel_t currentChannel = LEDC_CHANNEL_0;

static bool isTimerInitialised(ledc_timer_t timerNum)
{
    return((1 << timerNum) & initialisedTimer);
}

static uint32_t map(float x, float in_min, float in_max, uint32_t out_min, uint32_t out_max)
{
    return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

static void initTimer(ledc_timer_t timerNum = LEDC_TIMER_0, ledc_mode_t speedMode = LEDC_HIGH_SPEED_MODE,
    uint32_t freq = 50, ledc_clk_cfg_t clockConfig = LEDC_USE_APB_CLK)
{
    if (isTimerInitialised(timerNum))
    {
        ESP_LOGI("TIMER", "timer %d is already initialised", timerNum);
        return;
    }

    ledc_timer_config_t timerConfig = {};

    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.freq_hz = freq;
    timerConfig.clk_cfg = LEDC_USE_APB_CLK;
    uint32_t maxResolution = ledc_find_suitable_duty_resolution(80000000, 50);

    if (maxResolution)
    {
        ESP_LOGI("DUTY_RES", "Max Duty Resolution is %lu", maxResolution);
        if (maxResolution < LEDC_TIMER_BIT_MAX)
            timerConfig.duty_resolution = static_cast<ledc_timer_bit_t>(maxResolution);
        else
            timerConfig.duty_resolution = static_cast<ledc_timer_bit_t>(LEDC_TIMER_BIT_MAX - 1);
    }
    else
    {
        ESP_LOGI("DUTY_RES", "No Duty Resolution is possible");
        return;
    }

    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));
}

Servo::Servo(gpio_num_t gpio, ledc_timer_t timerNum)
{
    if (currentChannel >= LEDC_CHANNEL_MAX)
    {
        ESP_LOGI("CHANNEL", "out of channel");
        return;
    }

    if (!isTimerInitialised(timerNum))
    {
        initTimer(timerNum);
    }

    fChannel = currentChannel;
    fTimer = timerNum;
    fGpio = gpio;

    currentChannel = static_cast<ledc_channel_t>(currentChannel + 1);

    ledc_channel_config_t channelConfig = {};

    channelConfig.channel = fChannel;
    channelConfig.gpio_num = fGpio;
    channelConfig.timer_sel = fTimer;

    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
}

void Servo::write(float angle)
{
    unsigned long value = map(angle, 0.0f, 180.0f, MIN, MAX);
    fAngle = angle;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, fChannel, value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, fChannel));
}