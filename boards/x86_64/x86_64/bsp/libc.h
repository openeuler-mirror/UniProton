#ifndef _PRT_LIBC_H_
#define _PRT_LIBC_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EXC_PREFIX_OFFSET   5
#define HWI_PREFIX_OFFSET   5
#define SYS_PREFIX_OFFSET   5
#define TSK_PREFIX_OFFSET   6
#define TSK_TAIL_OFFSET     7

#define AGENT_TASK_NAME     "ShellAgent"

#define SECUREC_VSPRINTF_PARAM_ERROR(format, strDest, destMax, maxLimit) \
    ((format) == NULL || (strDest) == NULL || (destMax) == 0 || (destMax) > (maxLimit))

#define SECUREC_VSPRINTF_CLEAR_DEST(strDest, destMax, maxLimit) do { \
    if ((strDest) != NULL && (destMax) > 0 && (destMax) <= (maxLimit)) { \
        *(strDest) = '\0'; \
    } \
} while (0)
#define SECUREC_FORMAT_FLAG_TABLE_SIZE 128

/* The return value of the internal function, which is returned when truncated */
#define SECUREC_PRINTF_TRUNCATE (-2)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif