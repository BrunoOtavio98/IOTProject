/*
 * FreeRTOSHooks.c
 *
 *  Created on: 5 de nov de 2025
 *      Author: BrunoOtavio
 */

#include "FreeRTosConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Idle task memory (required when configSUPPORT_STATIC_ALLOCATION == 1) */
static StaticTask_t xIdleTaskTCB;
static StackType_t  xIdleStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

/* Timer task memory (required when configUSE_TIMERS == 1 and configSUPPORT_STATIC_ALLOCATION == 1) */
#if ( configUSE_TIMERS == 1 )
static StaticTask_t xTimerTaskTCB;
#ifndef configTIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH  configMINIMAL_STACK_SIZE
#endif
static StackType_t xTimerStack[ configTIMER_TASK_STACK_DEPTH ];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
#endif /* configUSE_TIMERS */

#ifdef __cplusplus
}
#endif


