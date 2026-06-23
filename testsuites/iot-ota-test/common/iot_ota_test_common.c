#include "iot_ota_test_common.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(OS_SUPPORT_OTA)
#include "ota/ota_api.h"
#include "sota/sota.h"
#include "sota_hal.h"
#endif

#if defined(OS_SUPPORT_NB_IOT)
#include "nb_iot/los_nb_api.h"
#endif

extern unsigned int PRT_Printf(const char *format, ...);

#define TEST_OTA_FULL_SOFTWARE 0U

#if defined(OS_SUPPORT_OTA)
#define SOTA_BUF_LEN 640U
#define PCP_HEAD 0xFFFEU
#define MSG_NOTIFY_NEW_VER 20U
#define MSG_GET_BLOCK 21U
#define MSG_EXC_UPDATE 23U
#endif

uint8_t g_iot_ota_test_flash[IOT_OTA_TEST_FLASH_SIZE];
uint8_t g_iot_ota_test_update_info[IOT_OTA_TEST_UPDATE_INFO_SIZE];
uint8_t g_iot_ota_test_package[IOT_OTA_TEST_PACKAGE_SIZE];
uint8_t g_iot_ota_test_downloaded_package[IOT_OTA_TEST_PACKAGE_SIZE];

#if defined(OS_SUPPORT_OTA)
static int8_t g_sota_buf[SOTA_BUF_LEN];
#endif

static const uint8_t g_firmware[] =
    "UniProton sd3403 OTA firmware image via LwM2M manifest, MQTT chunks and NB-IoT proxy report."
    " Version two payload for end to end upgrade validation.";

void iot_ota_test_log(const char *fmt, ...)
{
    char str_buf[192];
    va_list list;
    int ret;

    va_start(list, fmt);
    ret = vsnprintf(str_buf, sizeof(str_buf), fmt, list);
    va_end(list);
    if (ret > 0) {
        PRT_Printf("[IOT_OTA_TEST] %s\n", str_buf);
    }
}

int iot_ota_test_check(int condition, const char *name)
{
    if (condition) {
        iot_ota_test_log("[PASS] %s", name);
        return 0;
    }

    iot_ota_test_log("[FAIL] %s", name);
    return 1;
}

void iot_ota_test_reset_flash(void)
{
    (void)memset(g_iot_ota_test_flash, 0, sizeof(g_iot_ota_test_flash));
    (void)memset(g_iot_ota_test_update_info, 0, sizeof(g_iot_ota_test_update_info));
}

void iot_ota_test_reset_download(void)
{
    (void)memset(g_iot_ota_test_downloaded_package, 0, sizeof(g_iot_ota_test_downloaded_package));
}

const uint8_t *iot_ota_test_firmware(void)
{
    return g_firmware;
}

uint32_t iot_ota_test_firmware_len(void)
{
    return sizeof(g_firmware);
}

static void put_be16(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)((value >> 8) & 0xFFU);
    buf[1] = (uint8_t)(value & 0xFFU);
}

static void put_be32(uint8_t *buf, uint32_t value)
{
    buf[0] = (uint8_t)((value >> 24) & 0xFFU);
    buf[1] = (uint8_t)((value >> 16) & 0xFFU);
    buf[2] = (uint8_t)((value >> 8) & 0xFFU);
    buf[3] = (uint8_t)(value & 0xFFU);
}

uint32_t iot_ota_test_build_package(void)
{
    uint32_t head_len = 20U;
    uint32_t total_len = head_len + (uint32_t)sizeof(g_firmware);

    (void)memset(g_iot_ota_test_package, 0, sizeof(g_iot_ota_test_package));
    put_be32(&g_iot_ota_test_package[0], 0);
    put_be32(&g_iot_ota_test_package[4], head_len);
    put_be32(&g_iot_ota_test_package[8], total_len);
    put_be16(&g_iot_ota_test_package[12], 4);
    put_be16(&g_iot_ota_test_package[14], 4);
    put_be32(&g_iot_ota_test_package[16], TEST_OTA_FULL_SOFTWARE);
    (void)memcpy(&g_iot_ota_test_package[head_len], g_firmware, sizeof(g_firmware));
    return total_len;
}

#if defined(OS_SUPPORT_OTA)
static uint32_t min_u32(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

static int ota_flash_read(ota_flash_type_e type, void *buf, int32_t len, uint32_t location)
{
    uint8_t *storage;
    uint32_t storage_len;

    if (buf == NULL || len < 0) {
        return -1;
    }

    if (type == OTA_UPDATE_INFO) {
        storage = g_iot_ota_test_update_info;
        storage_len = sizeof(g_iot_ota_test_update_info);
    } else {
        storage = g_iot_ota_test_flash;
        storage_len = sizeof(g_iot_ota_test_flash);
    }

    if (location + (uint32_t)len > storage_len) {
        return -1;
    }
    (void)memcpy(buf, &storage[location], (uint32_t)len);
    return 0;
}

static int ota_flash_write(ota_flash_type_e type, const void *buf, int32_t len, uint32_t location)
{
    uint8_t *storage;
    uint32_t storage_len;

    if (buf == NULL || len < 0) {
        return -1;
    }

    if (type == OTA_UPDATE_INFO) {
        storage = g_iot_ota_test_update_info;
        storage_len = sizeof(g_iot_ota_test_update_info);
    } else {
        storage = g_iot_ota_test_flash;
        storage_len = sizeof(g_iot_ota_test_flash);
    }

    if (location + (uint32_t)len > storage_len) {
        return -1;
    }
    (void)memcpy(&storage[location], buf, (uint32_t)len);
    return 0;
}

static int read_ver(char *buf, uint32_t len)
{
    const char ver[] = "V0.0";

    if (buf == NULL || len == 0) {
        return -1;
    }
    (void)memset(buf, 0, len);
    (void)memcpy(buf, ver, min_u32((uint32_t)strlen(ver), len));
    return 0;
}

static int sota_log(const char *fmt, ...)
{
    char str_buf[192];
    va_list list;
    int ret;

    va_start(list, fmt);
    ret = vsnprintf(str_buf, sizeof(str_buf), fmt, list);
    va_end(list);
    if (ret > 0) {
        PRT_Printf("%s", str_buf);
    }
    return ret;
}

static void *sota_malloc(size_t size)
{
    return malloc(size);
}

static void sota_free(void *ptr)
{
    free(ptr);
}

static int sota_send_str(const char *buf, int len)
{
#if defined(OS_SUPPORT_NB_IOT)
    return nb_send_str(buf, len);
#else
    (void)buf;
    return len;
#endif
}

int iot_ota_test_init_sota(void)
{
    sota_arg_s flash_op;

    (void)memset(&flash_op, 0, sizeof(flash_op));
    flash_op.get_ver = read_ver;
    flash_op.sota_send = sota_send_str;
    flash_op.sota_malloc = sota_malloc;
    flash_op.sota_free = sota_free;
    flash_op.sota_printf = sota_log;
    flash_op.current_run_stage = APPLICATION;
    flash_op.firmware_download_stage = APPLICATION;
    flash_op.ota_info.read_flash = ota_flash_read;
    flash_op.ota_info.write_flash = ota_flash_write;
    flash_op.ota_info.flash_block_size = 64;

    return (sota_init(&flash_op) == SOTA_OK) ? 0 : -1;
}

static void bytes_to_hex(const uint8_t *in, uint32_t in_len, char *out)
{
    uint32_t i;
    static const char hex[] = "0123456789ABCDEF";

    for (i = 0; i < in_len; i++) {
        out[i * 2U] = hex[(in[i] >> 4) & 0x0FU];
        out[i * 2U + 1U] = hex[in[i] & 0x0FU];
    }
    out[in_len * 2U] = '\0';
}

static int build_pcp_frame(uint8_t msg_code, const uint8_t *payload, uint32_t payload_len,
    char *frame, uint32_t frame_len)
{
    uint8_t bin[160];
    char hex[sizeof(bin) * 2U + 1U];
    uint16_t data_len;
    uint16_t crc;
    uint32_t bin_len;

    if (frame == NULL || payload_len + 8U > sizeof(bin) || (payload == NULL && payload_len != 0)) {
        return -1;
    }

    bin_len = payload_len + 8U;
    data_len = (uint16_t)payload_len;
    put_be16(&bin[0], PCP_HEAD);
    bin[2] = 1;
    bin[3] = msg_code;
    bin[4] = 0;
    bin[5] = 0;
    put_be16(&bin[6], data_len);
    if (payload_len > 0) {
        (void)memcpy(&bin[8], payload, payload_len);
    }

    crc = (uint16_t)crc_check(bin, (int)bin_len);
    put_be16(&bin[4], crc);
    bytes_to_hex(bin, bin_len, hex);
    if (snprintf(frame, frame_len, "+NNMI:%u,%s", (unsigned int)bin_len, hex) <= 0) {
        return -1;
    }
    return 0;
}

static int process_sota_frame(uint8_t msg_code, const uint8_t *payload, uint32_t payload_len)
{
    char frame[384];

    if (build_pcp_frame(msg_code, payload, payload_len, frame, sizeof(frame)) != 0) {
        return SOTA_FAILED;
    }
    if (sota_parse((const int8_t *)frame, (int32_t)strlen(frame), g_sota_buf, sizeof(g_sota_buf)) != SOTA_OK) {
        return SOTA_FAILED;
    }
    return sota_process(NULL, g_sota_buf, sizeof(g_sota_buf));
}

static int check_sota_parse_case(const char *name, const int8_t *in_buf, int32_t in_len,
    int8_t *out_buf, int32_t out_len, int32_t expected)
{
    int32_t ret = sota_parse(in_buf, in_len, out_buf, out_len);

    return iot_ota_test_check(ret == expected, name);
}

static int check_sota_process_case(const char *name, const char *frame, int32_t expected)
{
    int32_t ret;

    if (iot_ota_test_init_sota() != 0) {
        return iot_ota_test_check(0, name);
    }
    ret = sota_parse((const int8_t *)frame, (int32_t)strlen(frame), g_sota_buf, sizeof(g_sota_buf));
    if (ret != SOTA_OK) {
        return iot_ota_test_check(0, name);
    }
    ret = sota_process(NULL, g_sota_buf, sizeof(g_sota_buf));
    return iot_ota_test_check(ret == expected, name);
}

int iot_ota_test_run_sota_liteos_negative(void)
{
    int failures = 0;
    int8_t nnmi[] = "NNMI";
    int8_t valid_empty[] = "+NNMI:8,FFFE01134c9a0000";
    int8_t bad_msg[] = "+NNMI:8,FFFE0112b5010000";
    int8_t bad_type[] = "+NNMI:8,FFFE02134c9a0000";
    int8_t short_head[] = "+NNMI:8,FFFE0113";

    failures += iot_ota_test_check(sota_init(NULL) == SOTA_FAILED, "ota SOTA init rejects NULL callbacks");
    failures += check_sota_parse_case("ota SOTA parse rejects NULL input", NULL, 0,
        g_sota_buf, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects bad prefix", nnmi, (int32_t)strlen((char *)nnmi),
        g_sota_buf, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects zero output length", nnmi, (int32_t)strlen((char *)nnmi),
        g_sota_buf, 0, SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects NULL output", valid_empty,
        (int32_t)strlen((char *)valid_empty), NULL, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects wrong message code", bad_msg,
        (int32_t)strlen((char *)bad_msg), g_sota_buf, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects wrong packet type", bad_type,
        (int32_t)strlen((char *)bad_type), g_sota_buf, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse rejects short frame", short_head,
        (int32_t)strlen((char *)short_head), g_sota_buf, sizeof(g_sota_buf), SOTA_FAILED);
    failures += check_sota_parse_case("ota SOTA parse accepts empty ACK frame", valid_empty,
        (int32_t)strlen((char *)valid_empty), g_sota_buf, sizeof(g_sota_buf), SOTA_OK);

    failures += check_sota_process_case("ota SOTA process rejects notify without payload",
        "+NNMI:8,FFFE0114e51f0000", SOTA_INVALID_PACKET);
    failures += check_sota_process_case("ota SOTA process rejects unexpected version response",
        "+NNMI:26,FFFE0115765A001256312E300000000000000000000000000002", SOTA_UNEXPECT_PACKET);
    failures += check_sota_process_case("ota SOTA process accepts valid new-version notify",
        "+NNMI:30,FFFE0114913F001656312E3000000000000000000000000001F400043132", SOTA_DOWNLOADING);
    return failures;
}

int iot_ota_test_run_sota_upgrade(const uint8_t *package, uint32_t package_len)
{
    uint8_t notify_payload[22];
    uint8_t block_payload[IOT_OTA_TEST_SOTA_BLOCK_SIZE + 3U];
    uint32_t block_total;
    uint32_t offset;
    uint32_t block_seq;
    int ret;

    if (package == NULL || package_len == 0) {
        return -1;
    }

    (void)memset(notify_payload, 0, sizeof(notify_payload));
    (void)memcpy(notify_payload, "V2.0", strlen("V2.0"));
    block_total = (package_len + IOT_OTA_TEST_SOTA_BLOCK_SIZE - 1U) / IOT_OTA_TEST_SOTA_BLOCK_SIZE;
    put_be16(&notify_payload[16], IOT_OTA_TEST_SOTA_BLOCK_SIZE);
    put_be16(&notify_payload[18], (uint16_t)block_total);
    put_be16(&notify_payload[20], 0x1234);
    ret = process_sota_frame(MSG_NOTIFY_NEW_VER, notify_payload, sizeof(notify_payload));
    if (ret != SOTA_DOWNLOADING) {
        return -1;
    }

    offset = 0;
    block_seq = 0;
    while (offset < package_len) {
        uint32_t chunk_len = min_u32(IOT_OTA_TEST_SOTA_BLOCK_SIZE, package_len - offset);
        block_payload[0] = 0;
        put_be16(&block_payload[1], (uint16_t)block_seq);
        (void)memcpy(&block_payload[3], &package[offset], chunk_len);
        ret = process_sota_frame(MSG_GET_BLOCK, block_payload, chunk_len + 3U);
        offset += chunk_len;
        block_seq++;
        if (offset < package_len && ret != SOTA_DOWNLOADING) {
            return -1;
        }
        if (offset == package_len && ret != SOTA_UPDATING) {
            return -1;
        }
    }

    ret = process_sota_frame(MSG_EXC_UPDATE, NULL, 0);
    return (ret == SOTA_UPDATED) ? 0 : -1;
}
#endif
