/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2021 Rockchip Electronics Co., Ltd.
 */

#ifndef _HAL_CONF_H_
#define _HAL_CONF_H_

/*
 * The file hal_conf.h.template is the summary of all the driver modules in
 * the HAL, the hal_conf of each specific chip only needs to extract the
 * macro definition of the corresponding module.
 * Rules:
 *     1.Including three part: soc config, driver config and driver sub config.
 *     2.For the optional configuration, add a prominent note.
 */

/************************ HAL SOC Config **************************************/

//#define RKMCU_KOALA
//#define RKMCU_PISCES
//#define RKMCU_RK2106
//#define RKMCU_RK2108
//#define RKMCU_RK2206
//#define SOC_RK1808
//#define SOC_RK3568
#define SOC_RK3588
//#define SOC_RV1108
//#define SOC_RV1126
//#define SOC_SWALLOW

#define HAL_AP_CORE  /**< Only be defined in Asymmetric Multi-Processing AP project */
//#define HAL_MCU_CORE /**< Only be defined in Asymmetric Multi-Processing MCU project */

// #define SYS_TIMER TIMER10 /**< System timer designation (RK TIMER) */

/************************ HAL Driver Config ***********************************/

// #define HAL_ACDCDIG_MODULE_ENABLED
// #define HAL_ACODEC_MODULE_ENABLED
// #define HAL_ARCHTIMER_MODULE_ENABLED
// #define HAL_AUDIOPWM_MODULE_ENABLED
// #define HAL_BUFMGR_MODULE_ENABLED
// #define HAL_CACHE_ECC_MODULE_ENABLED
#define HAL_CANFD_MODULE_ENABLED
// #define HAL_CKCAL_MODULE_ENABLED
// #define HAL_CPU_TOPOLOGY_MODULE_ENABLED
#define HAL_CRU_MODULE_ENABLED
// #define HAL_CRYPTO_MODULE_ENABLED
// #define HAL_DCACHE_MODULE_ENABLED
// #define HAL_DDR_ECC_MODULE_ENABLED
// #define HAL_DEMO_MODULE_ENABLED
// #define HAL_DSI_MODULE_ENABLED
// #define HAL_DSP_MODULE_ENABLED
// #define HAL_DWDMA_MODULE_ENABLED
// #define HAL_EFUSE_MODULE_ENABLED
// #define HAL_EHCI_MODULE_ENABLED
// #define HAL_FSPI_MODULE_ENABLED
// #define HAL_GIC_MODULE_ENABLED
// #define HAL_GMAC_MODULE_ENABLED
// #define HAL_GPIO_IRQ_GROUP_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
// #define HAL_HCD_MODULE_ENABLED
// #define HAL_HWSPINLOCK_MODULE_ENABLED
// #define HAL_HYPERPSRAM_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
// #define HAL_I2S_MODULE_ENABLED
// #define HAL_I2STDM_MODULE_ENABLED
// #define HAL_ICACHE_MODULE_ENABLED
// #define HAL_INTC_MODULE_ENABLED
// #define HAL_IRQ_HANDLER_MODULE_ENABLED
// #define HAL_KEYCTRL_MODULE_ENABLED
// #define HAL_MBOX_MODULE_ENABLED
// #define HAL_NVIC_MODULE_ENABLED
// #define HAL_OHCI_MODULE_ENABLED
// #define HAL_PCD_MODULE_ENABLED
// #define HAL_PCIE_MODULE_ENABLED
// #define HAL_PDM_MODULE_ENABLED
#define HAL_PINCTRL_MODULE_ENABLED
// #define HAL_PL330_MODULE_ENABLED
// #define HAL_PM_CPU_SLEEP_MODULE_ENABLED
// #define HAL_PM_RUNTIME_MODULE_ENABLED
// #define HAL_PM_SLEEP_MODULE_ENABLED
// #define HAL_PMU_MODULE_ENABLED
// #define HAL_PVTM_MODULE_ENABLED
// #define HAL_PWM_MODULE_ENABLED
// #define HAL_PWR_INTBUS_MODULE_ENABLED
// #define HAL_PWR_MODULE_ENABLED
// #define HAL_QPIPSRAM_MODULE_ENABLED
// #define HAL_RISCVIC_MODULE_ENABLED
// #define HAL_SARADC_MODULE_ENABLED
// #define HAL_SDIO_MODULE_ENABLED
// #define HAL_SFC_MODULE_ENABLED
// #define HAL_SMCCC_MODULE_ENABLED
// #define HAL_SNOR_MODULE_ENABLED
// #define HAL_SPI2APB_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
// #define HAL_SPINAND_MODULE_ENABLED
// #define HAL_SPINLOCK_MODULE_ENABLED
// #define HAL_SYSTICK_MODULE_ENABLED
#define HAL_TIMER_MODULE_ENABLED
// #define HAL_TOUCHKEY_MODULE_ENABLED
// #define HAL_TSADC_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
// #define HAL_VAD_MODULE_ENABLED
// #define HAL_VICAP_MODULE_ENABLED
// #define HAL_VOP_MODULE_ENABLED
// #define HAL_WDT_MODULE_ENABLED

/************************ HAL Driver Sub Config *******************************/

//#define HAL_DBG_USING_RTT_SERIAL
//#define HAL_DBG_USING_LIBC_PRINTF
// #define HAL_DBG_ON
// #define HAL_DBG_INFO_ON
// #define HAL_DBG_WRN_ON
// #define HAL_DBG_ERR_ON
// #define HAL_ASSERT_ON

#ifdef HAL_CRU_MODULE_ENABLED
#define HAL_CRU_AS_FEATURE_ENABLED
#endif

#ifdef HAL_FSPI_MODULE_ENABLED
#define HAL_FSPI_XIP_ENABLE /**< FSPI supports XIP by default */
// #define HAL_FSPI_ALIGNED_CHECK_BYPASS /**< Disable memery unalighed check if these behavior is support */
#endif

#ifdef HAL_GIC_MODULE_ENABLED
#define HAL_GIC_AMP_FEATURE_ENABLED
#define HAL_GIC_PREEMPT_FEATURE_ENABLED
#define HAL_GIC_WAIT_LINUX_INIT_ENABLED
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
#define HAL_GPIO_VIRTUAL_MODEL_FEATURE_ENABLED
#endif

#ifdef HAL_GPIO_IRQ_GROUP_MODULE_ENABLED
#define HAL_GPIO_IRQ_GROUP_PRIO_LEVEL_MAX (3) /**< max priority level */
#endif

#ifdef HAL_HWSPINLOCK_MODULE_ENABLED
#define HAL_HWSPINLOCK_OWNER_ID 1
#endif

#ifdef HAL_QPIPSRAM_MODULE_ENABLED
#define HAL_QPIPSRAM_HALF_SLEEP_ENABLED /**< If support half sleep, configure it */
#endif

#ifdef HAL_SNOR_MODULE_ENABLED
#define HAL_SNOR_FSPI_HOST /**< Choose SNOR Host: FSPI */
//#define HAL_SNOR_SPI_HOST /**< Choose SNOR Host: SPI */
//#define HAL_SNOR_SFC_HOST /**< Choose SNOR Host: SFC */
#endif

#ifdef HAL_WDT_MODULE_ENABLED
#define HAL_WDT_DYNFREQ_FEATURE_ENABLED
#endif

#endif
