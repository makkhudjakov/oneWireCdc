#include <stdint.h>
#include "debug.h"
#include "encoder.h"

encoderRotationCallback _rotationCallback;
encoderClickCallback _clickCallback;

void encoderSetClickCallback(encoderClickCallback callback) {
    _clickCallback = callback;
}

void encoderSetRotationCallback(encoderRotationCallback callback) {
    _rotationCallback = callback;
}

static volatile uint8_t prev_state = 0;

void abInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    EXTI_InitTypeDef EXTI_InitStruct = {0};
    NVIC_InitTypeDef NVIC_InitStruct = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource9);

    EXTI_InitStruct.EXTI_Line = EXTI_Line8 | EXTI_Line9;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    prev_state = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) << 1) | GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9);
}

void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void) {
    uint8_t curr_state = 0;
    static const int8_t transitions[] = {0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0};

    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        curr_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9);
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET) {
        curr_state = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) << 1) | 0;
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
    uint8_t index = (prev_state << 2) | curr_state;
    int delta = transitions[index];
    prev_state = curr_state;

    if(_rotationCallback != NULL && delta != 0) {
        _rotationCallback(delta);
    }
}

#define DEBOUNCE_DELAY 50 // 技扼

volatile uint32_t button_press_time = 0;
volatile uint8_t button_pressed = 0;


void buttonInit(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);

    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
        static uint32_t last_interrupt = 0;
        uint32_t now = millisecondsGet();

        // 孝扼找把忘扶快扶我快 忱把快忌快戒忍忘
        if (now - last_interrupt > DEBOUNCE_DELAY) {
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == Bit_RESET) {
                button_press_time = now;
                button_pressed = 1;
            }
            else {
                if (button_pressed) {
                    uint32_t press_duration = now - button_press_time;
                    if (press_duration >= 1000 && _clickCallback != NULL) {
                        _clickCallback(ENCODER_LONG_CLICK);
                    }
                    else {
                        _clickCallback(ENCODER_CLICK);
                    }
                    button_pressed = 0;
                }
            }
            last_interrupt = now;
        }
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}

void encoderInit(void) {
    abInit();
    buttonInit();
}

