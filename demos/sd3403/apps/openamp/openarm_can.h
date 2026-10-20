/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 网关的达妙电机 CAN FD 协议层
 *
 * 协议参考: https://github.com/enactic/openarm_can 1.4.1
 *   include/openarm/damiao_motor/dm_motor_constants.hpp（限幅）
 *   src/openarm/damiao_motor/dm_motor_control.cpp（MIT 帧打包、回帧解析）
 *
 * 总线: CAN FD, 仲裁段 1M / 数据段 5M (BRS), 帧长 8 字节。
 * 关节 send ID 0x01~0x07 / recv ID 0x11~0x17; 夹爪 send 0x08 / recv 0x18。
 */

#ifndef OPENARM_CAN_H
#define OPENARM_CAN_H

#include <stdbool.h>
#include <stdint.h>

#define OPENARM_NUM_MOTORS       8U
#define OPENARM_NUM_JOINTS       7U            /* 关节数 (不含夹爪) */
#define OPENARM_GRIPPER_IDX      7U            /* motors[7] = Gripper */
#define OPENARM_CANID_BROADCAST  0x7FFU        /* 参数查询/回读请求帧ID */

#define OPENARM_CMD_ENABLE       0xFCU
#define OPENARM_CMD_DISABLE      0xFDU
#define OPENARM_CMD_SET_ZERO     0xFEU

/* 回帧 D[0] 高 4 位（达妙手册 ERR）：0 失能、1 使能，8~E 为各类故障 */
#define OPENARM_STATE_ENABLED    1U

/* 达妙电机量化限幅: 位置统一 ±12.5 rad */
#define OPENARM_P_MAX            12.5
#define OPENARM_KP_MAX           500.0
#define OPENARM_KD_MAX           5.0

typedef enum {
    OPENARM_MOTOR_DM4310 = 1,
    OPENARM_MOTOR_DM4340 = 3,
    OPENARM_MOTOR_DM8009 = 7,
} OpenArmMotorType;

typedef struct {
    const char *name;
    uint32_t send_id;
    uint32_t recv_id;
    OpenArmMotorType type;
    double p_max;   /* rad   */
    double v_max;   /* rad/s */
    double t_max;   /* Nm    */
    /* 遥测状态 */
    double q;       /* rad   */
    double dq;      /* rad/s */
    double tau;     /* Nm    */
    int8_t t_mos;   /* 功率管温度 */
    int8_t t_rotor; /* 转子温度   */
    uint8_t state;  /* 最近一帧回帧的 ERR（OPENARM_STATE_*） */
    bool online;
} OpenArmMotor;

/* 按本机机械臂配置初始化电机表: [DM8009, DM8009, DM4340, DM4340,
 * DM4310, DM4310, DM4310] + 夹爪 DM4310
 * 调用方提供长度为 OPENARM_NUM_MOTORS 的数组。 */
void OpenArmMotorsInit(OpenArmMotor *motors);

/* MIT 控制帧打包 (与 openarm_can 库 pack_mit_control_data 逐字节一致)；
 * 各量钳到量程内，NaN 按 0 */
void OpenArmPackMit(const OpenArmMotor *motor, double kp, double kd,
    double q, double dq, double tau, uint8_t data[8]);

/* enable/disable/set_zero 命令帧: FF FF FF FF FF FF FF <cmd> */
void OpenArmPackCommand(uint8_t cmd, uint8_t data[8]);

/* 回读请求帧 (ID=0x7FF): {send_id_L, send_id_H, 0xCC, 0,0,0,0,0} */
void OpenArmPackRefresh(const OpenArmMotor *motor, uint8_t data[8]);

/* 解析电机回读帧 (recv_id, 8字节), 更新电机遥测状态 */
bool OpenArmParseState(OpenArmMotor *motor, const uint8_t data[8]);

#endif /* OPENARM_CAN_H */
