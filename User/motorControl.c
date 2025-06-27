#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "encoder.h"
#include "multiplex.h"
#include "indicator.h"
#include "motorControl.h"
#include "dshot.h"
#include "pwm.h"

static uint32_t permissionTimer;
static const uint32_t permissionTimeout = 1000;
static bool workPermition;

static volatile motorControlType_t type;
static volatile motorControlChannel_t channel;
static volatile uint16_t throttle;

static bool indicatorUpdateType;
static bool indicatorUpdateChannel;
static bool indicatorUpdateThrottle;

void resetThrottle() {
    throttle = 0;
    indicatorUpdateThrottle = true;
}

void rotationCallback(int delta) {
    if(workPermition && type != MOTOR_CONTROL_DISABLE) {
        int16_t new_throttle = (int)throttle + delta;
        throttle = (new_throttle < 0) ? 0 : (new_throttle > 100) ? 100 : (uint16_t)new_throttle;
        indicatorUpdateThrottle = true;
    }
}

void switchType() {
    if(type == MOTOR_CONTROL_DSHOT) {
        type = MOTOR_CONTROL_PWM;
    }
    else if (type == MOTOR_CONTROL_DISABLE) {
        type = MOTOR_CONTROL_DSHOT;
    }
    else {
        type = MOTOR_CONTROL_DISABLE;
    }
}

void switchChannel() {
    uint8_t new_channel = channel + 1;
    if(new_channel > MOTOR_CONTROL_CHANNEL_4) {
        channel = MOTOR_CONTROL_CHANNEL_1;
    }
    else {
        channel = new_channel;
    }
}

void clickCallback(encoderClickType_t clickType) {
    if(workPermition) {
        resetThrottle();
        if(clickType == ENCODER_LONG_CLICK) {
            switchType();
            indicatorUpdateType = true;
        }
        else {
            switchChannel();
            indicatorUpdateChannel = true;
        }
    }
}

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void configTimer() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct = {0};
    TIM_TimeBaseInitStruct.TIM_Period = 999;
    TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct = {0};
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void startTimer() {
    TIM_Cmd(TIM3, ENABLE);
}

void stopTimer() {
    TIM_Cmd(TIM3, DISABLE);
}

void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if(workPermition) {
            multiplexSetChannel(channel);
            if(type == MOTOR_CONTROL_DSHOT) {
                uint16_t frame = dshotCreateFrame(throttle);
                dshotSendFrame(frame);
            }
            else if(type == MOTOR_CONTROL_PWM) {
                uint16_t duty = pwmThrottleToDuty(throttle);
                pwmSetDuty(duty);
            }
        }
    }
}

void motorControlInit() {
    type = MOTOR_CONTROL_DISABLE;
    channel = MOTOR_CONTROL_CHANNEL_1;
    throttle = 0;
    workPermition = false;
    permissionTimer = millisecondsGet();

    encoderInit();
    encoderSetRotationCallback(&rotationCallback);
    encoderSetClickCallback(&clickCallback);

    indicatorInit();

    indicatorUpdateType = false;
    indicatorUpdateChannel = false;
    indicatorUpdateThrottle = false;

    indicateType(type);
    indicateChannel(channel);
    indicateThrottle(throttle);

    configTimer();
}

void motorControlDisable() {
    if(workPermition) {
        resetThrottle();
        type = MOTOR_CONTROL_DISABLE;
        indicatorUpdateType = true;
        stopTimer();
        workPermition = false;
        dshotSetIn();
        Delay_Ms(150);
    }
    permissionTimer = millisecondsGet();
}

void motorControlTask() {
    uint32_t now = millisecondsGet();
    if(now - permissionTimer > permissionTimeout) {
        workPermition = true;
        startTimer();
    }

    if(indicatorUpdateType == true) {
        indicateType(type);
        indicatorUpdateType = false;
        if(type == MOTOR_CONTROL_PWM) {
            pwmEnable();
        }
    }
    if(indicatorUpdateChannel == true) {
        indicateChannel(channel);
        indicatorUpdateChannel = false;
    }
    if(indicatorUpdateThrottle == true) {
        indicateThrottle(throttle);
        indicatorUpdateThrottle = false;
    }
}
