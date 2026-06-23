#ifndef IOT_OTA_TEST_COMMON_H
#define IOT_OTA_TEST_COMMON_H

#include "iot_ota_test.h"

#include <stdint.h>

#define IOT_OTA_TEST_FLASH_SIZE 512U
#define IOT_OTA_TEST_UPDATE_INFO_SIZE 512U
#define IOT_OTA_TEST_PACKAGE_SIZE 256U
#define IOT_OTA_TEST_SOTA_BLOCK_SIZE 64U

extern uint8_t g_iot_ota_test_flash[IOT_OTA_TEST_FLASH_SIZE];
extern uint8_t g_iot_ota_test_update_info[IOT_OTA_TEST_UPDATE_INFO_SIZE];
extern uint8_t g_iot_ota_test_package[IOT_OTA_TEST_PACKAGE_SIZE];
extern uint8_t g_iot_ota_test_downloaded_package[IOT_OTA_TEST_PACKAGE_SIZE];

void iot_ota_test_log(const char *fmt, ...);
int iot_ota_test_check(int condition, const char *name);
void iot_ota_test_reset_flash(void);
void iot_ota_test_reset_download(void);
uint32_t iot_ota_test_build_package(void);
const uint8_t *iot_ota_test_firmware(void);
uint32_t iot_ota_test_firmware_len(void);
int iot_ota_test_init_sota(void);
int iot_ota_test_run_sota_upgrade(const uint8_t *package, uint32_t package_len);
int iot_ota_test_run_sota_liteos_negative(void);

#endif
