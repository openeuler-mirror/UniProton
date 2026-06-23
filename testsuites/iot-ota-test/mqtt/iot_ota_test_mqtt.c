#include "iot_ota_test_common.h"
#include "iot_ota_test_config.h"

#include "mqtt_demo_client.h"

#if defined(OS_SUPPORT_MQTT)
int iot_ota_test_run_mqtt(void)
{
    int failures = 0;
    int ret;

    ret = mqtt_demo_run(IOT_OTA_HOST_SERVER, IOT_OTA_MQTT_PORT, IOT_OTA_MQTT_CLIENT_ID);
    failures += iot_ota_test_check(ret == 0, "mqtt agenttiny-style bind report command response");
    return failures;
}
#endif
