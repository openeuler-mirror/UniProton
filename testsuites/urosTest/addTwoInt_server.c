#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "example_interfaces/srv/add_two_ints.h"

#include <stdio.h>
#include <unistd.h>

#include "prt_task.h"
#include "prt_mem.h"

extern U32 PRT_Printf(const char *format, ...);

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

static void service_callback(const void * req, void * res){
  example_interfaces__srv__AddTwoInts_Request * req_in = (example_interfaces__srv__AddTwoInts_Request *) req;
  example_interfaces__srv__AddTwoInts_Response * res_in = (example_interfaces__srv__AddTwoInts_Response *) res;

  printf("Service request value: %d + %d.\n", (int) req_in->a, (int) req_in->b);

  res_in->sum = req_in->a + req_in->b;
}

static rclc_support_t support; // global
static rcl_allocator_t allocator; // global
static rcl_node_t node_server;
static rcl_node_t node_client;
static rclc_executor_t executor_server; // ??
static rcl_service_t service;
static rcl_client_t client;
static example_interfaces__srv__AddTwoInts_Response res_server;
static example_interfaces__srv__AddTwoInts_Request req_server;

example_interfaces__srv__AddTwoInts_Request req_client;
example_interfaces__srv__AddTwoInts_Response res_client;

static void client_callback(const void * msg){
  example_interfaces__srv__AddTwoInts_Response * msgin = (example_interfaces__srv__AddTwoInts_Response * ) msg;
  printf("Received service response %ld + %ld = %ld\n", req_client.a, req_client.b, msgin->sum);
}

// https://github.com/micro-ROS/micro-ROS-demos/blob/humble/rclc/addtwoints_server/main.c

static void ros_add_two_int_server_init(void)
{	
    // rmw_uros_set_custom_transport?

    // rmw_uros_options_set_udp_address()?
    allocator = rcl_get_default_allocator();

    // create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // create node（应该初始化所有node?)
    RCCHECK(rclc_node_init_default(&node_server, "add_twoints_server_rclc", "test", &support));
    sleep(1);
    RCCHECK(rclc_node_init_default(&node_client, "add_twoints_client_rclc", "test", &support));

    // create service
    RCCHECK(rclc_service_init_default(&service, &node_server, ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, srv, AddTwoInts), "/addtwoints"));
    RCCHECK(rclc_client_init_default(&client, &node_client, ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, srv, AddTwoInts), "/addtwoints"));

    // create executor
    RCCHECK(rclc_executor_init(&executor_server, &support.context, 2, &allocator));

    // 应该添加所有要注册的东西？？
    RCCHECK(rclc_executor_add_service(&executor_server, &service, &req_server, &res_server, service_callback));
    RCCHECK(rclc_executor_add_client(&executor_server, &client, &res_client, client_callback));

    printf("ros_add_two_int_server_init finish\n");
    return;
}

static void ros_add_two_int_server_run(void)
{
  printf("ros_add_two_int_server_run\n");
  rclc_executor_spin(&executor_server);

  RCCHECK(rcl_service_fini(&service, &node_server));

  RCCHECK(rcl_node_fini(&node_server));

  RCCHECK(rcl_client_fini(&client, &node_client));
  RCCHECK(rcl_node_fini(&node_client));
}


static void ros_add_two_int_client(void)
{
  int64_t seq; 
  example_interfaces__srv__AddTwoInts_Request__init(&req_client);
  req_client.a = 24;
  req_client.b = 42;
  printf("ros_add_two_int_client\n");
  sleep(2); // Sleep a while to ensure DDS matching before sending request

  RCCHECK(rcl_send_request(&client, &req_client, &seq))
  printf("Send service request %ld + %ld. Seq %ld\n",req_client.a, req_client.b, seq);
}


static int servere_init = 0;

static void ServerTaskEntry()
{
    printf("[TEST] ros server task entry\n");

    ros_add_two_int_server_init();

    servere_init = 1;
    ros_add_two_int_server_run();
}

static void ClientTaskEntry()
{
    printf("[TEST] ros client task entry\n");

    while (servere_init != 1) {
        PRT_TaskDelay(100);
    }
    PRT_TaskDelay(100);
    ros_add_two_int_client();
}

U32 RosServerClientTest()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task server
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)ServerTaskEntry;
    param.name = "addTwoInt_server";
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

    // task client
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)ClientTaskEntry;
    param.name = "addTwoInt_client";
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