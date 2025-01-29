#include <stdint.h>
#include <stdbool.h>
#include "debug.h"
#include "dshot.h"

/* Private variables */
#define Dshot300PeriodTicks 160
#define Dshot300T0HTicks 56
#define Dshot300T1HTicks 112

#define FRAME_LEN 16U
static uint16_t buf[FRAME_LEN + 1];

static uint16_t inBufDuty[FRAME_LEN];

static inputFrameCallback inputCallback;
static bool outputEnabled = false;

static void TIM2_PWMOut_Init(u16 arr, u16 psc, u16 ccp);
static void TIM2_DMAOut_Init(u32 ppadr, u32 memadr, u16 bufsize);
static void TIM2_PWMIn_Init(u16 arr, u16 psc, u16 ccp);
static void TIM2_DMAIn_Init(u32 ppadr, u32 memadr, u16 bufsize);
void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
static void enableOut(void);
static void disableOut(void);
static void setOut(void);
static void enableIn(void);
static void disableIn(void);


static void TIM2_PWMOut_Init(u16 arr, u16 psc, u16 ccp)
{
    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_DeInit(TIM2);
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
}

static void TIM2_DMAOut_Init(u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

    DMA_Cmd(DMA1_Channel5, ENABLE);
}

static void GPIO_InConfig() {
    GPIO_InitTypeDef        GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}

static void GPIO_OutConfig() {
    GPIO_InitTypeDef        GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}

static void TIM2_PWMIn_Init(u16 arr, u16 psc, u16 ccp)
{
    TIM_ICInitTypeDef       TIM_ICInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_DeInit(TIM2);
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x08;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

    TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
}

static void TIM2_DMAIn_Init(uint32_t ppadr2, uint32_t memadr2, uint16_t bufsize2)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr2;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr2;
    DMA_InitStructure.DMA_BufferSize = bufsize2;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_EnableIRQ(DMA1_Channel7_IRQn);

    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    DMA_Cmd(DMA1_Channel7, ENABLE);
}


void DMA1_Channel5_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET) {
        GPIO_InConfig();
        disableOut();
        setIn();
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
}

void DMA1_Channel7_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET) {
        uint16_t value = 0;
        for(uint8_t i = 0; i < FRAME_LEN; i++) {
            if(inBufDuty[FRAME_LEN - 1 - i] > (Dshot300T1HTicks - 5) && inBufDuty[FRAME_LEN - 1 - i] < (Dshot300T1HTicks + 5)) {
                value |=  1 << i;
            }
        }
        inputCallback(value);
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        DMA_SetCurrDataCounter(DMA1_Channel7, FRAME_LEN);
    }
}

static void enableOut(void) {
    TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    outputEnabled = true;
}

static void disableOut(void) {
    DMA_DeInit(DMA1_Channel5);
    TIM_DeInit(TIM2);
    outputEnabled = false;
}

static void setOut(void) {
    TIM2_PWMOut_Init(Dshot300PeriodTicks, 1, 0);
    TIM2_DMAOut_Init((uint32_t)(&TIM2->CH1CVR), (uint32_t)buf, FRAME_LEN + 1);
}


static void enableIn(void) {
    TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

static void disableIn(void) {
    DMA_DeInit(DMA1_Channel7);
    TIM_DeInit(TIM2);
}

void setIn(void) {
    TIM2_PWMIn_Init(Dshot300PeriodTicks * 2, 1, 0);
    TIM2_DMAIn_Init((uint32_t)(&TIM2->CH2CVR), (uint32_t)inBufDuty, FRAME_LEN);
    enableIn();
}

void setCallback(inputFrameCallback callback) {
    inputCallback = callback;
}

void sendFrame(uint16_t frame) {
    while(outputEnabled) {
        asm("nop");
    }
    for(uint8_t i = 0; i < FRAME_LEN; i++) {
        if(frame & (1 << (FRAME_LEN - 1 - i))) {
            buf[i] = Dshot300T1HTicks;
        }
        else {
            buf[i] = Dshot300T0HTicks;
        }
    }
    setOut();
    GPIO_OutConfig();
    enableOut();
}
