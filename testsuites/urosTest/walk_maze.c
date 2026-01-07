#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

// #include "example_interfaces/srv/add_two_ints.h"

#include "std_msgs/msg/string.h"

#include <stdio.h>
#include <unistd.h>
#include "prt_task.h"
#include "prt_mem.h"

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

// example_interfaces__srv__AddTwoInts_Request req;
static std_msgs__msg__String msg;
static char buffer[100];

static const int maze[5][5] = {
    {1, 0, 0, 0, 1},
    {1, 0, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0},
};
static int init_pos[2] = {4, 4};
static const int maze_size[2] = {5, 5};
// static const char block[3] = "██\0";
static const char block[3] = "||\0";
static const char road[3] = "  \0";
static const char object[3] = "<>\0";

static bool checkPosValid(int x, int y)
{
    if ((x >= maze_size[0]) || 
        (y >= maze_size[1]) ||
        x < 0 || y < 0 || maze[x][y] == 1) {
        return false;
    } else {
        return true;
    }
}

static void drawMaze()
{
    printf("========\n");
    for (int x = -1; x < 2; x++) {
        printf("=");
        for (int y = -1; y < 2; y++) {
            if (x == 0 && y == 0) {
                printf(object);
            } else if (checkPosValid(init_pos[0] + x, init_pos[1] + y)) {
                printf(road);
            } else {
                printf(block);
            }
        }
        printf("=\n");
    }
    printf("========\n");
}

static void subscription_callback(const void * msg_recv)
{
    std_msgs__msg__String *msgin = (std_msgs__msg__String *)msg_recv;

    printf("Received msg: %s\n", msgin->data.data);
    switch (msgin->data.data[0])
    {
    case 'w':
    case 'W':
        if (checkPosValid(init_pos[0] - 1, init_pos[1])) {
            init_pos[0] = init_pos[0] - 1;
        }
        break;
    case 'a':
    case 'A':
        if (checkPosValid(init_pos[0] , init_pos[1] - 1)) {
            init_pos[1] = init_pos[1] - 1;
        }
        break;
    case 's':
    case 'S':
        if (checkPosValid(init_pos[0] + 1, init_pos[1])) {
            init_pos[0] = init_pos[0] + 1;
        }
        break;
    case 'd':
    case 'D':
        if (checkPosValid(init_pos[0], init_pos[1] + 1)) {
            init_pos[1] = init_pos[1] + 1;
        }
        break;
    default:
        break;
    }
    drawMaze();
}

static int maze_subscriber(void)
{
    rcl_ret_t ret;
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    msg.data.capacity = sizeof(buffer);
    msg.data.data = buffer;
    msg.data.size = 0;

    // create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    // create node
    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "topic_helloworld_sub", "", &support));
    // create subscriber 
    rcl_subscription_t subscriber;
    RCCHECK(rclc_subscription_init_default(&subscriber, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "/mazeControl"));
    // create executor
    rclc_executor_t executor;
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
    drawMaze();
    ret = rclc_executor_spin(&executor);
    (void)printf("[TEST] executor spin %d", ret);
    RCCHECK(rcl_subscription_fini(&subscriber, &node));
    RCCHECK(rcl_node_fini(&node));
    return 0;
}

static void MazeTaskTEntry()
{
    printf("[TEST] maze test task entry\n");
    maze_subscriber();
}

U32 RosMazeDemo()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task server
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)MazeTaskTEntry;
    param.name = "maze_sub";
    param.taskPrio = 25;
    param.stackSize = 0x2000;
    param.policy = OS_TSK_SCHED_RR;

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }
    return 0;
}