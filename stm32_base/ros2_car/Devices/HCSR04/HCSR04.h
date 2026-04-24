#ifndef __HCSR04_H
#define __HCSR04_H

#include "config.h"

#if DEVICE_HCSR04

#include <stdint.h>

void    HCSR04_Init(void);
int16_t HCSR04_Read(void);

#endif /* DEVICE_HCSR04 */

#endif /* __HCSR04_H */
