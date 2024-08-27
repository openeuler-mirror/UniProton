#ifndef RPMSG_COMMON_EXTERN_H
#define RPMSG_COMMON_EXTERN_H
#include <stdint.h>

void rpmsg_master(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
void rpmsg_remote(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);

void rpmsg_remote_shmemonly(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
void rpmsg_master_shmemonly(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
void rpmsg_remote_rpc_on_que(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
void rpmsg_master_rpc_on_que(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
#endif