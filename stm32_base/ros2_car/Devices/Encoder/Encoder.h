#ifndef ENCODER_H_
#define ENCODER_H_

#include "config.h"

#if DEVICE_ENCODER

void Encoder_Init(void);
void Encoder_Read(float *left, float *right);

#endif

#endif