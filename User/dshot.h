#ifndef DSHOT_H_
#define DSHOT_H_

typedef void (*inputFrameCallback)(uint16_t);

void sendFrame(uint16_t frame);
void setIn(void);
void setCallback(inputFrameCallback callback);

#endif
