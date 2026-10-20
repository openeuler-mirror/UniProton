/* openarm_control_dual.h - dual-arm library-backed OpenArm control.
 *
 * Two independent 7-joint chains on one fixed base: no dynamic coupling,
 * so each arm runs the same admittance law with its own URDF-verified
 * parameters. Messages are per-arm Direct IPC frames: seven joints plus
 * the gripper in slot 7.
 */
#ifndef OPENARM_CONTROL_DUAL_H
#define OPENARM_CONTROL_DUAL_H

#include "direct_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

void OpenArmControlDualReset(void);

/* arm slot order: left first, right second. The parameters must have
 * passed DirectJointParamsValid. */
void OpenArmControlStepDual(const struct DirectRobotFeedbackMessage *fLeft,
    const struct DirectRobotFeedbackMessage *fRight,
    struct DirectRobotTargetMessage *tLeft,
    struct DirectRobotTargetMessage *tRight,
    U32 activeJointMaskLeft, U32 activeJointMaskRight,
    const struct DirectJointParams *paramsLeft,
    const struct DirectJointParams *paramsRight);

#ifdef __cplusplus
}
#endif

#endif /* OPENARM_CONTROL_DUAL_H */
