/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 网关的达妙电机 CAN FD 协议层实现
 *
 * 帧格式与 openarm_can 库 (enactic/openarm_can) 保持一致:
 *   MIT 控制帧 (send_id, 8B):
 *     [0:2] 位置 q    uint16 BE, [-p_max, +p_max] rad, 16bit
 *     [2:4] 速度 dq   12bit,       [-v_max, +v_max] rad/s
 *     [3:5] kp       12bit,       [0, 500]
 *     [5:7] kd       12bit,       [0, 5]
 *     [6:8] 力矩 tau  12bit,       [-t_max, +t_max] Nm
 *   回帧 (recv_id, 8B): [0] ERR<<4 | ID, [1:3] q 16bit, [3:5] dq 12bit,
 *     [4:6] tau 12bit, [6] T_mos, [7] T_rotor（量程同 MIT 帧）
 */

#include <math.h>
#include "openarm_can.h"

static double OpenArmLimitVMax(OpenArmMotorType type)
{
    switch (type) {
        case OPENARM_MOTOR_DM4340:
            return 10.0;
        case OPENARM_MOTOR_DM8009:
            return 45.0;
        case OPENARM_MOTOR_DM4310:
        default:
            return 30.0;
    }
}

static double OpenArmLimitTMax(OpenArmMotorType type)
{
    switch (type) {
        case OPENARM_MOTOR_DM4340:
            return 28.0;
        case OPENARM_MOTOR_DM8009:
            return 54.0;
        case OPENARM_MOTOR_DM4310:
        default:
            return 10.0;
    }
}

void OpenArmMotorsInit(OpenArmMotor *motors)
{
    static const OpenArmMotorType type_table[OPENARM_NUM_MOTORS] = {
        OPENARM_MOTOR_DM8009, OPENARM_MOTOR_DM8009,
        OPENARM_MOTOR_DM4340, OPENARM_MOTOR_DM4340,
        OPENARM_MOTOR_DM4310, OPENARM_MOTOR_DM4310, OPENARM_MOTOR_DM4310,
        OPENARM_MOTOR_DM4310, /* 夹爪 */
    };
    static const char *name_table[OPENARM_NUM_MOTORS] = {
        "J1", "J2", "J3", "J4", "J5", "J6", "J7", "Gripper",
    };
    uint32_t i;

    for (i = 0; i < OPENARM_NUM_MOTORS; i++) {
        OpenArmMotor *m = &motors[i];
        m->name = name_table[i];
        m->send_id = i + 1U;          /* 0x01 ~ 0x08 */
        m->recv_id = i + 1U + 0x10U;  /* 0x11 ~ 0x18 */
        m->type = type_table[i];
        m->p_max = OPENARM_P_MAX;
        m->v_max = OpenArmLimitVMax(type_table[i]);
        m->t_max = OpenArmLimitTMax(type_table[i]);
        m->q = 0.0;
        m->dq = 0.0;
        m->tau = 0.0;
        m->t_mos = 0;
        m->t_rotor = 0;
        m->state = 0;
        m->online = false;
    }
}

static uint16_t OpenArmQuant(double x, double x_min, double x_max, int bits)
{
    if (isnan(x)) {         /* (uint16_t)NaN 未定义；0 在每个量程内（kp/kd 为下限） */
        x = 0.0;
    }
    if (x < x_min) {
        x = x_min;
    }
    if (x > x_max) {
        x = x_max;
    }
    return (uint16_t)((x - x_min) / (x_max - x_min) * ((1U << bits) - 1U));
}

static double OpenArmDequant(uint16_t v, double x_min, double x_max, int bits)
{
    return x_min + (double)v / ((1U << bits) - 1U) * (x_max - x_min);
}

void OpenArmPackMit(const OpenArmMotor *motor, double kp, double kd,
    double q, double dq, double tau, uint8_t data[8])
{
    uint16_t q_uint = OpenArmQuant(q, -motor->p_max, motor->p_max, 16);
    uint16_t dq_uint = OpenArmQuant(dq, -motor->v_max, motor->v_max, 12);
    uint16_t kp_uint = OpenArmQuant(kp, 0.0, OPENARM_KP_MAX, 12);
    uint16_t kd_uint = OpenArmQuant(kd, 0.0, OPENARM_KD_MAX, 12);
    uint16_t tau_uint = OpenArmQuant(tau, -motor->t_max, motor->t_max, 12);

    data[0] = (uint8_t)(q_uint >> 8);
    data[1] = (uint8_t)(q_uint & 0xFFU);
    data[2] = (uint8_t)(dq_uint >> 4);
    data[3] = (uint8_t)(((dq_uint & 0xFU) << 4) | ((kp_uint >> 8) & 0xFU));
    data[4] = (uint8_t)(kp_uint & 0xFFU);
    data[5] = (uint8_t)(kd_uint >> 4);
    data[6] = (uint8_t)(((kd_uint & 0xFU) << 4) | ((tau_uint >> 8) & 0xFU));
    data[7] = (uint8_t)(tau_uint & 0xFFU);
}

void OpenArmPackCommand(uint8_t cmd, uint8_t data[8])
{
    uint32_t i;
    for (i = 0; i < 7U; i++) {
        data[i] = 0xFFU;
    }
    data[7] = cmd;
}

void OpenArmPackRefresh(const OpenArmMotor *motor, uint8_t data[8])
{
    data[0] = (uint8_t)(motor->send_id & 0xFFU);
    data[1] = (uint8_t)((motor->send_id >> 8) & 0xFFU);
    data[2] = 0xCCU;
    data[3] = 0U;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;
}

bool OpenArmParseState(OpenArmMotor *motor, const uint8_t data[8])
{
    uint16_t q_uint = ((uint16_t)data[1] << 8) | data[2];
    uint16_t dq_uint = ((uint16_t)data[3] << 4) | (data[4] >> 4);
    uint16_t tau_uint = ((uint16_t)(data[4] & 0xFU) << 8) | data[5];

    motor->q = OpenArmDequant(q_uint, -motor->p_max, motor->p_max, 16);
    motor->dq = OpenArmDequant(dq_uint, -motor->v_max, motor->v_max, 12);
    motor->tau = OpenArmDequant(tau_uint, -motor->t_max, motor->t_max, 12);
    motor->t_mos = (int8_t)data[6];
    motor->t_rotor = (int8_t)data[7];
    motor->state = (uint8_t)(data[0] >> 4);
    motor->online = true;
    return true;
}
