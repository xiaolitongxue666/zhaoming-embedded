/* Minimal syntax stub for fsl_lpi2c.h */
#ifndef FSL_LPI2C_H
#define FSL_LPI2C_H

#include <stdint.h>
#include "fsl_gpio.h"

typedef struct { int dummy; } LPI2C_Type;
#define LPI2C1 ((LPI2C_Type *)0x40104000UL)

typedef enum {
	kLPI2C_Read = 0,
	kLPI2C_Write,
} lpi2c_direction_t;

typedef struct {
	uint8_t           slaveAddress;
	lpi2c_direction_t direction;
	uint32_t          subaddress;
	uint8_t           subaddressSize;
	uint8_t          *data;
	uint32_t          dataSize;
	uint32_t          flags;
} lpi2c_master_transfer_t;

status_t LPI2C_MasterTransferBlocking(LPI2C_Type *base,
                                      lpi2c_master_transfer_t *xfer);

#endif
