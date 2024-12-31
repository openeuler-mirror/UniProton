#include "rtos_cmdqu.h"
#include "cvi_spinlock.h"
#include "cvi_mailbox.h"
#include <stdint.h>
#include <stddef.h>
#include "top_reg.h"
static cmdqu_irq_handler g_cmd_handler[MAX_CMD_NUM];
static void* g_private_data[MAX_CMD_NUM];


/* mailbox parameters */
volatile struct mailbox_set_register *mbox_reg;
volatile struct mailbox_done_register *mbox_done_reg;
volatile unsigned long *mailbox_context; // mailbox buffer context is 64 Bytess

#define CMDID_VALID(x)  				((x) >= CMDQU_SEND_TEST && (x) < CMDQU_SYSTEM_BUTT)
#define CMDID_INVALID(x)				(!CMDID_VALID(x))
static inline int cmdid_handler_occupied(enum SYSTEM_CMD_TYPE cmdid)
{
	if(CMDID_INVALID(cmdid))
		return -1;
	if(g_cmd_handler[cmdid - CMDQU_SEND_TEST] != NULL)
		return 1;
	return 0;
}

static inline int cmdid_handler_free(enum SYSTEM_CMD_TYPE cmdid)
{
	int ret = cmdid_handler_occupied(cmdid);
	if(ret < 0)
		return ret;
	return !ret;
}
//private function
static inline void __set_cmdid_handler(enum SYSTEM_CMD_TYPE cmdid, cmdqu_irq_handler handler, void* data)
{
		g_cmd_handler[cmdid - CMDQU_SEND_TEST] = handler;
		g_private_data[cmdid - CMDQU_SEND_TEST] = data;
}


int request_cmdqu_irq(enum SYSTEM_CMD_TYPE cmdid, cmdqu_irq_handler cmdqu_irq_func, void* data)
{
	if(CMDID_INVALID(cmdid))
		return -1;
	__set_cmdid_handler(cmdid, cmdqu_irq_func, data);
	return 0;
}



DEFINE_CVI_SPINLOCK(mailbox_lock, SPIN_MBOX);
int rtos_cmdqu_send(cmdqu_t* cmdq)
{
    int ret = 0;
	int valid;
	int mb_flags;
	cmdqu_t *rtos_cmdqu_t;


	drv_spin_lock_irqsave(&mailbox_lock, mb_flags);
	if (mb_flags == MAILBOX_LOCK_FAILED) 
	{
		uart_printf("cmdqu send error : ip_id=%d cmd_id=%d param_ptr=%x\n", cmdq->ip_id, cmdq->cmd_id, (unsigned int)cmdq->param_ptr);
		return -1;
	}

	rtos_cmdqu_t = (cmdqu_t *) mailbox_context;
	for (valid = 0; valid < MAILBOX_MAX_NUM; valid++) 
	{
		if (rtos_cmdqu_t->resv.valid.linux_valid == 0 && rtos_cmdqu_t->resv.valid.rtos_valid == 0) 
		{
			// mailbox buffer context is int (4 bytes) access
			int *ptr = (int *)rtos_cmdqu_t;

			rtos_cmdqu_t->resv.valid.rtos_valid = 1;
			*ptr = ((cmdq->ip_id << 0) | (cmdq->cmd_id << 8) | (cmdq->block << 15) |
					(rtos_cmdqu_t->resv.valid.linux_valid << 16) |
					(rtos_cmdqu_t->resv.valid.rtos_valid << 24));
			rtos_cmdqu_t->param_ptr = cmdq->param_ptr;
			/*
			uart_printf("mailbox contex id = %d\n", valid);	
			uart_printf("ip_id = %d\n", rtos_cmdqu_t->ip_id);
			uart_printf("cmd_id = %d\n", rtos_cmdqu_t->cmd_id);
			uart_printf("block = %d\n", rtos_cmdqu_t->block);
			uart_printf("linux_valid = %d\n", rtos_cmdqu_t->resv.valid.linux_valid);
			uart_printf("rtos_valid = %d\n", rtos_cmdqu_t->resv.valid.rtos_valid);
			uart_printf("param_ptr = %x\n", rtos_cmdqu_t->param_ptr);
			uart_printf("*ptr = %x\n", *ptr);
			*/
			// clear mailbox
			mbox_reg->cpu_mbox_set[SEND_TO_CPU1].cpu_mbox_int_clr.mbox_int_clr = (1 << valid);
			// trigger mailbox valid to rtos
			mbox_reg->cpu_mbox_en[SEND_TO_CPU1].mbox_info |= (1 << valid);
			mbox_reg->mbox_set.mbox_set = (1 << valid);
			break;
		}
		rtos_cmdqu_t++;
	}

	if (valid >= MAILBOX_MAX_NUM) 
	{
		uart_printf("No valid mailbox is available\n");
		drv_spin_unlock_irqrestore(&mailbox_lock, mb_flags);
		return -1;
	}
	drv_spin_unlock_irqrestore(&mailbox_lock, mb_flags);
	return ret;
}

void rtos_cmdqu_init(void)
{
    cvi_spinlock_init();
    unsigned int reg_base = MAILBOX_REG_BASE;
    mbox_reg = (struct mailbox_set_register *) reg_base;
    mbox_done_reg = (struct mailbox_done_register *) (reg_base + 2);
    mailbox_context = (unsigned long *) (MAILBOX_REG_BUFF);
    for(int i=0;i<MAX_CMD_NUM;i++)
    {
        g_cmd_handler[i] = NULL;
        g_private_data[i] = NULL;
    }       
    uart_printf("rtos cmdqu init done!\n");
}

void cmdqu_intr(void)
{
	unsigned char set_val;
	unsigned char valid_val;
	int i;
	cmdqu_t *cmdq;
	int flags;
	
	while(1)
	{
		drv_spin_lock_irqsave(&mailbox_lock, flags);
		if(flags == MAILBOX_LOCK_FAILED)
			continue;
		break;
	}
	set_val = mbox_reg->cpu_mbox_set[RECEIVE_CPU].cpu_mbox_int_int.mbox_int;
	int errno_mailbox[MAILBOX_MAX_NUM] = {0};
	cmdqu_t err_cmdq[MAILBOX_MAX_NUM];
	int erro_num = 0;
	if (set_val) 
	{
		for(i = 0; i < MAILBOX_MAX_NUM && set_val > 0; i++) 
		{
			valid_val = set_val  & (1 << i);
			if (valid_val) 
			{
				cmdqu_t rtos_cmdq;
				cmdq = (cmdqu_t *)(mailbox_context) + i;

				/* mailbox buffer context is send from linux, clear mailbox interrupt */
				mbox_reg->cpu_mbox_set[RECEIVE_CPU].cpu_mbox_int_clr.mbox_int_clr = valid_val;
				// need to disable enable bit
				mbox_reg->cpu_mbox_en[RECEIVE_CPU].mbox_info &= ~valid_val;

				// copy cmdq context (8 bytes) to buffer ASAP
				*((unsigned long *) &rtos_cmdq) = *((unsigned long *)cmdq);
				/* need to clear mailbox interrupt before clear mailbox buffer */
				*((unsigned long*) cmdq) = 0;
				
				set_val &= ~valid_val;

				/* mailbox buffer context is send from linux*/
				if (rtos_cmdq.resv.valid.linux_valid == 1) 
				{
					/*
					uart_printf("mailbox_contex_id = %d\n", i);
					uart_printf("cmdq->ip_id =%d\n", rtos_cmdq.ip_id);
					uart_printf("cmdq->cmd_id =%d\n", rtos_cmdq.cmd_id);
					uart_printf("cmdq->param_ptr =%x\n", rtos_cmdq.param_ptr);
					uart_printf("cmdq->block =%x\n", rtos_cmdq.block);
					uart_printf("cmdq->linux_valid =%d\n", rtos_cmdq.resv.valid.linux_valid);
					uart_printf("cmdq->rtos_valid =%x\n", rtos_cmdq.resv.valid.rtos_valid);
					*/
					if(CMDID_INVALID(rtos_cmdq.cmd_id)) 
					{
						uart_printf("recieve a invalid cmd package!\n");
						continue;
					}

					if(g_cmd_handler[rtos_cmdq.cmd_id - CMDQU_SEND_TEST] == NULL) 
					{
						uart_printf("a packeg cmd_id don't register %d\n",rtos_cmdq.cmd_id);
						continue;
					}
					
					int ret = g_cmd_handler[rtos_cmdq.cmd_id - CMDQU_SEND_TEST](&rtos_cmdq, g_private_data[rtos_cmdq.cmd_id - CMDQU_SEND_TEST]);
					if(ret) 
					{
						errno_mailbox[i] = ret;
						err_cmdq[i] = *cmdq;
						erro_num++;
					}
				} 
				else
				{
						uart_printf("rtos cmdq is not valid %d, ip=%d , cmd=%d\n",
						rtos_cmdq.resv.valid.rtos_valid, rtos_cmdq.ip_id, rtos_cmdq.cmd_id);
				}
			}
		}
	}
	drv_spin_unlock_irqrestore(&mailbox_lock, flags);
	if(erro_num > 0) 
	{
		uart_printf("rtos irq_handler error! total amount: %d\n", erro_num);
		for (i = 0; i < MAILBOX_MAX_NUM; i++) 
		{
			if(errno_mailbox[i] != 0) 
			{
				uart_printf("mailbox contex id %d errno %d\n", i, errno_mailbox[i]);
				uart_printf("cmdq->ip_id =%d\n", err_cmdq[i].ip_id);
				uart_printf("cmdq->cmd_id =%d\n", err_cmdq[i].cmd_id);
				uart_printf("cmdq->param_ptr =%x\n", err_cmdq[i].param_ptr);
				uart_printf("cmdq->block =%d\n", err_cmdq[i].block);
				uart_printf("cmdq->linux_valid =%d\n", err_cmdq[i].resv.valid.linux_valid);
				uart_printf("cmdq->rtos_valid =%x\n", err_cmdq[i].resv.valid.rtos_valid);
			}
		}
	}
}

