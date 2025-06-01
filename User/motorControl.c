#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "encoder.h"
#include "multiplex.h"
#include "indicator.h"
#include "motorControl.h"


static uint32_t permissionTimer;
static const uint32_t permissionTimeout = 1000;
static bool workPermition;

static volatile motorControlType_t type;
static volatile motorControlChannel_t channel;
static volatile uint16_t throttle;

static bool indicatorUpdateType;
static bool indicatorUpdateChannel;
static bool indicatorUpdateThrottle;

void rotationCallback(int delta) {
    if(workPermition) {
        int16_t new_throttle = (int)throttle + delta;
        throttle = (new_throttle < 0) ? 0 : (new_throttle > 100) ? 100 : (uint16_t)new_throttle;
        indicatorUpdateThrottle = true;
    }
}

void switchType() {
    if(type == MOTOR_CONTROL_DSHOT) {
        type = MOTOR_CONTROL_PWM;
    }
    else {
        type = MOTOR_CONTROL_DSHOT;
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

void motorControlInit() {
    type = MOTOR_CONTROL_DSHOT;
    channel = MOTOR_CONTROL_CHANNEL_1;
    throttle = 0;
    workPermition = false;
    permissionTimer = millisecondsGet();
    indicatorUpdateType = true;
    indicatorUpdateChannel = true;
    indicatorUpdateThrottle = true;

    encoderInit();
    encoderSetRotationCallback(&rotationCallback);
    encoderSetClickCallback(&clickCallback);

    indicatorInit();
}

void motorControlDisable() {
    workPermition = false;
    permissionTimer = millisecondsGet();
}

void motorControlTask() {
    uint32_t now = millisecondsGet();
    if(now - permissionTimer > permissionTimeout) {
        workPermition = true;
    }

    if(indicatorUpdateType == true) {
        indicateType(type);
        indicatorUpdateType = false;
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
