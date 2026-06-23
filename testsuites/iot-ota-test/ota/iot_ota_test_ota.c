#include "iot_ota_test_common.h"

#include <string.h>

#if defined(OS_SUPPORT_OTA)
int iot_ota_test_run_ota(void)
{
    uint32_t package_len;
    int failures = 0;
    int ret;

    iot_ota_test_reset_flash();
    package_len = iot_ota_test_build_package();
    iot_ota_test_log("ota flow: run LiteOS-style SOTA negative parser/state-machine cases");
    failures += iot_ota_test_run_sota_liteos_negative();
    iot_ota_test_log("ota flow: build package manifest version=V2.0 package_len=%u block_size=%u",
        package_len, IOT_OTA_TEST_SOTA_BLOCK_SIZE);
    ret = iot_ota_test_init_sota();
    failures += iot_ota_test_check(ret == 0, "ota initialize SOTA callbacks and flash adapter");
    iot_ota_test_log("ota flow: notify new version, download package by blocks, execute update");
    ret = iot_ota_test_run_sota_upgrade(g_iot_ota_test_package, package_len);
    failures += iot_ota_test_check(ret == 0, "ota SOTA version notice block download execute update");
    failures += iot_ota_test_check(memcmp(g_iot_ota_test_flash, iot_ota_test_firmware(),
        iot_ota_test_firmware_len()) == 0, "ota firmware image written to flash");
    return failures;
}
#endif
