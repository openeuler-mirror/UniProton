#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <turtlesim/action/rotate_absolute.h>
#include <geometry_msgs/msg/twist.h>

#include "prt_log.h"
#include "prt_task.h"
#include "prt_mem.h"

#define RCCHECK(fn) {rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) {printf( \
        "Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) {rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) {printf( \
        "Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc);}}

static bool goal_completed = false;
static bool executor_start = false;

static geometry_msgs__msg__Twist twist_msg;
static turtlesim__action__RotateAbsolute_SendGoal_Request ros_goal_request;
static rclc_action_client_t action_client;
static rcl_publisher_t pub;

// https://github.com/micro-ROS/micro-ROS-demos/blob/humble/rclc/fibonacci_action_server/main.c

static void goal_request_callback(rclc_action_goal_handle_t * goal_handle, bool accepted, void * context)
{
  (void) context;
  turtlesim__action__RotateAbsolute_SendGoal_Request * request =
    (turtlesim__action__RotateAbsolute_SendGoal_Request *) goal_handle->ros_goal_request;

  (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "Goal request (theta: %f): %s\r\n",
    request->goal.theta, accepted ? "Accepted" : "Rejected");

  if (!accepted) {
    goal_completed = true;
  }
}

static void feedback_callback(rclc_action_goal_handle_t * goal_handle, void * ros_feedback, void * context)
{
  (void) context;

  turtlesim__action__RotateAbsolute_SendGoal_Request * request =
    (turtlesim__action__RotateAbsolute_SendGoal_Request *) goal_handle->ros_goal_request;

  turtlesim__action__RotateAbsolute_FeedbackMessage * feedback =
    (turtlesim__action__RotateAbsolute_FeedbackMessage *) ros_feedback;
  
  (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "Goal Feedback (theta: %f) [ remaining: %f ]\r\n",
    request->goal.theta, feedback->feedback.remaining);
}

static void result_request_callback(
  rclc_action_goal_handle_t * goal_handle, void * ros_result_response,
  void * context)
{
  (void) context;

  turtlesim__action__RotateAbsolute_SendGoal_Request * request =
    (turtlesim__action__RotateAbsolute_SendGoal_Request *) goal_handle->ros_goal_request;

  (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "Goal Feedback (theta: %f)", request->goal.theta);

  turtlesim__action__RotateAbsolute_GetResult_Response * result =
    (turtlesim__action__RotateAbsolute_GetResult_Response *) ros_result_response;

  if (result->status == GOAL_STATE_SUCCEEDED) {
    (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "finished, delta: %f ", result->result.delta);
  } else if (result->status == GOAL_STATE_CANCELED) {
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "CANCELED", 9);
  } else {
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "ABORTED", 7);
  }

  goal_completed = true;
}

static void cancel_request_callback(
  rclc_action_goal_handle_t * goal_handle, bool cancelled,
  void * context)
{
  (void) context;

  turtlesim__action__RotateAbsolute_SendGoal_Request * request =
    (turtlesim__action__RotateAbsolute_SendGoal_Request *) goal_handle->ros_goal_request;

  (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "Goal cancel request (theta: %f): %s",
    request->goal.theta, cancelled ? "Accepted" : "Rejected");

  if (cancelled) {
    goal_completed = true;
  }
}

static int ros_turtlesim_control()
{
  rcl_ret_t ret;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  // create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  rcl_node_t node;
  RCCHECK(rclc_node_init_default(&node, "turtlesim_control", "", &support));

  // Create action client

  RCCHECK(
    rclc_action_client_init_default(
      &action_client,
      &node,
      ROSIDL_GET_ACTION_TYPE_SUPPORT(turtlesim, RotateAbsolute),
      "/turtle1/rotate_absolute"
  ));

  RCCHECK(
    rclc_publisher_init_default(&pub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "/turtle1/cmd_vel"
  ));

  // Create executor
  rclc_executor_t executor;
  rclc_executor_init(&executor, &support.context, 1, &allocator);

  turtlesim__action__RotateAbsolute_FeedbackMessage ros_feedback;
  turtlesim__action__RotateAbsolute_GetResult_Response ros_result_response;

  RCCHECK(
    rclc_executor_add_action_client(
      &executor,
      &action_client,
      10,
      &ros_result_response,
      NULL,
      goal_request_callback,
      NULL,
      result_request_callback,
      cancel_request_callback,
      (void *) &action_client
  ));

  sleep(2);
  
  executor_start = true;
  ret = rclc_executor_spin(&executor);
  (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "[TEST] executor spin %d", ret);

  (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "finish", 6);
  RCSOFTCHECK(rclc_action_client_fini(&action_client, &node))
  RCSOFTCHECK(rcl_publisher_fini(&pub, &node));
  RCSOFTCHECK(rcl_node_fini(&node))

  return 0;
}

static void static_turtle_control()
{
    while (!executor_start) {
        sleep(1);
    }

    printf("move forward\n");
    twist_msg.linear.x = 1;
    twist_msg.angular.z = 0;
    RCSOFTCHECK(rcl_publish(&pub, &twist_msg, NULL));

    sleep(1);

    printf("turn up\n");
    goal_completed = false;
    ros_goal_request.goal.theta = 1.5708;
    RCSOFTCHECK(rclc_action_send_goal_request(&action_client, &ros_goal_request, NULL));

    while (!goal_completed) {
        usleep(100000);
    }

    printf("move forward\n");
    twist_msg.linear.x = 1;
    twist_msg.angular.z = 0;
    RCSOFTCHECK(rcl_publish(&pub, &twist_msg, NULL));

    sleep(1);

    printf("turn right\n");
    goal_completed = false;
    ros_goal_request.goal.theta = 3.1416;
    RCSOFTCHECK(rclc_action_send_goal_request(&action_client, &ros_goal_request, NULL));

    while (!goal_completed) {
        usleep(100000);
    }

    printf("move forward\n");
    twist_msg.linear.x = 1;
    twist_msg.angular.z = 0;
    RCSOFTCHECK(rcl_publish(&pub, &twist_msg, NULL));

    sleep(1);

    printf("turn up\n");
    goal_completed = false;
    ros_goal_request.goal.theta = 1.5708;
    RCSOFTCHECK(rclc_action_send_goal_request(&action_client, &ros_goal_request, NULL));
    while (!goal_completed) {
        usleep(100000);
    }

    printf("move downward\n");
    twist_msg.linear.x = -1;
    twist_msg.angular.z = 0;
    RCSOFTCHECK(rcl_publish(&pub, &twist_msg, NULL));

    sleep(1);

    printf("draw circle\n");
    twist_msg.linear.x = 1;
    twist_msg.angular.z = 0.5;
    while(1) {
        RCSOFTCHECK(rcl_publish(&pub, &twist_msg, NULL));
        sleep(1);
    }
}

static void TurtleTaskTEntry()
{
    printf("[TEST] ros turtle test task entry\n");
    ros_turtlesim_control();
}

U32 RosTurtleActionTest()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task server
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TurtleTaskTEntry;
    param.name = "turtle_init";
    param.taskPrio = 25;
    param.stackSize = 0x2000;
    param.policy = OS_TSK_SCHED_RR;

    PRT_LogOff();

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }

    // task client
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)static_turtle_control;
    param.name = "static control";
    param.taskPrio = 24;
    param.stackSize = 0x2000;
    param.policy = OS_TSK_SCHED_RR;

    ret = PRT_TaskCreate(&testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[1]);
    if (ret) {
        return ret;
    }

    return 0;
}
