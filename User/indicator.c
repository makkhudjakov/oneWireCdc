#include <stdlib.h>
#include "debug.h"
#include "indicator.h"
#include "driver_ssd1306_basic.h"

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
    ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);
    ssd1306_basic_clear();
    char typeStr[] = "Type: ";
    char channelStr[] = "Channel: ";
    char throttleStr[] = "Throttle: ";
    ssd1306_basic_string(startX, typeY, typeStr, strlen(typeStr), 1, SSD1306_FONT_16);
    ssd1306_basic_string(startX, channelY, channelStr, strlen(channelStr), 1, SSD1306_FONT_16);
    ssd1306_basic_string(startX, throttleY, throttleStr, strlen(throttleStr), 1, SSD1306_FONT_16);
}

void indicateType(motorControlType_t type) {
    const char* typeStr = motorControlTypeToString(type);
    const char* prevTypeStr = motorControlTypeToString(prevType);

    ssd1306_basic_string(typeX, typeY, (char*)prevTypeStr, strlen(prevTypeStr), 0, SSD1306_FONT_16);
    ssd1306_basic_string(typeX, typeY, (char*)typeStr, strlen(typeStr), 1, SSD1306_FONT_16);
    prevType = type;
}

void indicateChannel(motorControlChannel_t channel) {
    const char* channelStr = motorControlChannelToString(channel);
    const char* prevChannelStr = motorControlChannelToString(prevChannel);

    ssd1306_basic_string(channelX, channelY, (char*)prevChannelStr, strlen(prevChannelStr), 0, SSD1306_FONT_16);
    ssd1306_basic_string(channelX, channelY, (char*)channelStr, strlen(channelStr), 1, SSD1306_FONT_16);
    prevChannel = channel;
}

void indicateThrottle(uint16_t throttle) {
    char throttleStr[3] = "";
    char prevThrottleStr[3] = "";
    itoa(throttle, throttleStr, 10);
    itoa(prevThrottle, prevThrottleStr, 10);

    ssd1306_basic_string(throttleX, throttleY, (char*)prevThrottleStr, strlen(prevThrottleStr), 0, SSD1306_FONT_16);
    ssd1306_basic_string(throttleX, throttleY, (char*)throttleStr, strlen(throttleStr), 1, SSD1306_FONT_16);
    prevThrottle = throttle;
}
