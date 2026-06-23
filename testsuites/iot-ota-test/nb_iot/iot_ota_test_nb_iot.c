#include "iot_ota_test_common.h"
#include "iot_ota_test_config.h"

#include <string.h>

#include "nb_iot/los_nb_api.h"

#if defined(OS_SUPPORT_NB_IOT)
int iot_ota_test_run_nb_iot(void)
{
    int failures = 0;
    int ret;

    ret = los_nb_init((const int8_t *)IOT_OTA_HOST_SERVER, (const int8_t *)IOT_OTA_LWM2M_PORT_STRING, NULL);
    failures += iot_ota_test_check(ret == 0, "nb_iot init bearer");
    ret = los_nb_report("feature:nb_iot", (int)strlen("feature:nb_iot"));
    failures += iot_ota_test_check(ret == 0, "nb_iot report payload");
    (void)los_nb_deinit();
    return failures;
}
#endif
