/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016,2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 * FILE NAME
 *
 *       rpmsg_compiler.h
 *
 * DESCRIPTION
 *
 *       This file defines compiler-specific macros.
 *
 ***************************************************************************/
#ifndef RPMSG_COMPILER_H_
#define RPMSG_COMPILER_H_

#define MEM_BARRIER() __asm__ volatile("fence rw, rw" ::: "memory")
#define RMEM_BARRIER() __asm__ volatile("fence r, r" ::: "memory")
#define WMEM_BARRIER() __asm__ volatile("fence w, w" ::: "memory")

#ifndef RL_PACKED_BEGIN
#define RL_PACKED_BEGIN
#endif

#ifndef RL_PACKED_END
#define RL_PACKED_END __attribute__((__packed__))
#endif

#ifndef RL_WEAK
#define RL_WEAK __attribute__((__weak__)) 
#endif
#endif /* RPMSG_COMPILER_H_ */
