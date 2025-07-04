#ifndef MULTIPLEX_H_
#define MULTIPLEX_H_

typedef enum multiplexChannel_e {
    MULTIPLEX_CHANNEL_1,
    MULTIPLEX_CHANNEL_2,
    MULTIPLEX_CHANNEL_3,
    MULTIPLEX_CHANNEL_4,
    MULTIPLEX_CHANNEL_NONE,
} multiplexChannel_t;

void multiplexInit(void);
void multiplexSetChannel(multiplexChannel_t channel);

#endif
