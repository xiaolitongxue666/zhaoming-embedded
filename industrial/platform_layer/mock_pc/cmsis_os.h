/* SPDX-License-Identifier: MIT */
/**
  ******************************************************************************
  * @file    cmsis_os.h (PC mock)
  * @brief   PC 端 CMSIS-RTOS v2 最小子集 mock, 让 platform_layer 用到的
  *          osMutex* + osWaitForever + osStatus_t 在 PC 上跑得动.
  *
  * @details 真机上 cmsis_os.h 由 ARM 官方 / FreeRTOS-Plus-CMSIS / RT-Thread CMSIS
  *          兼容层提供. PC 这一份只 cover platform_i2c.c 真正调到的接口:
  *            osMutexNew / osMutexAcquire / osMutexRelease / osMutexDelete
  *            osMutexAttr_t + osMutexPrioInherit + osWaitForever + osStatus_t
  *          其余 thread / queue / semaphore 一律不抄, 保证 mock 边界足够小.
  *
  *          osMutex 用 pthread_mutex_t * 实现, osMutexPrioInherit 标志映射成
  *          pthread mutex 的 PTHREAD_PRIO_INHERIT 协议. PC 上没有真实优先级
  *          反转问题, 留这一行保证语义自洽.
  ******************************************************************************
  */
#ifndef CMSIS_OS_H_PC_MOCK_
#define CMSIS_OS_H_PC_MOCK_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define osWaitForever       0xFFFFFFFFU

#define osMutexRecursive    0x00000001U
#define osMutexPrioInherit  0x00000002U
#define osMutexRobust       0x00000008U

typedef enum {
    osOK            =  0,
    osError         = -1,
    osErrorTimeout  = -2,
    osErrorResource = -3,
    osErrorParameter= -4,
    osErrorNoMemory = -5,
    osErrorISR      = -6,
    osStatusReserved= 0x7FFFFFFF
} osStatus_t;

typedef void *osMutexId_t;

typedef struct {
    const char *name;
    uint32_t    attr_bits;
    void       *cb_mem;
    uint32_t    cb_size;
} osMutexAttr_t;

osMutexId_t osMutexNew    (const osMutexAttr_t *attr);
osStatus_t  osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout);
osStatus_t  osMutexRelease(osMutexId_t mutex_id);
osStatus_t  osMutexDelete (osMutexId_t mutex_id);

#ifdef __cplusplus
}
#endif

#endif /* CMSIS_OS_H_PC_MOCK_ */
