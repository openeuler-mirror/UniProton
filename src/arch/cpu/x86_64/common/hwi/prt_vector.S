#include "prt_buildef.h"

.section .text

.extern OsHwiDispatchProc
.extern OsHwiDisapatchTail
.extern OsTskContextLoad
#ifdef OS_GDB_STUB
.extern OsHwiDbgExcProc
.extern OsGdbReenterChk
.global g_debugContext
#endif
.global g_runningTask
.global __os_sys_sp_end

#define HWI_RAX_OFFSET        0
#define HWI_RBX_OFFSET        8
#define HWI_RCX_OFFSET        16
#define HWI_RDX_OFFSET        24
#define HWI_RSI_OFFSET        32
#define HWI_RDI_OFFSET        40
#define HWI_RBP_OFFSET        48
#define HWI_R8_OFFSET         56
#define HWI_R9_OFFSET         64
#define HWI_R10_OFFSET        72
#define HWI_R11_OFFSET        80
#define HWI_R12_OFFSET        88
#define HWI_R13_OFFSET        96
#define HWI_R14_OFFSET        104
#define HWI_R15_OFFSET        112
#define HWI_INTNUM_OFFSET     120
#define HWI_RBP_FRAME_OFFSET  128
#define HWI_RIP_FRAME_OFFSET  136
#define HWI_ERROR_WORD_OFFSET 144
#define HWI_RIP_OFFSET        152
#define HWI_CS_OFFSET         160
#define HWI_RFLAGS_OFFSET     168
#define HWI_RSP_OFFSET        176
#define HWI_SS_OFFSET         184

.macro SAVE_REGISTER
    popq    HWI_ERROR_WORD_OFFSET(%rax)
    popq    HWI_RIP_OFFSET(%rax)
    popq    HWI_CS_OFFSET(%rax)
    popq    HWI_RFLAGS_OFFSET(%rax)
    popq    HWI_RSP_OFFSET(%rax)
    popq    HWI_SS_OFFSET(%rax)
    movq    %rbx, HWI_RBX_OFFSET(%rax)
    movq    %rcx, HWI_RCX_OFFSET(%rax)
    movq    %rdx, HWI_RDX_OFFSET(%rax)
    movq    %rsi, HWI_RSI_OFFSET(%rax)
    movq    %rdi, HWI_RDI_OFFSET(%rax)
    movq    %rbp, HWI_RBP_OFFSET(%rax)
    movq    %r8,  HWI_R8_OFFSET(%rax)
    movq    %r9,  HWI_R9_OFFSET(%rax)
    movq    %r10, HWI_R10_OFFSET(%rax)
    movq    %r11, HWI_R11_OFFSET(%rax)
    movq    %r12, HWI_R12_OFFSET(%rax)
    movq    %r13, HWI_R13_OFFSET(%rax)
    movq    %r14, HWI_R14_OFFSET(%rax)
    movq    %r15, HWI_R15_OFFSET(%rax)
.endm

.macro NOERROR_VECTOR intNum
.align 128
.globl OsVector\intNum
OsVector\intNum:
    pushq   $0
    cli
    and     $~15, %rsp
    sub     $0xc0, %rsp
    movq    %rax, HWI_RAX_OFFSET(%rsp)
    mov     %rsp, %rax
    add     $0xc0, %rsp
    SAVE_REGISTER
    movq    $(\intNum), HWI_INTNUM_OFFSET(%rax)
    sub     $0x200, %rax
    fxsave64 (%rax)
    movabs  $g_runningTask, %rcx
    movq    (%rcx), %r8
    movq    %rax, (%r8)
    jmp OsIntDispatcher
.endm

.macro ERROR_VECTOR intNum
.align 128
.globl OsVector\intNum
OsVector\intNum:
    cli
    and     $~15, %rsp
    sub     $0xc0, %rsp
    movq    %rax, HWI_RAX_OFFSET(%rsp)
    mov     %rsp, %rax
    add     $0xc0, %rsp
    SAVE_REGISTER
    movq    $(\intNum), HWI_INTNUM_OFFSET(%rax)
    sub     $0x200, %rax
    fxsave64 (%rax)
    movabs  $g_runningTask, %rcx
    movq    (%rcx), %r8
    movq    %rax, (%r8)
    jmp OsIntDispatcher

.endm

OsIntDispatcher:
    movq %rax, %rdi
    movabs $__os_sys_sp_end, %rsp
    cld

    call OsHwiDispatchProc
    movabs  $g_runningTask, %rax
    movq    (%rax), %rdi
    jmp     OsTskContextLoad

#ifdef OS_GDB_STUB
.macro NOERROR_VECTOR_DBG intNum
.align 128
.globl OsVector\intNum
.pushsection .stub.text,"ax"
OsVector\intNum:
    pushq   $0
    cli
    and     $~15, %rsp
    sub     $0xc0, %rsp
    movq    %rax, HWI_RAX_OFFSET(%rsp)
    mov     %rsp, %rax
    add     $0xc0, %rsp
    SAVE_REGISTER
    movq    $(\intNum), HWI_INTNUM_OFFSET(%rax)
    sub     $0x200, %rax
#if defined(OS_OPTION_FPU)
    fxsave64 (%rax)
#endif
    jmp OsDbgExcDispatcher
.popsection
.endm

.pushsection .stub.text,"ax"
OsDbgExcDispatcher:
    pushq   %rax
    leaq    0x200(%rax), %rdi
    call    OsGdbReenterChk
    test    %rax,%rax
    jne     L1
    movabs  $g_debugContext, %rcx
    popq    %rax
    movq    %rax, (%rcx)
    leaq    0x200(%rax), %rdi
    movabs  $__os_sys_sp_end, %rsp
    sub     $0x4000, %rsp
    cld
    call    OsHwiDbgExcProc
    movabs  $g_debugContext, %rdi
    jmp     OsDbgContextRestore
L1:
    popq    %rdi
    jmp     OsDbgContextRestore

OsDbgContextRestore:
    movq    (%rdi), %rax
#if defined(OS_OPTION_FPU)
    fxrstor64 (%rax)
#endif
    add     $0x200, %rax
    movq    HWI_RBX_OFFSET(%rax), %rbx
    movq    HWI_RCX_OFFSET(%rax), %rcx
    movq    HWI_RDX_OFFSET(%rax), %rdx
    movq    HWI_RSI_OFFSET(%rax), %rsi
    movq    HWI_RDI_OFFSET(%rax), %rdi
    movq    HWI_RBP_OFFSET(%rax), %rbp
    movq    HWI_R8_OFFSET(%rax),  %r8
    movq    HWI_R9_OFFSET(%rax),  %r9
    movq    HWI_R10_OFFSET(%rax), %r10
    movq    HWI_R11_OFFSET(%rax), %r11
    movq    HWI_R12_OFFSET(%rax), %r12
    movq    HWI_R13_OFFSET(%rax), %r13
    movq    HWI_R14_OFFSET(%rax), %r14
    movq    HWI_R15_OFFSET(%rax), %r15
    pushq   HWI_SS_OFFSET(%rax)
    pushq   HWI_RSP_OFFSET(%rax)
    pushq   HWI_RFLAGS_OFFSET(%rax)
    pushq   HWI_CS_OFFSET(%rax)
    pushq   HWI_RIP_OFFSET(%rax)
    movq    HWI_RAX_OFFSET(%rax), %rax
    iretq
.popsection
#endif

NOERROR_VECTOR 0
#ifdef OS_GDB_STUB
NOERROR_VECTOR_DBG 1
NOERROR_VECTOR_DBG 2
NOERROR_VECTOR_DBG 3
#else
NOERROR_VECTOR 1
NOERROR_VECTOR 2
NOERROR_VECTOR 3
#endif
NOERROR_VECTOR 4
NOERROR_VECTOR 5
NOERROR_VECTOR 6
NOERROR_VECTOR 7
ERROR_VECTOR   8
NOERROR_VECTOR 9
ERROR_VECTOR   10
ERROR_VECTOR   11
ERROR_VECTOR   12
ERROR_VECTOR   13
ERROR_VECTOR   14
NOERROR_VECTOR 15
NOERROR_VECTOR 16
NOERROR_VECTOR 17
NOERROR_VECTOR 18
NOERROR_VECTOR 19
NOERROR_VECTOR 20
NOERROR_VECTOR 21
NOERROR_VECTOR 22
NOERROR_VECTOR 23
NOERROR_VECTOR 24
NOERROR_VECTOR 25
NOERROR_VECTOR 26
NOERROR_VECTOR 27
NOERROR_VECTOR 28
NOERROR_VECTOR 29
NOERROR_VECTOR 30
NOERROR_VECTOR 31
NOERROR_VECTOR 32
NOERROR_VECTOR 33
NOERROR_VECTOR 34
NOERROR_VECTOR 35
NOERROR_VECTOR 36
NOERROR_VECTOR 37
NOERROR_VECTOR 38
NOERROR_VECTOR 39
NOERROR_VECTOR 40
NOERROR_VECTOR 41
NOERROR_VECTOR 42
NOERROR_VECTOR 43
NOERROR_VECTOR 44
NOERROR_VECTOR 45
NOERROR_VECTOR 46
NOERROR_VECTOR 47
NOERROR_VECTOR 48
NOERROR_VECTOR 49
NOERROR_VECTOR 50
NOERROR_VECTOR 51
NOERROR_VECTOR 52
NOERROR_VECTOR 53
NOERROR_VECTOR 54
NOERROR_VECTOR 55
NOERROR_VECTOR 56
NOERROR_VECTOR 57
NOERROR_VECTOR 58
NOERROR_VECTOR 59
NOERROR_VECTOR 60
NOERROR_VECTOR 61
NOERROR_VECTOR 62
NOERROR_VECTOR 63
NOERROR_VECTOR 64
NOERROR_VECTOR 65
NOERROR_VECTOR 66
NOERROR_VECTOR 67
NOERROR_VECTOR 68
NOERROR_VECTOR 69
NOERROR_VECTOR 70
NOERROR_VECTOR 71
NOERROR_VECTOR 72
NOERROR_VECTOR 73
NOERROR_VECTOR 74
NOERROR_VECTOR 75
NOERROR_VECTOR 76
NOERROR_VECTOR 77
NOERROR_VECTOR 78
NOERROR_VECTOR 79
NOERROR_VECTOR 80
NOERROR_VECTOR 81
NOERROR_VECTOR 82
NOERROR_VECTOR 83
NOERROR_VECTOR 84
NOERROR_VECTOR 85
NOERROR_VECTOR 86
NOERROR_VECTOR 87
NOERROR_VECTOR 88
NOERROR_VECTOR 89
NOERROR_VECTOR 90
NOERROR_VECTOR 91
NOERROR_VECTOR 92
NOERROR_VECTOR 93
NOERROR_VECTOR 94
NOERROR_VECTOR 95
NOERROR_VECTOR 96
NOERROR_VECTOR 97
NOERROR_VECTOR 98
NOERROR_VECTOR 99
NOERROR_VECTOR 100
NOERROR_VECTOR 101
NOERROR_VECTOR 102
NOERROR_VECTOR 103
NOERROR_VECTOR 104
NOERROR_VECTOR 105
NOERROR_VECTOR 106
NOERROR_VECTOR 107
NOERROR_VECTOR 108
NOERROR_VECTOR 109
NOERROR_VECTOR 110
NOERROR_VECTOR 111
NOERROR_VECTOR 112
NOERROR_VECTOR 113
NOERROR_VECTOR 114
NOERROR_VECTOR 115
NOERROR_VECTOR 116
NOERROR_VECTOR 117
NOERROR_VECTOR 118
NOERROR_VECTOR 119
NOERROR_VECTOR 120
NOERROR_VECTOR 121
NOERROR_VECTOR 122
NOERROR_VECTOR 123
NOERROR_VECTOR 124
NOERROR_VECTOR 125
NOERROR_VECTOR 126
NOERROR_VECTOR 127
NOERROR_VECTOR 128
NOERROR_VECTOR 129
NOERROR_VECTOR 130
NOERROR_VECTOR 131
NOERROR_VECTOR 132
NOERROR_VECTOR 133
NOERROR_VECTOR 134
NOERROR_VECTOR 135
NOERROR_VECTOR 136
NOERROR_VECTOR 137
NOERROR_VECTOR 138
NOERROR_VECTOR 139
NOERROR_VECTOR 140
NOERROR_VECTOR 141
NOERROR_VECTOR 142
NOERROR_VECTOR 143
NOERROR_VECTOR 144
NOERROR_VECTOR 145
NOERROR_VECTOR 146
NOERROR_VECTOR 147
NOERROR_VECTOR 148
NOERROR_VECTOR 149
NOERROR_VECTOR 150
NOERROR_VECTOR 151
NOERROR_VECTOR 152
NOERROR_VECTOR 153
NOERROR_VECTOR 154
NOERROR_VECTOR 155
NOERROR_VECTOR 156
NOERROR_VECTOR 157
NOERROR_VECTOR 158
NOERROR_VECTOR 159
NOERROR_VECTOR 160
NOERROR_VECTOR 161
NOERROR_VECTOR 162
NOERROR_VECTOR 163
NOERROR_VECTOR 164
NOERROR_VECTOR 165
NOERROR_VECTOR 166
NOERROR_VECTOR 167
NOERROR_VECTOR 168
NOERROR_VECTOR 169
NOERROR_VECTOR 170
NOERROR_VECTOR 171
NOERROR_VECTOR 172
NOERROR_VECTOR 173
NOERROR_VECTOR 174
NOERROR_VECTOR 175
NOERROR_VECTOR 176
NOERROR_VECTOR 177
NOERROR_VECTOR 178
NOERROR_VECTOR 179
NOERROR_VECTOR 180
NOERROR_VECTOR 181
NOERROR_VECTOR 182
NOERROR_VECTOR 183
NOERROR_VECTOR 184
NOERROR_VECTOR 185
NOERROR_VECTOR 186
NOERROR_VECTOR 187
NOERROR_VECTOR 188
NOERROR_VECTOR 189
NOERROR_VECTOR 190
NOERROR_VECTOR 191
NOERROR_VECTOR 192
NOERROR_VECTOR 193
NOERROR_VECTOR 194
NOERROR_VECTOR 195
NOERROR_VECTOR 196
NOERROR_VECTOR 197
NOERROR_VECTOR 198
NOERROR_VECTOR 199
NOERROR_VECTOR 200
NOERROR_VECTOR 201
NOERROR_VECTOR 202
NOERROR_VECTOR 203
NOERROR_VECTOR 204
NOERROR_VECTOR 205
NOERROR_VECTOR 206
NOERROR_VECTOR 207
NOERROR_VECTOR 208
NOERROR_VECTOR 209
NOERROR_VECTOR 210
NOERROR_VECTOR 211
NOERROR_VECTOR 212
NOERROR_VECTOR 213
NOERROR_VECTOR 214
NOERROR_VECTOR 215
NOERROR_VECTOR 216
NOERROR_VECTOR 217
NOERROR_VECTOR 218
NOERROR_VECTOR 219
NOERROR_VECTOR 220
NOERROR_VECTOR 221
NOERROR_VECTOR 222
NOERROR_VECTOR 223
NOERROR_VECTOR 224
NOERROR_VECTOR 225
NOERROR_VECTOR 226
NOERROR_VECTOR 227
NOERROR_VECTOR 228
NOERROR_VECTOR 229
NOERROR_VECTOR 230
NOERROR_VECTOR 231
NOERROR_VECTOR 232
NOERROR_VECTOR 233
NOERROR_VECTOR 234
NOERROR_VECTOR 235
NOERROR_VECTOR 236
NOERROR_VECTOR 237
NOERROR_VECTOR 238
NOERROR_VECTOR 239
NOERROR_VECTOR 240
NOERROR_VECTOR 241
NOERROR_VECTOR 242
NOERROR_VECTOR 243
NOERROR_VECTOR 244
NOERROR_VECTOR 245
NOERROR_VECTOR 246
NOERROR_VECTOR 247
NOERROR_VECTOR 248
NOERROR_VECTOR 249
NOERROR_VECTOR 250
NOERROR_VECTOR 251
NOERROR_VECTOR 252
NOERROR_VECTOR 253
NOERROR_VECTOR 254
NOERROR_VECTOR 255

.data
.globl g_osVectors
g_osVectors:
    .quad OsVector0
    .quad OsVector1
    .quad OsVector2
    .quad OsVector3
    .quad OsVector4
    .quad OsVector5
    .quad OsVector6
    .quad OsVector7
    .quad OsVector8
    .quad OsVector9
    .quad OsVector10
    .quad OsVector11
    .quad OsVector12
    .quad OsVector13
    .quad OsVector14
    .quad OsVector15
    .quad OsVector16
    .quad OsVector17
    .quad OsVector18
    .quad OsVector19
    .quad OsVector20
    .quad OsVector21
    .quad OsVector22
    .quad OsVector23
    .quad OsVector24
    .quad OsVector25
    .quad OsVector26
    .quad OsVector27
    .quad OsVector28
    .quad OsVector29
    .quad OsVector30
    .quad OsVector31
    .quad OsVector32
    .quad OsVector33
    .quad OsVector34
    .quad OsVector35
    .quad OsVector36
    .quad OsVector37
    .quad OsVector38
    .quad OsVector39
    .quad OsVector40
    .quad OsVector41
    .quad OsVector42
    .quad OsVector43
    .quad OsVector44
    .quad OsVector45
    .quad OsVector46
    .quad OsVector47
    .quad OsVector48
    .quad OsVector49
    .quad OsVector50
    .quad OsVector51
    .quad OsVector52
    .quad OsVector53
    .quad OsVector54
    .quad OsVector55
    .quad OsVector56
    .quad OsVector57
    .quad OsVector58
    .quad OsVector59
    .quad OsVector60
    .quad OsVector61
    .quad OsVector62
    .quad OsVector63
    .quad OsVector64
    .quad OsVector65
    .quad OsVector66
    .quad OsVector67
    .quad OsVector68
    .quad OsVector69
    .quad OsVector70
    .quad OsVector71
    .quad OsVector72
    .quad OsVector73
    .quad OsVector74
    .quad OsVector75
    .quad OsVector76
    .quad OsVector77
    .quad OsVector78
    .quad OsVector79
    .quad OsVector80
    .quad OsVector81
    .quad OsVector82
    .quad OsVector83
    .quad OsVector84
    .quad OsVector85
    .quad OsVector86
    .quad OsVector87
    .quad OsVector88
    .quad OsVector89
    .quad OsVector90
    .quad OsVector91
    .quad OsVector92
    .quad OsVector93
    .quad OsVector94
    .quad OsVector95
    .quad OsVector96
    .quad OsVector97
    .quad OsVector98
    .quad OsVector99
    .quad OsVector100
    .quad OsVector101
    .quad OsVector102
    .quad OsVector103
    .quad OsVector104
    .quad OsVector105
    .quad OsVector106
    .quad OsVector107
    .quad OsVector108
    .quad OsVector109
    .quad OsVector110
    .quad OsVector111
    .quad OsVector112
    .quad OsVector113
    .quad OsVector114
    .quad OsVector115
    .quad OsVector116
    .quad OsVector117
    .quad OsVector118
    .quad OsVector119
    .quad OsVector120
    .quad OsVector121
    .quad OsVector122
    .quad OsVector123
    .quad OsVector124
    .quad OsVector125
    .quad OsVector126
    .quad OsVector127
    .quad OsVector128
    .quad OsVector129
    .quad OsVector130
    .quad OsVector131
    .quad OsVector132
    .quad OsVector133
    .quad OsVector134
    .quad OsVector135
    .quad OsVector136
    .quad OsVector137
    .quad OsVector138
    .quad OsVector139
    .quad OsVector140
    .quad OsVector141
    .quad OsVector142
    .quad OsVector143
    .quad OsVector144
    .quad OsVector145
    .quad OsVector146
    .quad OsVector147
    .quad OsVector148
    .quad OsVector149
    .quad OsVector150
    .quad OsVector151
    .quad OsVector152
    .quad OsVector153
    .quad OsVector154
    .quad OsVector155
    .quad OsVector156
    .quad OsVector157
    .quad OsVector158
    .quad OsVector159
    .quad OsVector160
    .quad OsVector161
    .quad OsVector162
    .quad OsVector163
    .quad OsVector164
    .quad OsVector165
    .quad OsVector166
    .quad OsVector167
    .quad OsVector168
    .quad OsVector169
    .quad OsVector170
    .quad OsVector171
    .quad OsVector172
    .quad OsVector173
    .quad OsVector174
    .quad OsVector175
    .quad OsVector176
    .quad OsVector177
    .quad OsVector178
    .quad OsVector179
    .quad OsVector180
    .quad OsVector181
    .quad OsVector182
    .quad OsVector183
    .quad OsVector184
    .quad OsVector185
    .quad OsVector186
    .quad OsVector187
    .quad OsVector188
    .quad OsVector189
    .quad OsVector190
    .quad OsVector191
    .quad OsVector192
    .quad OsVector193
    .quad OsVector194
    .quad OsVector195
    .quad OsVector196
    .quad OsVector197
    .quad OsVector198
    .quad OsVector199
    .quad OsVector200
    .quad OsVector201
    .quad OsVector202
    .quad OsVector203
    .quad OsVector204
    .quad OsVector205
    .quad OsVector206
    .quad OsVector207
    .quad OsVector208
    .quad OsVector209
    .quad OsVector210
    .quad OsVector211
    .quad OsVector212
    .quad OsVector213
    .quad OsVector214
    .quad OsVector215
    .quad OsVector216
    .quad OsVector217
    .quad OsVector218
    .quad OsVector219
    .quad OsVector220
    .quad OsVector221
    .quad OsVector222
    .quad OsVector223
    .quad OsVector224
    .quad OsVector225
    .quad OsVector226
    .quad OsVector227
    .quad OsVector228
    .quad OsVector229
    .quad OsVector230
    .quad OsVector231
    .quad OsVector232
    .quad OsVector233
    .quad OsVector234
    .quad OsVector235
    .quad OsVector236
    .quad OsVector237
    .quad OsVector238
    .quad OsVector239
    .quad OsVector240
    .quad OsVector241
    .quad OsVector242
    .quad OsVector243
    .quad OsVector244
    .quad OsVector245
    .quad OsVector246
    .quad OsVector247
    .quad OsVector248
    .quad OsVector249
    .quad OsVector250
    .quad OsVector251
    .quad OsVector252
    .quad OsVector253
    .quad OsVector254
    .quad OsVector255
