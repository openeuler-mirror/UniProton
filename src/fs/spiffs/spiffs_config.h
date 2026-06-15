#ifndef UNIPROTON_SPIFFS_CONFIG_H
#define UNIPROTON_SPIFFS_CONFIG_H

#include <stddef.h>
#include <stdint.h>

typedef int8_t s8_t;
typedef uint8_t u8_t;
typedef int16_t s16_t;
typedef uint16_t u16_t;
typedef int32_t s32_t;
typedef uint32_t u32_t;

#define SPIFFS_PHYS_ADDR 0
#define SPIFFS_PHYS_SIZE 0x100000

#define SPIFFS_USE_MAGIC 1
#define SPIFFS_USE_MAGIC_LENGTH 1
#define SPIFFS_HAL_CALLBACK_EXTRA 1
#define SPIFFS_FILEHDL_OFFSET 1
#define TEST_SPIFFS_FILEHDL_OFFSET 0x1000
#define SPIFFS_OBJ_NAME_LEN 32

#define SPIFFS_BUFFER_HELP 0
#define SPIFFS_CACHE 1
#define SPIFFS_CACHE_WR 1
#define SPIFFS_CACHE_STATS 0
#define SPIFFS_PAGE_CHECK 1
#define SPIFFS_GC_MAX_RUNS 5
#define SPIFFS_GC_STATS 0
#define SPIFFS_GC_HEUR_W_DELET 5
#define SPIFFS_GC_HEUR_W_USED (-1)
#define SPIFFS_GC_HEUR_W_ERASE_AGE 50
#define SPIFFS_OBJ_META_LEN 0
#define SPIFFS_COPY_BUFFER_STACK 64
#define SPIFFS_SINGLETON 0
#define SPIFFS_ALIGNED_OBJECT_INDEX_TABLES 0
#define SPIFFS_READ_ONLY 0
#define SPIFFS_TEMPORAL_FD_CACHE 1
#define SPIFFS_TEMPORAL_CACHE_HIT_SCORE 4
#define SPIFFS_IX_MAP 1
#define SPIFFS_NO_BLIND_WRITES 0
#define SPIFFS_TEST_VISUALISATION 0

#define SPIFFS_LOCK(fs)
#define SPIFFS_UNLOCK(fs)

typedef u16_t spiffs_block_ix;
typedef u16_t spiffs_page_ix;
typedef u16_t spiffs_obj_id;
typedef u16_t spiffs_span_ix;

#define SPIFFS_DBG(_f, ...)
#define SPIFFS_GC_DBG(_f, ...)
#define SPIFFS_CACHE_DBG(_f, ...)
#define SPIFFS_CHECK_DBG(_f, ...)
#define SPIFFS_API_DBG(_f, ...)

#define _SPIPRIi   "%ld"
#define _SPIPRIad  "%08x"
#define _SPIPRIbl  "%04x"
#define _SPIPRIpg  "%04x"
#define _SPIPRIsp  "%04x"
#define _SPIPRIfd  "%d"
#define _SPIPRIid  "%04x"
#define _SPIPRIfl  "%02x"

#endif
