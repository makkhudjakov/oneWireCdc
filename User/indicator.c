#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "indicator.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

const uint8_t startX = 2;
const uint8_t typeX = 72;
const uint8_t channelX = 72;
const uint8_t throttleX = 72;

const uint8_t typeY = 2;
const uint8_t channelY = 20;
const uint8_t throttleY = 38;

motorControlType_t prevType;
motorControlChannel_t prevChannel;
uint16_t prevThrottle;

#define SSD1306_I2C_ADDR 0x78

// §ª§ß§Ú§è§Ú§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ I2C
void I2C_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure={0};
    I2C_InitTypeDef I2C_InitTSturcture={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    I2C_InitTSturcture.I2C_ClockSpeed = 400000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = SSD1306_I2C_ADDR;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );

    I2C_Cmd( I2C1, ENABLE );
}

// §°§ä§á§â§Ñ§Ó§Ü§Ñ §Õ§Ñ§ß§ß§í§ç §é§Ö§â§Ö§Ù I2C
void I2C_Write(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len) {
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );
    I2C_GenerateSTART( I2C1, ENABLE );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, reg);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    for(uint16_t i=0; i<len; i++) {
        I2C_SendData(I2C1, data[i]);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
}

void ssd1306_Reset(void) {

}
void ssd1306_WriteCommand(uint8_t byte) {
    I2C_Write(SSD1306_I2C_ADDR, 0x00, &byte, 1);
}
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    I2C_Write(SSD1306_I2C_ADDR, 0x40, buffer, buff_size);
}

void HAL_Delay(uint32_t ms) {
    Delay_Ms(ms);
}

void printText(uint16_t x, uint16_t y, char* str) {
    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(str, Font_7x10, White);
    ssd1306_UpdateScreen();
}

void deleteText(uint16_t x, uint16_t y, char* str) {
    size_t len = strlen(str);
    ssd1306_FillRectangle(x, y, x + (Font_7x10.width * len), y + Font_7x10.height, Black);
    ssd1306_UpdateScreen();
}


const char* motorControlTypeToString(motorControlType_t type) {
    static const char* const typeStrings[] = {
        [MOTOR_CONTROL_DSHOT] = "DSHOT",
        [MOTOR_CONTROL_PWM]   = "PWM",
    };
    const char* unknownType = "Unknown";
    const size_t numElements = sizeof(typeStrings) / sizeof(typeStrings[0]);

    if ((unsigned int)type < numElements && typeStrings[type] != NULL) {
        return typeStrings[type];
    }
    return unknownType;
}

const char* motorControlChannelToString(motorControlChannel_t channel) {
    static const char* const channelStrings[] = {
        [MOTOR_CONTROL_CHANNEL_1] = "1",
        [MOTOR_CONTROL_CHANNEL_2] = "2",
        [MOTOR_CONTROL_CHANNEL_3] = "3",
        [MOTOR_CONTROL_CHANNEL_4] = "4",
        [MOTOR_CONTROL_CHANNEL_ALL] = "ALL",
    };
    const char* unknownChannel = "Unknown";
    const size_t numElements = sizeof(channelStrings) / sizeof(channelStrings[0]);

    if ((unsigned int)channel < numElements && channelStrings[channel] != NULL) {
        return channelStrings[channel];
    }
    return unknownChannel;
}

void indicatorInit() {
    I2C_init();
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    char typeStr[] = "Type: ";
    char channelStr[] = "Channel: ";
    char throttleStr[] = "Throttle: ";
    printText(startX, typeY, typeStr);
    printText(startX, channelY, channelStr);
    printText(startX, throttleY, throttleStr);
}

void indicateType(motorControlType_t type) {
    const char* typeStr = motorControlTypeToString(type);
    const char* prevTypeStr = motorControlTypeToString(prevType);

    deleteText(typeX, typeY, (char*)prevTypeStr);
    printText(typeX, typeY, (char*)typeStr);
    prevType = type;
}

void indicateChannel(motorControlChannel_t channel) {
    const char* channelStr = motorControlChannelToString(channel);
    const char* prevChannelStr = motorControlChannelToString(prevChannel);

    deleteText(channelX, channelY, (char*)prevChannelStr);
    printText(channelX, channelY, (char*)channelStr);
    prevChannel = channel;
}

void indicateThrottle(uint16_t throttle) {
    char throttleStr[3] = "";
    char prevThrottleStr[3] = "";
    itoa(throttle, throttleStr, 10);
    itoa(prevThrottle, prevThrottleStr, 10);

    deleteText(throttleX, throttleY, (char*)prevThrottleStr);
    printText(throttleX, throttleY, (char*)throttleStr);
    prevThrottle = throttle;
}
