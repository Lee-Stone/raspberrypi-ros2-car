#ifndef TB6612_H_
#define TB6612_H_

#include "config.h"

#if DEVICE_TB6612

void TB6612_Init(void);
void TB6612_Motor(int16_t motora, int16_t motorb);

#endif

#endif
