#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

extern char OsGdbGetchar();

extern void OsGdbPutchar(char ch);

extern int OsGdbRingBufferInit();

extern void OsGdbFlush();

#endif /* _RINGBUFFER_H_ */