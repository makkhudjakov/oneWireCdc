#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include "multiplex.h"

typedef enum motorControlType_e {
    MOTOR_CONTROL_DISABLE,
    MOTOR_CONTROL_DSHOT,
    MOTOR_CONTROL_PWM,
} motorControlType_t;

typedef enum motorControlChannel_e {
    MOTOR_CONTROL_CHANNEL_1 = MULTIPLEX_CHANNEL_1,
    MOTOR_CONTROL_CHANNEL_2 = MULTIPLEX_CHANNEL_2,
    MOTOR_CONTROL_CHANNEL_3 = MULTIPLEX_CHANNEL_3,
    MOTOR_CONTROL_CHANNEL_4 = MULTIPLEX_CHANNEL_4,
} motorControlChannel_t;

void motorControlInit();
void motorControlEnable();
void motorControlDisable();
void motorControlTask();

#endif
