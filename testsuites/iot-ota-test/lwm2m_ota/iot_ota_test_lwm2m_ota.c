#include "iot_ota_test_common.h"
#include "iot_ota_test_config.h"

#include <string.h>

#include "lwm2m_ota_client.h"

#if defined(OS_SUPPORT_OTA) && defined(OS_SUPPORT_LWM2M)
int iot_ota_test_run_lwm2m_ota(void)
{
    uint32_t local_package_len;
    uint32_t remote_package_len = 0;
    int failures = 0;
    int ret;

    iot_ota_test_reset_flash();
    iot_ota_test_reset_download();
    local_package_len = iot_ota_test_build_package();

    ret = lwm2m_ota_register_and_receive_package(IOT_OTA_HOST_SERVER, IOT_OTA_LWM2M_PORT,
        IOT_OTA_LWM2M_ENDPOINT, g_iot_ota_test_downloaded_package, IOT_OTA_TEST_PACKAGE_SIZE,
        &remote_package_len);
    failures += iot_ota_test_check(ret == 0, "ota-lwm2m receive package resource");
    failures += iot_ota_test_check(remote_package_len == local_package_len, "ota-lwm2m package size");
    failures += iot_ota_test_check(memcmp(g_iot_ota_test_downloaded_package, g_iot_ota_test_package,
        local_package_len) == 0, "ota-lwm2m package payload");
    ret = iot_ota_test_init_sota();
    failures += iot_ota_test_check(ret == 0, "ota-lwm2m sota_init");
    ret = iot_ota_test_run_sota_upgrade(g_iot_ota_test_downloaded_package, remote_package_len);
    failures += iot_ota_test_check(ret == 0, "ota-lwm2m sota upgrade");
    failures += iot_ota_test_check(memcmp(g_iot_ota_test_flash, iot_ota_test_firmware(),
        iot_ota_test_firmware_len()) == 0, "ota-lwm2m firmware payload");
    return failures;
}
#endif
