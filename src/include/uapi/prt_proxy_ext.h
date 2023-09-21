#ifndef PRT_PROXY_EXT_H
#define PRT_PROXY_EXT_H

int PRT_ProxyIoctl(int fd, unsigned long request, void *arg, size_t len);

int PRT_ProxyPrintf(const char *format, ...);

#endif /* PRT_PROXY_EXT_H */