#include "ecrt.h"

#define FREQUENCY 1000
#define NUM_DIG_OUT 1

#define NSEC_PER_SECOND (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)

static uint8_t *domain1_pd;

#define BusCouplerPos     0, 0
#define DigOutSlavePos(X) 0, (1 + (X))
#define CounterSlavePos   0, 2

#define Beckhoff_EK1100 0x00000002, 0x044c2c52
#define Beckhoff_EL2008 0x00000002, 0x07d83052
#define IDS_Counter     0x000012ad, 0x05de3052

static int off_dig_out[NUM_DIG_OUT];
static int off_counter_in;
static int off_counter_out;

/*****************************************************************************/

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

/*****************************************************************************/

static ec_pdo_entry_info_t el2008_channels[] = {
    {0x7000, 1, 1},
    {0x7010, 1, 1},
    {0x7020, 1, 1},
    {0x7030, 1, 1},
    {0x7040, 1, 1},
    {0x7050, 1, 1},
    {0x7060, 1, 1},
    {0x7070, 1, 1}
};

static ec_pdo_info_t el2008_pdos[] = {
    {0x1600, 1, &el2008_channels[0]},
    {0x1601, 1, &el2008_channels[1]},
    {0x1602, 1, &el2008_channels[2]},
    {0x1603, 1, &el2008_channels[3]},
    {0x1604, 1, &el2008_channels[4]},
    {0x1605, 1, &el2008_channels[5]},
    {0x1606, 1, &el2008_channels[6]},
    {0x1607, 1, &el2008_channels[7]}
};

static ec_sync_info_t el2008_syncs[] = {
    {0, EC_DIR_OUTPUT, 8, el2008_pdos},
    {1, EC_DIR_INPUT},
    {0xff}
};

int ethercat_main(void)
{
    int ret = -1, i;
    ec_slave_config_t *sc;

    printf("Starting...\n");
    master = ecrt_request_master(0);
    if (!master) {
        printf("Requesting master 0 failed!\n");
        return -1;
    }

    printf("Registering domain...\n");
    domain1 = ecrt_master_create_domain(master);
    if (!domain1) {
        printf("Domain creation failed!\n");
        ret = -2;
        goto out_release_master;
    }

    printf("Configuring PDOs...\n");
    // create configuration for reference clock FIXME
    sc = ecrt_master_slave_config(master, 0, 0, Beckhoff_EK1100);
    if (!sc) {
        printf("Failed to get slave configuration.\n");
        ret = -3;
        goto out_release_master;
    }

    for (i = 0; i < NUM_DIG_OUT; i++) {
        if (!(sc = ecrt_master_slave_config(master,
            DigOutSlavePos(i), Beckhoff_EL2008))) {
            printf("Failed to get slave configuration.\n");
            ret = -4;
            goto out_release_master;
        }

        if (ecrt_slave_config_pdos(sc, EC_END, el2008_syncs)) {
            printf("Failed to configure PDOs.\n");
            ret = -5;
            goto out_release_master;
        }

        off_dig_out[i] = ecrt_slave_config_reg_pdo_entry(sc,
            0x7000, 1, domain1, NULL);

        if (off_dig_out[i] < 0) {
            ret = -6;
            goto out_release_master;
        }
    }

    if (!(sc = ecrt_master_slave_config(master,
        CounterSlavePos, IDS_Counter))) {
        printf("Failed to get slave configuration.\n");
        ret = -7;
        goto out_release_master;
    }
    off_counter_in = ecrt_slave_config_reg_pdo_entry(sc, 
        0x6020, 0x11, domain1, NULL);
    if (off_counter_in < 0) {
        ret = -8;
        goto out_release_master;
    }
    off_counter_out = ecrt_slave_config_reg_pdo_entry(sc,
        0x7020, 1, domain1, NULL);
    if (off_counter_out < 0) {
        ret = -9;
        goto out_release_master;
    }

    // configure SYNC signals for this slave
    ecrt_slave_config_dc(sc, 0x0700, 1000000, 440000, 0, 0);

    printf("Activating master...\n");
    if (ecrt_master_activate(master)) {
        printf("Failed to activate master!\n");
        ret = -10;
        goto out_release_master;
    }

    // Get internal process data for domain
    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        ret = -11;
        goto out_release_master;
    }

    printf("Initialized.\n");
    return 0;

 out_release_master:
    printf("Releasing master...\n");
    ecrt_release_master(master);
    printf("Failed to load. Aborting.\n");
    return ret;
}