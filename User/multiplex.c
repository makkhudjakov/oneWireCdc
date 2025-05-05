#include <stdint.h>
#include <stdbool.h>
#include "debug.h"
#include "multiplex.h"

void multiplexInit(void) {
    GPIO_InitTypeDef        GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

void multiplexSetChannel(multiplexChannell_t channel) {
    switch (channel) {
        case MULTIPLEX_CHANNEL_1:
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
            GPIO_SetBits(GPIOB, GPIO_Pin_11);
            break;
        case MULTIPLEX_CHANNEL_2:
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            GPIO_ResetBits(GPIOB, GPIO_Pin_1);
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
            GPIO_SetBits(GPIOB, GPIO_Pin_11);
            break;
        case MULTIPLEX_CHANNEL_3:
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);
            GPIO_SetBits(GPIOB, GPIO_Pin_11);
            break;
        case MULTIPLEX_CHANNEL_4:
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
            GPIO_SetBits(GPIOB, GPIO_Pin_1);
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
            GPIO_ResetBits(GPIOB, GPIO_Pin_11);
            break;
        default:
            break;
    }
}
