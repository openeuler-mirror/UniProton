#include "iot_ota_test.h"

#include "iot_ota_test_common.h"

#if defined(OS_SUPPORT_OTA)
int iot_ota_test_run_ota(void);
#endif

#if defined(OS_SUPPORT_LWM2M)
int iot_ota_test_run_lwm2m(void);
#endif

#if defined(OS_SUPPORT_MQTT)
int iot_ota_test_run_mqtt(void);
#endif

#if defined(OS_SUPPORT_OTA) && defined(OS_SUPPORT_LWM2M)
int iot_ota_test_run_lwm2m_ota(void);
#endif

#if defined(OS_SUPPORT_OTA) && defined(OS_SUPPORT_MQTT)
int iot_ota_test_run_mqtt_ota(void);
#endif

#if defined(OS_SUPPORT_NB_IOT)
int iot_ota_test_run_nb_iot(void);
#endif

void iot_ota_test(void)
{
    int failures = 0;

    iot_ota_test_log("start UniProton OTA/connectivity tests");

#if defined(OS_SUPPORT_NB_IOT)
    failures += iot_ota_test_run_nb_iot();
#endif
#if defined(OS_SUPPORT_OTA)
    failures += iot_ota_test_run_ota();
#endif
#if defined(OS_SUPPORT_LWM2M)
    failures += iot_ota_test_run_lwm2m();
#endif
#if defined(OS_SUPPORT_MQTT)
    failures += iot_ota_test_run_mqtt();
#endif
#if defined(OS_SUPPORT_OTA) && defined(OS_SUPPORT_LWM2M)
    failures += iot_ota_test_run_lwm2m_ota();
#endif
#if defined(OS_SUPPORT_OTA) && defined(OS_SUPPORT_MQTT)
    failures += iot_ota_test_run_mqtt_ota();
#endif
    if (failures == 0) {
        iot_ota_test_log("ALL TESTS PASSED");
    } else {
        iot_ota_test_log("TESTS FAILED failures=%d", failures);
    }
}
