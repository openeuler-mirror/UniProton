#ifndef __RTOS_COMMAND_QUEUE__
#define __RTOS_COMMAND_QUEUE__

struct valid_t {
	unsigned char linux_valid;
	unsigned char rtos_valid;
} __attribute__((packed));

typedef union resv_t {
	struct valid_t valid;
	unsigned short mstime; // 0 : noblock, -1 : block infinite
} resv_t;

typedef struct cmdqu_t cmdqu_t;
/* cmdqu size should be 8 bytes because of mailbox buffer size */
struct cmdqu_t {
	unsigned char ip_id;
	unsigned char cmd_id : 7;
	unsigned char block : 1;
	union resv_t resv;
	unsigned int  param_ptr;
} __attribute__((packed)) __attribute__((aligned(0x8)));

#define MAX_CMD_NUM	 128
/* keep those commands for ioctl system used */
/* cmd type don't more than 128!!!!!*/
enum SYSTEM_CMD_TYPE {
	CMDQU_SEND_TEST 	= 0	,
	CMDQU_DUO_TEST			,
	CMDQU_MCS_BOOT			,
	CMDQU_MCS_COMMUNICATE	,
	CMDQU_SYSTEM_BUTT		,
};

typedef int (*cmdqu_irq_handler)(cmdqu_t* cmdq, void* data);
int rtos_cmdqu_send(cmdqu_t* cmdq);
int request_cmdqu_irq(enum SYSTEM_CMD_TYPE, cmdqu_irq_handler cmdqu_irq_func, void* data);

void rtos_cmdqu_init(void);




#endif  // end of __RTOS_COMMAND_QUEUE__

