#include "iot_ota_test_common.h"
#include "iot_ota_test_config.h"

#include "lwm2m_demo_client.h"

#if defined(OS_SUPPORT_LWM2M)
int iot_ota_test_run_lwm2m(void)
{
    int failures = 0;
    int ret;

    ret = lwm2m_demo_register_and_process(IOT_OTA_HOST_SERVER, IOT_OTA_LWM2M_PORT, IOT_OTA_LWM2M_ENDPOINT);
    failures += iot_ota_test_check(ret == 0, "lwm2m agenttiny-style register read write execute");
    return failures;
}
#endif
