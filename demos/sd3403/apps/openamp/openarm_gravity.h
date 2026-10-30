#ifndef OPENARM_GRAVITY_H
#define OPENARM_GRAVITY_H

#define OPENARM_GRAVITY_DOF 7U

#ifdef __cplusplus
extern "C" {
#endif

void OpenArmGravityRight(const double q[OPENARM_GRAVITY_DOF],
    double tau[OPENARM_GRAVITY_DOF]);

#ifdef __cplusplus
}
#endif

#endif
