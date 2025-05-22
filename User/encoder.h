#ifndef ENCODER_H_
#define ENCODER_H_

typedef enum encoderClickType_e {
    ENCODER_CLICK,
    ENCODER_LONG_CLICK
} encoderClickType_t;

typedef void (*encoderRotationCallback)(int);
typedef void (*encoderClickCallback)(encoderClickType_t);

void encoderInit(void);
void encoderSetRotationCallback(encoderRotationCallback callback);
void encoderSetClickCallback(encoderClickCallback callback);

#endif
