/**
 * @file    device_config.h
 * @brief   Flash-based device configuration (last flash page).
 *
 * The device config lives in the last 2KB page of flash (0x0803F800) on
 * the STM32F103RC.  Both the OpenBLT bootloader and the application read
 * from this address at boot to obtain the Modbus slave ID.
 *
 * If the page is unprogrammed (0xFF) or the magic word is invalid, the
 * compile-time default DEVICE_CONFIG_DEFAULT_SLAVE_ID is used.
 */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* -----------------------------------------------------------------------
 * Flash page address — STM32F103RC (256KB flash, 2KB pages)
 * ----------------------------------------------------------------------- */
#define DEVICE_CONFIG_FLASH_ADDR      0x0803F800U
#define DEVICE_CONFIG_PAGE_SIZE       0x800U        /* 2KB */

/* Magic word — must match for config to be considered valid */
#define DEVICE_CONFIG_MAGIC           0xA5C0CF16U

/* Compile-time default slave ID.
 * In the bootloader context, falls back to BOOT_COM_MBRTU_NODE_ID from
 * blt_conf.h.  In the app context, falls back to MODBUS_SLAVE_ID from
 * axacontrol_config.h.  Can also override with -D at build time. */
#ifndef DEVICE_CONFIG_DEFAULT_SLAVE_ID
  #ifdef BOOT_COM_MBRTU_NODE_ID
    #define DEVICE_CONFIG_DEFAULT_SLAVE_ID  BOOT_COM_MBRTU_NODE_ID
  #else
    #define DEVICE_CONFIG_DEFAULT_SLAVE_ID  10
  #endif
#endif

/* -----------------------------------------------------------------------
 * On-flash layout
 * ----------------------------------------------------------------------- */
typedef struct {
    uint32_t magic;         /* must equal DEVICE_CONFIG_MAGIC */
    uint8_t  slave_id;      /* Modbus RTU slave ID (1-247) */
    uint8_t  reserved[3];   /* pad to 4-byte alignment */
} device_config_t;

/* -----------------------------------------------------------------------
 * Reader — works in both bootloader and application context
 * ----------------------------------------------------------------------- */
static inline uint8_t device_config_get_slave_id(void)
{
    const device_config_t *cfg = (const device_config_t *)DEVICE_CONFIG_FLASH_ADDR;

    if (cfg->magic == DEVICE_CONFIG_MAGIC &&
        cfg->slave_id >= 1 && cfg->slave_id <= 247) {
        return cfg->slave_id;
    }
    return (uint8_t)DEVICE_CONFIG_DEFAULT_SLAVE_ID;
}

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_CONFIG_H */
