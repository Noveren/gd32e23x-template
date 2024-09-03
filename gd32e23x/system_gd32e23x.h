#ifndef __SYSTEM_GD32E23X_H_
#define __SYSTEM_GD32E23X_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern uint32_t SystemCoreClock;

/* function declarations */
/* initialize the system and update the SystemCoreClock variable */
// extern void SystemInit(void);
/* update the SystemCoreClock with current core clock retrieved from cpu registers */
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif