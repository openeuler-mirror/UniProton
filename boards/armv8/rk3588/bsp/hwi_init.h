#ifndef __HWI_INIT_H__
#define __HWI_INIT_H__

#if (OS_GIC_VER == 3)
enum SicGroupType {
    SIC_GROUP_G0S  = 0,
    SIC_GROUP_G1NS = 1,
    SIC_GROUP_G1S  = 2,
    SIC_GROUP_BUTT,
};
void OsSicSetGroup(U32 intId, enum SicGroupType groupId);
#endif

#endif