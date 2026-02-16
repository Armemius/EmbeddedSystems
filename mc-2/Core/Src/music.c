#include "music.h"

#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"

void init_music()
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	stop_note();
}

void play_note(uint16_t freq)
{
    uint32_t arr = 1000000 / freq;

    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr / 2); // 50%
}

void stop_note()
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

void play_amogus()
{
    uint32_t frequencies[] = {
        262, 311, 349, 370, 349, 311, 262, 233, 294, 262,
        262, 311, 349, 370, 349, 311, 370, 370, 349, 311, 370, 349, 311, 262
    };
    uint32_t durations_ms[] = {
        250, 250, 250, 250, 250, 250, 750, 125, 125,
        1000, 250, 250, 250, 250, 250, 250, 750, 125, 125, 125, 125, 125, 125, 125
    };
    size_t num_notes = sizeof(frequencies) / sizeof(frequencies[0]);

    for (size_t i = 0; i < num_notes; i++) {
        play_note(frequencies[i]);
        HAL_Delay(100);
        stop_note();
        HAL_Delay(durations_ms[i]);
    }
    stop_note();
}
