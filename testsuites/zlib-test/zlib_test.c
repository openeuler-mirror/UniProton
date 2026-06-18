#include "zlib_test.h"

#include <string.h>
#include "zlib.h"

extern unsigned int PRT_Printf(const char *format, ...);

#define ZLIB_TEST_LOG(fmt, ...) PRT_Printf("[ZLIB_TEST] " fmt "\n", ##__VA_ARGS__)

static int zlib_check(int condition, const char *name)
{
    if (condition) {
        ZLIB_TEST_LOG("[PASS] %s", name);
        return 0;
    }

    ZLIB_TEST_LOG("[FAIL] %s", name);
    return 1;
}

static int zlib_basic_api_test(void)
{
    int fail = 0;
    const Bytef data[] = "UniProton zlib checksum api test data";
    uInt split = 12;
    uLong adler_once;
    uLong adler_step;
    uLong crc_once;
    uLong crc_step;

    ZLIB_TEST_LOG("version=%s, compile_flags=0x%lx", zlibVersion(), zlibCompileFlags());

    fail += zlib_check(strcmp(zlibVersion(), ZLIB_VERSION) == 0, "zlibVersion");
    fail += zlib_check(strcmp(zError(Z_STREAM_ERROR), "stream error") == 0, "zError");
    fail += zlib_check(compressBound(sizeof(data)) >= sizeof(data), "compressBound");

    adler_once = adler32(0L, Z_NULL, 0);
    adler_once = adler32(adler_once, data, sizeof(data));
    adler_step = adler32(0L, Z_NULL, 0);
    adler_step = adler32(adler_step, data, split);
    adler_step = adler32(adler_step, data + split, sizeof(data) - split);
    fail += zlib_check(adler_once == adler_step, "adler32 incremental");

    crc_once = crc32(0L, data, sizeof(data));
    crc_step = crc32(0L, Z_NULL, 0);
    crc_step = crc32(crc_step, data, split);
    crc_step = crc32(crc_step, data + split, sizeof(data) - split);
    fail += zlib_check(crc_once == crc_step, "crc32 incremental");

    return fail;
}

static int zlib_compress_uncompress_test(void)
{
    static const Bytef source[] =
        "UniProton zlib end to end test. "
        "This payload repeats repeats repeats so deflate can compress it. "
        "0123456789abcdefghijklmnopqrstuvwxyz.";
    Bytef compressed[256];
    Bytef restored[sizeof(source)];
    uLongf compressed_len = sizeof(compressed);
    uLongf restored_len = sizeof(restored);
    int ret;
    int fail = 0;

    ret = compress2(compressed, &compressed_len, source, sizeof(source), Z_BEST_COMPRESSION);
    fail += zlib_check(ret == Z_OK, "compress2");
    if (ret != Z_OK) {
        ZLIB_TEST_LOG("compress2 ret=%d", ret);
        return fail;
    }

    ret = uncompress(restored, &restored_len, compressed, compressed_len);
    fail += zlib_check(ret == Z_OK, "uncompress");
    fail += zlib_check(restored_len == sizeof(source), "uncompress length");
    fail += zlib_check(memcmp(restored, source, sizeof(source)) == 0, "uncompress payload");
    ZLIB_TEST_LOG("compress_uncompress source=%lu compressed=%lu restored=%lu",
        (unsigned long)sizeof(source), (unsigned long)compressed_len, (unsigned long)restored_len);

    return fail;
}

static int zlib_stream_test(void)
{
    static const Bytef source[] =
        "stream deflate inflate test stream deflate inflate test "
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    Bytef compressed[256];
    Bytef restored[sizeof(source)];
    z_stream defstream;
    z_stream infstream;
    int ret;
    int fail = 0;

    memset(&defstream, 0, sizeof(defstream));
    defstream.next_in = (Bytef *)source;
    defstream.avail_in = sizeof(source);
    defstream.next_out = compressed;
    defstream.avail_out = sizeof(compressed);

    ret = deflateInit(&defstream, Z_DEFAULT_COMPRESSION);
    fail += zlib_check(ret == Z_OK, "deflateInit");
    if (ret != Z_OK) {
        return fail;
    }

    ret = deflate(&defstream, Z_FINISH);
    fail += zlib_check(ret == Z_STREAM_END, "deflate finish");
    fail += zlib_check(deflateEnd(&defstream) == Z_OK, "deflateEnd");
    if (ret != Z_STREAM_END) {
        return fail;
    }

    memset(&infstream, 0, sizeof(infstream));
    infstream.next_in = compressed;
    infstream.avail_in = defstream.total_out;
    infstream.next_out = restored;
    infstream.avail_out = sizeof(restored);

    ret = inflateInit(&infstream);
    fail += zlib_check(ret == Z_OK, "inflateInit");
    if (ret != Z_OK) {
        return fail;
    }

    ret = inflate(&infstream, Z_FINISH);
    fail += zlib_check(ret == Z_STREAM_END, "inflate finish");
    fail += zlib_check(inflateEnd(&infstream) == Z_OK, "inflateEnd");
    fail += zlib_check(infstream.total_out == sizeof(source), "inflate length");
    fail += zlib_check(memcmp(restored, source, sizeof(source)) == 0, "inflate payload");
    ZLIB_TEST_LOG("stream source=%lu compressed=%lu restored=%lu",
        (unsigned long)sizeof(source), (unsigned long)defstream.total_out, (unsigned long)infstream.total_out);

    return fail;
}

void zlib_test(void)
{
    int failures = 0;

    ZLIB_TEST_LOG("start");
    failures += zlib_basic_api_test();
    failures += zlib_compress_uncompress_test();
    failures += zlib_stream_test();

    if (failures == 0) {
        ZLIB_TEST_LOG("ALL TESTS PASSED");
    } else {
        ZLIB_TEST_LOG("TESTS FAILED failures=%d", failures);
    }
}
