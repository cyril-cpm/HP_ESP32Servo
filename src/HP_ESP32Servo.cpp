#include "HP_ESP32Servo.h"
#include "esp_log.h"

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

static bool fadeFunctionInstalled = false;

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
#if defined(ARDUINO)
    uint32_t maxResolution = 20;//ledc_find_suitable_duty_resolution(80000000, 50);

#elif defined(ESP_PLATFORM)
    uint32_t maxResolution = ledc_find_suitable_duty_resolution(80000000, 50);

#endif

    if (maxResolution)
    {
        ESP_LOGI("DUTY_RES", "Max Duty Resolution is %lu", maxResolution);
        if (maxResolution < LEDC_TIMER_BIT_MAX)
            timerConfig.duty_resolution = static_cast<ledc_timer_bit_t>(maxResolution);
        else
        {
            timerConfig.duty_resolution = static_cast<ledc_timer_bit_t>(LEDC_TIMER_BIT_MAX - 1);
            ESP_LOGI("DUTY_RES", "Max Duty Resolution switched to %d", LEDC_TIMER_BIT_MAX - 1);
        }
    }
    else
    {
        ESP_LOGI("DUTY_RES", "No Duty Resolution is possible");
        return;
    }

    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));
    if (!fadeFunctionInstalled)
    {
        ESP_ERROR_CHECK(ledc_fade_func_install(0));
        fadeFunctionInstalled = true;
    }
}

IRAM_ATTR bool ledcCallback(const ledc_cb_param_t* param, void* userArg)
{
    //ESP_LOGI("LEDC", "LEDC_CAllback");
    if (userArg)
    {
        Servo* servo = reinterpret_cast<Servo*>(userArg);
        servo->fadingCallback();
    }
    return false;
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

    ledc_cbs_t cbStruct;
    cbStruct.fade_cb = &ledcCallback;

    ESP_ERROR_CHECK(ledc_cb_register(LEDC_HIGH_SPEED_MODE, fChannel, &cbStruct, this));

}

void Servo::write(float angle, bool force)
{
    if (force)
    {
        uint32_t newDutyCycle = map(angle, 0.0f, 180.0f, MIN, MAX);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, fChannel, newDutyCycle);

        fCurrentDutyCycle = newDutyCycle;
        fAngle = angle;
    }
    else if (angle != fAngle)
    {
        uint32_t targetDutyCycle = map(angle, 0.0f, 180.0f, MIN, MAX);
        float distance = abs(fAngle - angle);
        fAngle = angle;

        uint32_t steps = 0;

        if (targetDutyCycle > fCurrentDutyCycle)
            steps = targetDutyCycle - fCurrentDutyCycle;
        else
            steps = fCurrentDutyCycle - targetDutyCycle;

        uint32_t timeMs = (distance / fSpeed) * 1000;

        uint32_t nbCycle = timeMs / 20;

        uint32_t fadeScale;
        uint32_t fadeCycleNum;

        if (nbCycle > steps && steps)
        {
            fadeCycleNum = nbCycle / steps;
            fadeScale = 1;
        }
        else if (nbCycle)
        {
            fadeCycleNum = 1;
            fadeScale = steps / nbCycle;
        }
        else
        {
            fadeCycleNum = 1;
            fadeScale = steps;
        }
        
        ESP_LOGI("HP_Servo", "distance %f, speed %f, timeMs %lu", distance, fSpeed, timeMs);
        ESP_LOGI("HP_Servo", "steps %lu, nbCycle %lu", steps, nbCycle);
        ESP_LOGI("HP_Servo", "scale %lu, cycle %lu, targetDuty %lu, currentDuty %lu", fadeScale, fadeCycleNum, targetDutyCycle, fCurrentDutyCycle);
        
        if (fadeScale >= 1024)
        {
            ESP_LOGI("HP_Servo", "scale too big");
            fadeScale = 1023;
        }

        ledc_set_fade_step_and_start(LEDC_HIGH_SPEED_MODE, fChannel, targetDutyCycle, fadeScale, fadeCycleNum, LEDC_FADE_NO_WAIT);
        
        fCurrentDutyCycle = targetDutyCycle;
    }
    else if (fFadingCallback)
        fFadingCallback();
}

void Servo::setSpeed(float speed)
{
    if (speed == 0.0f)
        speed = 0.1f;

    fSpeed = speed;
}

void Servo::setFadingCallback(void (*callback)())
{
    fFadingCallback = callback;
}

void Servo::fadingCallback() const
{
    if (fFadingCallback)
        fFadingCallback();
}