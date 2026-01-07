#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "std_msgs/msg/string.h"

#include <stdio.h>
#include <unistd.h>

#include "prt_task.h"
#include "prt_mem.h"

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

static std_msgs__msg__String msg;
static char buffer[100];
static rcl_publisher_t pub;

static void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	printf("Last callback time: %ld\n\r",  last_call_time);
	if (timer != NULL) {
        
        msg.data.capacity = sizeof(buffer);
        msg.data.data = buffer;
        msg.data.size = snprintf(buffer, 100, "time: %ld", last_call_time);
        RCSOFTCHECK(rcl_publish(&pub, &msg, NULL));
            /* publish */
	}
}

static int ros_msg_publisher(void)
{
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  // create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  // create node
  rcl_node_t node;
  RCCHECK(rclc_node_init_default(&node, "topic_helloworld_sub", "", &support));
  // create subscriber 
  
  RCCHECK(rclc_publisher_init_default(&pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "/chatter"));

  const unsigned int timer_period = RCL_MS_TO_NS(1000);

  // create executor
  rclc_executor_t executor;
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  rcl_timer_t timer;
  RCCHECK(rclc_timer_init_default(&timer, &support, timer_period, timer_callback));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  ret = rclc_executor_spin(&executor);

  RCCHECK(rcl_publisher_fini(&pub, &node));
  RCCHECK(rcl_timer_fini(&timer));
  RCCHECK(rcl_node_fini(&node));
  return 0;
}

static void PubTaskEntry()
{
    printf("[TEST] ros pub task entry\n");

    ros_msg_publisher();
}

U32 RosPubTest()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task server
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)PubTaskEntry;
    param.name = "chatter_pub";
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