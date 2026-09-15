#include <math.h>
#include <stddef.h>
#include "openarm_gravity.h"

typedef struct { double x, y, z; } Vec3;
typedef struct { double v[3][3]; } Mat3;

static const Vec3 origin[7] = {
    {0.0, 0.0, 0.0625}, {-0.0301, 0.0, 0.06}, {0.0301, 0.0, 0.06625},
    {0.0, 0.0315, 0.15375}, {0.0, -0.0315, 0.0955},
    {0.0375, 0.0, 0.1205}, {-0.0375, 0.0, 0.0}
};
static const Vec3 axis[7] = {
    {0.0, 0.0, 1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0}
};
static const Vec3 com[7] = {
    {0.0011467657911800769, 0.00003319987657026362, 0.05395284380736254},
    {0.00839629182351943, -0.000000020145102027597523, 0.03256649300522363},
    {-0.002104752099628911, 0.0005549085042607548, 0.09047470545721961},
    {-0.0029006831074562967, -0.03030575826634669, 0.06339637422196209},
    {-0.003049665024221911, 0.0008866902457326625, 0.043079803024980934},
    {-0.037136587005447405, 0.00033230528343419053, -0.00009498374522309838},
    {0.00006875510271056056, 0.01266175250761268, 0.06951945409987448}
};
static const double mass[7] = {
    1.1416684646202298, 0.2775092746011571, 1.073863338202347,
    0.6348534566833373, 0.6156588026168502, 0.475202773187987,
    0.4659771327380578
};

static Vec3 add(Vec3 a, Vec3 b) { return (Vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static Vec3 sub(Vec3 a, Vec3 b) { return (Vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static Vec3 cross(Vec3 a, Vec3 b) {
    return (Vec3){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
static double dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static Vec3 mat_vec(Mat3 a, Vec3 b) {
    return (Vec3){a.v[0][0]*b.x+a.v[0][1]*b.y+a.v[0][2]*b.z,
        a.v[1][0]*b.x+a.v[1][1]*b.y+a.v[1][2]*b.z,
        a.v[2][0]*b.x+a.v[2][1]*b.y+a.v[2][2]*b.z};
}
static Mat3 mat_mul(Mat3 a, Mat3 b) {
    Mat3 r; size_t i, j, k;
    for (i=0; i<3; i++) for (j=0; j<3; j++) {
        r.v[i][j] = 0.0;
        for (k=0; k<3; k++) r.v[i][j] += a.v[i][k]*b.v[k][j];
    }
    return r;
}
static Mat3 rotation(Vec3 a, double angle) {
    double c=cos(angle), s=sin(angle), t=1.0-c;
    return (Mat3){{{t*a.x*a.x+c, t*a.x*a.y-s*a.z, t*a.x*a.z+s*a.y},
        {t*a.x*a.y+s*a.z, t*a.y*a.y+c, t*a.y*a.z-s*a.x},
        {t*a.x*a.z-s*a.y, t*a.y*a.z+s*a.x, t*a.z*a.z+c}}};
}

void OpenArmGravityRight(const double q[7], double tau[7])
{
    Mat3 r={{{1.0,0.0,0.0},{0.0,0.0,-1.0},{0.0,1.0,0.0}}};
    const Mat3 joint2_r=r; Vec3 p={0.0,0.0,0.0};
    Vec3 jp[7], ja[7], cp[7]; size_t i, link;
    for (i=0; i<7; i++) {
        p=add(p,mat_vec(r,origin[i]));
        if (i==1U) r=mat_mul(r,joint2_r);
        jp[i]=p; ja[i]=mat_vec(r,axis[i]);
        r=mat_mul(r,rotation(axis[i],q[i])); cp[i]=add(p,mat_vec(r,com[i]));
    }
    for (i=0; i<7; i++) {
        tau[i]=0.0;
        for (link=i; link<7; link++)
            tau[i]+=dot(ja[i],cross(sub(cp[link],jp[i]),(Vec3){0.0,0.0,mass[link]*9.81}));
    }
}
