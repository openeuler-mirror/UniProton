#include "ecrt.h"
#include "prt_sys.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define slavePos 0, 0
#define ASDA 0x000001dd, 0x00006010
#define frequency 100

static ec_master_t *master = NULL;
static ec_domain_t *domain = NULL;
static ec_domain_state_t domain_state = {};
static uint8_t *domain_pd = NULL;

/* Master 0, Slave 0, "ASDA-A3-E CoE Drive"
 * Vendor ID:       0x000001dd
 * Product code:    0x00006010
 * Revision number: 0x00030000
 */

struct pdo_entries_offset {
    unsigned int Control_Word;
    unsigned int Target_Position;
    unsigned int Status_Word;
    unsigned int Position_Actual_Value;
};

struct pdo_entries_offset pdos;

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x6040, 0x00, 16}, /* Control Word */
    {0x607a, 0x00, 32}, /* Target Position */
    {0x6041, 0x00, 16}, /* Status Word */
    {0x6064, 0x00, 32}, /* Actual Position */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1601, 2, slave_0_pdo_entries + 0},
    {0x1a01, 2, slave_0_pdo_entries + 2},
};

ec_sync_info_t slave_0_syncs[] = {{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
                                  {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
                                  {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
                                  {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
                                  {0xff}};

const static ec_pdo_entry_reg_t domain_regs[] = {
    // {Alias, Position, Vendor ID, Product Code, PDO Index, -Subindex, Offset}
    {slavePos, ASDA, 0x6040, 0x00, &pdos.Control_Word, NULL},
    {slavePos, ASDA, 0x607a, 0x00, &pdos.Target_Position, NULL},
    {slavePos, ASDA, 0x6041, 0x00, &pdos.Status_Word, NULL},
    {slavePos, ASDA, 0x6064, 0x00, &pdos.Position_Actual_Value, NULL},
    {}};

void check_domain_state(void)
{
    ec_domain_state_t ds;
    ecrt_domain_state(domain, &ds);
    // 从站读取 wkc = 2, 从站读取写入 wkc = 3
    if (ds.working_counter != domain_state.working_counter) {
        printf("发生了不完整的数据帧传输，当前工作计数器为%u\n", ds.working_counter);
    }
    if (ds.wc_state != domain_state.wc_state) {
        printf("工作计数器状态改变为%u\n", ds.wc_state);
        domain_state = ds;
    }
}

// 发送消息的频率设置
// 计时的时候需要使用到
struct period_info {
    struct timespec next_period;
    long period_ns;
    long period_s;
};

static void inc_period(struct period_info *pinfo)
{
    pinfo->next_period.tv_nsec += pinfo->period_ns;
    pinfo->next_period.tv_sec += pinfo->period_s;

    while (pinfo->next_period.tv_nsec >= OS_SYS_NS_PER_SECOND) {
        /* timespec nsec overflow */
        pinfo->next_period.tv_sec++;
        pinfo->next_period.tv_nsec -= OS_SYS_NS_PER_SECOND;
    }
}

static void periodic_task_init(struct period_info *pinfo)
{
    /* for simplicity, hardcoding a 10ms period */
    pinfo->period_ns = OS_SYS_NS_PER_SECOND / frequency;
    pinfo->period_s = 0;

    /* find timestamp to first run */
    clock_gettime(CLOCK_REALTIME, &(pinfo->next_period));
}

static void wait_rest_of_period(struct period_info *pinfo)
{
    inc_period(pinfo);

    /* for simplicity, ignoring possibilities of signal wakes */
    int ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &pinfo->next_period, NULL);
    if (ret != 0) {
        printf("[DEMO] sleep fail, ret:%d\n", ret);
    }
}

uint16_t ctrl_word[] = {0x06, 0x07, 0xF};
uint16_t clearword = 0x80;
#define ready_bits 0xF
#define fault_bit 0x8
#define switch_on_bits 0x6F
#define ready_to_switch_on 0x21
#define switch_on 0x23
#define servo_on 0x27

static int i = 0;
static int reach = 0;

static void do_rt_task()
{
    ecrt_master_receive(master);
    ecrt_domain_process(domain);
    check_domain_state();
    uint16_t raw_value = EC_READ_U16(domain_pd + pdos.Status_Word);
    int32_t current_pos = 0;

    ec_domain_state_t ds;
    printf("Status word:0x%x|", raw_value);
    ecrt_domain_state(domain, &ds);
    if ((raw_value & fault_bit) != 0) {
        printf("\n[TEST]Fault\n");
        EC_WRITE_U16(domain_pd + pdos.Control_Word, clearword);
    }

    if ((raw_value & ready_bits) == 0) {
        EC_WRITE_U16(domain_pd + pdos.Control_Word, ctrl_word[0]);
    }
    if ((raw_value & switch_on_bits) == ready_to_switch_on) {
        printf("\n[TEST]ready to switch on\n");
        EC_WRITE_U16(domain_pd + pdos.Control_Word, ctrl_word[1]);
    }
    if ((raw_value & switch_on_bits) == switch_on) {
        printf("\n[TEST]switch on\n");
        EC_WRITE_U16(domain_pd + pdos.Control_Word, ctrl_word[2]);
    }
    if ((raw_value & switch_on_bits) == servo_on) {
        // pp mode
        current_pos = EC_READ_S32(domain_pd + pdos.Position_Actual_Value);
        printf("\n[TEST] current position:%d, i:%d ||", current_pos, i);
        if ((raw_value & 0x400) && !reach) {
            EC_WRITE_U16(domain_pd + pdos.Control_Word, 0x0F);
            EC_WRITE_S32(domain_pd + pdos.Target_Position, i);
            reach = 1;
            i += 1000000;
            if (i > 10000000) {
                i = 0;
            }
            printf("\nTarget reach?");
        } else {
            reach = 0;
            EC_WRITE_U16(domain_pd + pdos.Control_Word, (0x10 | ctrl_word[2]));
            EC_WRITE_S32(domain_pd + pdos.Target_Position, i);
        }
    }

    ecrt_domain_queue(domain);

    ecrt_master_send(master);
}

void simple_cyclic_task()
{
    struct timespec start;
    struct timespec now;
    printf("cyclic job start\n");
    struct period_info pinfo;
    periodic_task_init(&pinfo);

    clock_gettime(CLOCK_REALTIME, &start);
    clock_gettime(CLOCK_REALTIME, &now);
    // 运行30秒
    while (now.tv_sec < start.tv_sec + 30) {
        do_rt_task();
        wait_rest_of_period(&pinfo);
        clock_gettime(CLOCK_REALTIME, &now);
    }
}

int init_ethercat()
{
    // 启动主站
    ec_slave_config_t *sc;
    struct period_info pinfo;
    int count = 0;
    int ret;

    master = ecrt_request_master(0);
    if (!master) {
        printf("[DEMO] request master fail\n");
        return -1;
    }

    // start configuration
    domain = ecrt_master_create_domain(master);
    if (!domain) {
        return -1;
    }
    if (!(sc = ecrt_master_slave_config(master, slavePos, ASDA))) {
        printf("[DEMO] slave config fail\n");
        return -1;
    }
    printf("[DEMO] config slave PDOS\n");
    if (ecrt_slave_config_pdos(sc, EC_END, slave_0_syncs)) {
        printf("[DEMO] config PDS fail\n");
        return -1;
    }
    // 在domain中注册PDO条目
    printf("[DEMO] reg PDOS entry\n");
    if (ecrt_domain_reg_pdo_entry_list(domain, domain_regs)) {
        printf("[DEMO] PDO reg fail\n");
        return -1;
    }

    ecrt_slave_config_sdo8(sc, 0x6060, 0, 1);
    ecrt_slave_config_sdo32(sc, 0x6081, 0, 1000000);

    uint8_t target[4];
    size_t target_size = sizeof(target);
    size_t result_size;
    uint32_t abort_code;

    // mode
    ecrt_master_sdo_upload(master, 0, 0x6060, 0, target, target_size, &result_size, &abort_code);
    printf("MODE: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // speed
    ecrt_master_sdo_upload(master, 0, 0x6081, 0, target, target_size, &result_size, &abort_code);
    printf("SPEED: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // acc
    ecrt_master_sdo_upload(master, 0, 0x6083, 0, target, target_size, &result_size, &abort_code);
    printf("ACC: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // dec
    ecrt_master_sdo_upload(master, 0, 0x6084, 0, target, target_size, &result_size, &abort_code);
    printf("DEC: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // TARGET
    ecrt_master_sdo_upload(master, 0, 0x607A, 0, target, target_size, &result_size, &abort_code);
    printf("TARGET: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // POS_WINDOW
    ecrt_master_sdo_upload(master, 0, 0x6067, 0, target, target_size, &result_size, &abort_code);
    printf("POS_WINDOW: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // POS_TIME_WINDOW
    ecrt_master_sdo_upload(master, 0, 0x6068, 0, target, target_size, &result_size, &abort_code);
    printf("POS_TIME_WINDOW: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3],
           target[2], target[1], target[0], target_size, result_size, abort_code);

    // POS_LIMIT_ENTRY
    ecrt_master_sdo_upload(master, 0, 0x607D, 0, target, target_size, &result_size, &abort_code);
    printf("POS_LIMIT_ENTRY: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3],
           target[2], target[1], target[0], target_size, result_size, abort_code);

    // POS_LIMIT_MIN
    ecrt_master_sdo_upload(master, 0, 0x607D, 1, target, target_size, &result_size, &abort_code);
    printf("POS_LIMIT_MIN: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // POS_LIMIT_MAX
    ecrt_master_sdo_upload(master, 0, 0x607D, 2, target, target_size, &result_size, &abort_code);
    printf("POS_LIMIT_MAX: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3], target[2],
           target[1], target[0], target_size, result_size, abort_code);

    // MAX_PROFILE_VELOCITY
    ecrt_master_sdo_upload(master, 0, 0x607F, 0, target, target_size, &result_size, &abort_code);
    printf("MAX_PROFILE_VELOCITY: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3],
           target[2], target[1], target[0], target_size, result_size, abort_code);

    // PROFILE_VELOCITY 1000000 p/s 16777216 p per rotation (16.8 seconds per rotation)
    ecrt_master_sdo_upload(master, 0, 0x6081, 0, target, target_size, &result_size, &abort_code);
    printf("PROFILE_VELOCITY: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3],
           target[2], target[1], target[0], target_size, result_size, abort_code);

    // MAX_MOTER_SPED 6000 rpm, 60 rotation per second
    ecrt_master_sdo_upload(master, 0, 0x6080, 0, target, target_size, &result_size, &abort_code);
    printf("MAX_MOTER_SPED: target:%x %x %x %x, target_size:%lu, result_size:%lu, abort_code:%u\n", target[3],
           target[2], target[1], target[0], target_size, result_size, abort_code);

    // 激活master
    if (ecrt_master_activate(master)) {
        return -1;
    }
    printf("activate master success\n");
    if (!(domain_pd = ecrt_domain_data(domain))) {
        printf("get domain data fail\n");
        ecrt_release_master(master);
        return -1;
    }

    return 0;
}

void test_ethercat_main()
{
    int ret;
    printf("[DEMO] test_ethercat_main enter\n");
    ret = init_ethercat();
    if (ret) {
        printf("Failed to init EtherCAT master.\n");
        if (master) {
            ecrt_release_master(master);
        }
        return;
    }
    printf("master init success\n");
    simple_cyclic_task();
    ecrt_release_master(master);
    printf("Finish.\n");
}