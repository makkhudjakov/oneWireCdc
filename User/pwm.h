#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

void pwmSetDuty(uint16_t duty);
uint16_t pwmThrottleToDuty(uint8_t throttle);
void pwmEnable(void);
void pwmDisable(void);

#endif
