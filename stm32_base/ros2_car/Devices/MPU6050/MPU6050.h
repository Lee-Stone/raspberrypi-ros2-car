#ifndef __MPU6050_H
#define __MPU6050_H

#include "config.h"

#if DEVICE_MPU6050

void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);
int MPU6050_DMP_Init(void);
int MPU6050_DMP_ReadEuler(float *pitchDeg, float *rollDeg, float *yawDeg);
float MPU6050_DMP_GetROSGyroz(void);

#endif // DEVICE_MPU6050

#endif
