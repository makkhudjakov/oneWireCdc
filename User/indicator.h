#ifndef INDICATOR_H_
#define INDICATOR_H_

#include <stdint.h>
#include "motorControl.h"

void indicatorInit();
void indicateType(motorControlType_t type);
void indicateChannel(motorControlChannel_t channel);
void indicateThrottle(uint16_t throttle);

#endif
