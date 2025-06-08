#ifndef DSHOT_H_
#define DSHOT_H_

typedef void (*dshotInputFrameCallback)(uint16_t);

void dshotSendFrame(uint16_t frame);
uint16_t dshotCreateFrame(uint8_t throttle);
void dshotSetIn(void);
void dshotSetCallback(dshotInputFrameCallback callback);

#endif
