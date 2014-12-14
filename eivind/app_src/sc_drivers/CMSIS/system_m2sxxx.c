/*******************************************************************************
 * (c) Copyright 2012 Microsemi Corporation.  All rights reserved.
 * 
 *  SmartFusion2 CMSIS system initialization.
 *
 * SVN $Revision: 5168 $
 * SVN $Date: 2013-03-15 13:01:27 +0000 (Fri, 15 Mar 2013) $
 */
#include "m2sxxx.h"
#include "../drivers_config/sys_config/sys_config.h"
#include "sys_init_cfg_types.h"

/*------------------------------------------------------------------------------
  Silicon revisions.
 */
#define UNKNOWN_SILICON_REV     0
#define M2S050_REV_A_SILICON    1
#define M2S050_REV_B_SILICON    2

/*------------------------------------------------------------------------------
 * 
 */
void mscc_post_hw_cfg_init(void);

/*------------------------------------------------------------------------------
 * System registers of interest.
 */
/*
 * MSSDDR_FACC1_CR register masks:
 */
#define DDR_CLK_EN_SHIFT    8u
#define FACC_GLMUX_SEL_MASK         0x00001000u
#define CONTROLLER_PLL_INIT_MASK    0x04000000u
#define RCOSC_DIV2_MASK             0x00000004u

/*
 * MSSDDR_PLL_STATUS register masks:
 */
#define FAB_PLL_LOCK_MASK   0x00000001u
#define MPLL_LOCK_MASK      0x00000002u

/*
 * MSSDDR_PLL_STATUS_HIGH_CR register masks:
 */
#define FACC_PLL_BYPASS_MASK    0x00000001u

/*------------------------------------------------------------------------------
 * Standard CMSIS global variables.
 */
uint32_t SystemCoreClock = MSS_SYS_M3_CLK_FREQ;         /*!< System Clock Frequency (Core Clock) */

/*------------------------------------------------------------------------------
 * SmartFusion specific clocks.
 */
uint32_t g_FrequencyPCLK0 = MSS_SYS_APB_0_CLK_FREQ;     /*!< Clock frequency of APB bus 0. */  
uint32_t g_FrequencyPCLK1 = MSS_SYS_APB_1_CLK_FREQ;     /*!< Clock frequency of APB bus 1. */
uint32_t g_FrequencyPCLK2 = MSS_SYS_APB_2_CLK_FREQ;     /*!< Clock frequency of APB bus 2. */
uint32_t g_FrequencyFIC0 = MSS_SYS_FIC_0_CLK_FREQ;      /*!< Clock frequecny of FPGA fabric interface controller 1. */
uint32_t g_FrequencyFIC1 = MSS_SYS_FIC_1_CLK_FREQ;      /*!< Clock frequecny of FPGA fabric inteface controller 2. */
uint32_t g_FrequencyFIC64 = MSS_SYS_FIC64_CLK_FREQ;     /*!< Clock frequecny of 64-bit FPGA fabric interface controller. */

/*------------------------------------------------------------------------------
 * System configuration tables generated by Libero.
 */
#if MSS_SYS_MDDR_CONFIG_BY_CORTEX
extern MDDR_TypeDef * const g_m2s_mddr_addr;
extern const ddr_subsys_cfg_t g_m2s_mddr_subsys_config;
#endif

#if MSS_SYS_FDDR_CONFIG_BY_CORTEX
extern FDDR_TypeDef * const g_m2s_fddr_addr;
extern const ddr_subsys_cfg_t g_m2s_fddr_subsys_config;
#endif

#define MSS_SYS_SERDES_CONFIG_BY_CORTEX     (MSS_SYS_SERDES_0_CONFIG_BY_CORTEX || MSS_SYS_SERDES_1_CONFIG_BY_CORTEX || MSS_SYS_SERDES_2_CONFIG_BY_CORTEX || MSS_SYS_SERDES_3_CONFIG_BY_CORTEX)

#if MSS_SYS_SERDES_0_CONFIG_BY_CORTEX
extern const cfg_addr_value_pair_t g_m2s_serdes_0_config[SERDES_0_CFG_NB_OF_PAIRS];
#endif

#if MSS_SYS_SERDES_1_CONFIG_BY_CORTEX
extern const cfg_addr_value_pair_t g_m2s_serdes_1_config[SERDES_1_CFG_NB_OF_PAIRS];
#endif

#if MSS_SYS_SERDES_2_CONFIG_BY_CORTEX
extern const cfg_addr_value_pair_t g_m2s_serdes_2_config[SERDES_2_CFG_NB_OF_PAIRS];
#endif

#if MSS_SYS_SERDES_3_CONFIG_BY_CORTEX
extern const cfg_addr_value_pair_t g_m2s_serdes_3_config[SERDES_3_CFG_NB_OF_PAIRS];
#endif

#define MSS_SYS_CORESF2RESET_USED  (MSS_SYS_MDDR_CONFIG_BY_CORTEX || MSS_SYS_FDDR_CONFIG_BY_CORTEX || MSS_SYS_SERDES_CONFIG_BY_CORTEX)

/*------------------------------------------------------------------------------
 * Local functions:
 */
static uint32_t get_silicon_revision(void);
static void silicon_workarounds(void);
static void m2s050_rev_a_workarounds(void);

#if (MSS_SYS_FACC_INIT_BY_CORTEX == 1)
static void complete_clock_config(void);
#endif

#if MSS_SYS_SERDES_CONFIG_BY_CORTEX
static void configure_serdes_intf(void);
#endif

#if (MSS_SYS_MDDR_CONFIG_BY_CORTEX || MSS_SYS_FDDR_CONFIG_BY_CORTEX)
static void config_ddr_subsys
(
    const ddr_subsys_cfg_t * p_ddr_subsys_cfg,
    DDRCore_TypeDef * p_ddr_subsys_regs
);
#endif

#if MSS_SYS_SERDES_CONFIG_BY_CORTEX
static void config_by_addr_value
(
    const cfg_addr_value_pair_t * p_addr_value_pair,
    uint32_t nb_of_cfg_pairs
);
#endif

static uint32_t get_rcosc_25_50mhz_frequency(void);
static void set_clock_frequency_globals(uint32_t fclk);

/***************************************************************************//**
 * See system_m2sxxx.h for details.
 */
void SystemInit(void)
{
    /*
     * Do not make use of global variables or make any asumptions regarding
     * memory content if modifying this function. The memory content has not been
     * initialised by the time this function is called by the start-up code.
     */
#if (MSS_SYS_FACC_INIT_BY_CORTEX == 1)
    complete_clock_config();
#endif

    silicon_workarounds();
    
    /*--------------------------------------------------------------------------
     * Set STKALIGN to ensure exception stacking starts on 8 bytes address
     * boundary. This ensures compliance with the "Procedure Call Standards for
     * the ARM Architecture" (AAPCS).
     */
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
    
    /*--------------------------------------------------------------------------
     * MDDR configuration
     */
#if MSS_SYS_MDDR_CONFIG_BY_CORTEX
    config_ddr_subsys(&g_m2s_mddr_subsys_config, &g_m2s_mddr_addr->core);
#endif

    /*--------------------------------------------------------------------------
     * MDDR configuration
     */
#if MSS_SYS_FDDR_CONFIG_BY_CORTEX
    config_ddr_subsys(&g_m2s_fddr_subsys_config, &g_m2s_fddr_addr->core);
#endif

    /*--------------------------------------------------------------------------
     * SERDES interfaces configuration.
     */
#if MSS_SYS_SERDES_CONFIG_BY_CORTEX
    configure_serdes_intf();
#endif

    /*--------------------------------------------------------------------------
     * Call user defined configuration function.
     */
    mscc_post_hw_cfg_init();
    
    /*--------------------------------------------------------------------------
     * Synchronize with CoreSF2Reset controlling resets from the fabric.
     */
#if MSS_SYS_CORESF2RESET_USED
    CORE_SF2_CFG->CONFIG_DONE = 1u; /* Signal to CoreSF2Reset that peripheral 
                                       configuration registers have been written.*/
    while(!CORE_SF2_CFG->INIT_DONE)
    {
        ;   /* Wait for INIT_DONE from CoreSF2Reset. */
    }
#endif
}

/***************************************************************************//**
 * SystemCoreClockUpdate()
 */
#define RCOSC_25_50MHZ_CLK_SRC  0u
#define CLK_XTAL_CLK_SRC        1u
#define RCOSC_1_MHZ_CLK_SRC     2u
#define CCC2ASCI_CLK_SRC        3u

#define FACC_STANDBY_SHIFT      6u
#define FACC_STANDBY_SEL_MASK   0x00000007u

#define FREQ_32KHZ   32786u
#define FREQ_1MHZ    1000000u
#define FREQ_25MHZ   25000000u
#define FREQ_50MHZ   50000000u

void SystemCoreClockUpdate(void)
{
#if 1
    uint32_t controller_pll_init;
    uint32_t clk_src;

    controller_pll_init = SYSREG->MSSDDR_FACC1_CR & CONTROLLER_PLL_INIT_MASK;
    
    if(0u == controller_pll_init)
    {
        /* Normal operations. */
        uint32_t global_mux_sel;
        
        global_mux_sel = SYSREG->MSSDDR_FACC1_CR & FACC_GLMUX_SEL_MASK;
        if(0u == global_mux_sel)
        {
            /* MSS clocked from MSS PLL. Use Libero flow defines. */
            SystemCoreClock = MSS_SYS_M3_CLK_FREQ;
            g_FrequencyPCLK0 = MSS_SYS_APB_0_CLK_FREQ;
            g_FrequencyPCLK1 = MSS_SYS_APB_1_CLK_FREQ;
            g_FrequencyPCLK2 = MSS_SYS_APB_2_CLK_FREQ;
            g_FrequencyFIC0 = MSS_SYS_FIC_0_CLK_FREQ;
            g_FrequencyFIC1 = MSS_SYS_FIC_1_CLK_FREQ;
            g_FrequencyFIC64 = MSS_SYS_FIC64_CLK_FREQ;
        }
        else
        {
            /* MSS clocked from standby clock. */
            const uint8_t standby_clock_lut[8] = { RCOSC_25_50MHZ_CLK_SRC,
                                                   CLK_XTAL_CLK_SRC,
                                                   RCOSC_25_50MHZ_CLK_SRC,
                                                   CLK_XTAL_CLK_SRC,
                                                   RCOSC_1_MHZ_CLK_SRC,
                                                   RCOSC_1_MHZ_CLK_SRC,
                                                   CCC2ASCI_CLK_SRC,
                                                   CCC2ASCI_CLK_SRC };
            
            uint32_t standby_sel;
            uint8_t clock_source;
            
            standby_sel = (SYSREG->MSSDDR_FACC2_CR >> FACC_STANDBY_SHIFT) & FACC_STANDBY_SEL_MASK;
            clock_source = standby_clock_lut[standby_sel];
            switch(clock_source)
            {
                case RCOSC_25_50MHZ_CLK_SRC:
                    clk_src = get_rcosc_25_50mhz_frequency();
                    set_clock_frequency_globals(clk_src);
                break;
                
                case CLK_XTAL_CLK_SRC:
                    set_clock_frequency_globals(FREQ_32KHZ);
                break;
                
                case RCOSC_1_MHZ_CLK_SRC:
                    set_clock_frequency_globals(FREQ_1MHZ);
                break;
                
                case CCC2ASCI_CLK_SRC:
                    /* Fall through. */
                default:
                    set_clock_frequency_globals(FREQ_1MHZ);
                break;
            }   
        }
    }
    else
    {
        /* PLL initialization mode. Running from 25/50MHZ RC oscillator. */
        clk_src = get_rcosc_25_50mhz_frequency();
        set_clock_frequency_globals(clk_src);
    }
    
#else
    /*
     * Reset the clock frequency global variables to the values delected in the
     * Libero flow.
     */
    SystemCoreClock = MSS_SYS_M3_CLK_FREQ;
    g_FrequencyPCLK0 = MSS_SYS_APB_0_CLK_FREQ;
    g_FrequencyPCLK1 = MSS_SYS_APB_1_CLK_FREQ;
    g_FrequencyPCLK2 = MSS_SYS_APB_2_CLK_FREQ;
    g_FrequencyFIC0 = MSS_SYS_FIC_0_CLK_FREQ;
    g_FrequencyFIC1 = MSS_SYS_FIC_1_CLK_FREQ;
    g_FrequencyFIC64 = MSS_SYS_FIC64_CLK_FREQ;
#endif
}

/***************************************************************************//**
 * Find out frequency generated by the 25_50mhz RC osciallator.
 */
static uint32_t get_rcosc_25_50mhz_frequency(void)
{
    uint32_t rcosc_div2;
    uint32_t rcosc_frequency;
    
    rcosc_div2 = SYSREG->MSSDDR_PLL_STATUS & RCOSC_DIV2_MASK;
    if(0u == rcosc_div2)
    {
        /* 25_50mhz oscillator is configured for 25 MHz operations. */
        rcosc_frequency = FREQ_25MHZ;
    }
    else
    {
        /* 25_50mhz oscillator is configured for 50 MHz operations. */
        rcosc_frequency = FREQ_50MHZ;
    }
    
    return rcosc_frequency;
}

/***************************************************************************//**
   Set the value of the clock frequency global variables based on the value of
   standby_clk passed as parameter.
   The following global variables are set by this function:
        - SystemCoreClock
        - g_FrequencyPCLK0
        - g_FrequencyPCLK1
        - g_FrequencyPCLK2
        - g_FrequencyFIC0
        - g_FrequencyFIC1
        - g_FrequencyFIC64
 */
static void set_clock_frequency_globals(uint32_t standby_clk)
{
    SystemCoreClock = standby_clk;
    g_FrequencyPCLK0 = standby_clk;
    g_FrequencyPCLK1 = standby_clk;
    g_FrequencyPCLK2 = MSS_SYS_APB_2_CLK_FREQ;
    g_FrequencyFIC0 = standby_clk;
    g_FrequencyFIC1 = standby_clk;
    g_FrequencyFIC64 = standby_clk;
}

/***************************************************************************//**
 * Write 16-bit configuration values into 32-bit word aligned registers.
 */
#if (MSS_SYS_MDDR_CONFIG_BY_CORTEX || MSS_SYS_FDDR_CONFIG_BY_CORTEX)
static void copy_cfg16_to_regs
(
    volatile uint32_t * p_regs,
    const uint16_t * p_cfg,
    uint32_t nb_16bit_words
)
{
    uint32_t inc;
    
    for(inc = 0u; inc < nb_16bit_words; ++inc)
    {
        p_regs[inc] = p_cfg[inc];
    }
}
#endif

/***************************************************************************//**
 * Configure peripheral using register address and register value pairs.
 */
#if MSS_SYS_SERDES_CONFIG_BY_CORTEX
static void config_by_addr_value
(
    const cfg_addr_value_pair_t * p_addr_value_pair,
    uint32_t nb_of_cfg_pairs
)
{
    uint32_t inc;
    
    for(inc = 0u; inc < nb_of_cfg_pairs; ++inc)
    {
        *p_addr_value_pair[inc].p_reg = p_addr_value_pair[inc].value;
    }
}
#endif

/***************************************************************************//**
 * DDR subsystem configuration.
 */
#if (MSS_SYS_MDDR_CONFIG_BY_CORTEX || MSS_SYS_FDDR_CONFIG_BY_CORTEX)

#define NB_OF_DDRC_REGS_TO_CONFIG       57u
#define NB_OF_DDR_PHY_REGS_TO_CONFIG    65u

static void config_ddr_subsys
(
    const ddr_subsys_cfg_t * p_ddr_subsys_cfg,
    DDRCore_TypeDef * p_ddr_subsys_regs
)
{
    volatile uint32_t * p_regs;
    const uint16_t * p_cfg;
    
    /*--------------------------------------------------------------------------
     * Configure DDR controller part of the MDDR subsystem.
     */
    p_cfg = &p_ddr_subsys_cfg->ddrc.DYN_SOFT_RESET_CR;
    p_regs = &p_ddr_subsys_regs->ddrc.DYN_SOFT_RESET_CR;

    copy_cfg16_to_regs(p_regs, p_cfg, NB_OF_DDRC_REGS_TO_CONFIG);
    
    /*--------------------------------------------------------------------------
     * Configure DDR PHY.
     */
    p_cfg = &p_ddr_subsys_cfg->phy.LOOPBACK_TEST_CR;
    p_regs = &p_ddr_subsys_regs->phy.LOOPBACK_TEST_CR;

    copy_cfg16_to_regs(p_regs, p_cfg, NB_OF_DDR_PHY_REGS_TO_CONFIG);
    
    /*--------------------------------------------------------------------------
     * Configure DDR FIC.
     */
    p_ddr_subsys_regs->fic.NB_ADDR_CR = p_ddr_subsys_cfg->fic.NB_ADDR_CR;
    p_ddr_subsys_regs->fic.NBRWB_SIZE_CR = p_ddr_subsys_cfg->fic.NBRWB_SIZE_CR;
    p_ddr_subsys_regs->fic.WB_TIMEOUT_CR = p_ddr_subsys_cfg->fic.WB_TIMEOUT_CR;
    p_ddr_subsys_regs->fic.HPD_SW_RW_EN_CR = p_ddr_subsys_cfg->fic.HPD_SW_RW_EN_CR;
    p_ddr_subsys_regs->fic.HPD_SW_RW_INVAL_CR = p_ddr_subsys_cfg->fic.HPD_SW_RW_INVAL_CR;
    p_ddr_subsys_regs->fic.SW_WR_ERCLR_CR = p_ddr_subsys_cfg->fic.SW_WR_ERCLR_CR;
    p_ddr_subsys_regs->fic.ERR_INT_ENABLE_CR = p_ddr_subsys_cfg->fic.ERR_INT_ENABLE_CR;
    p_ddr_subsys_regs->fic.NUM_AHB_MASTERS_CR = p_ddr_subsys_cfg->fic.NUM_AHB_MASTERS_CR;
    p_ddr_subsys_regs->fic.LOCK_TIMEOUTVAL_CR[0] = p_ddr_subsys_cfg->fic.LOCK_TIMEOUTVAL_1_CR;
    p_ddr_subsys_regs->fic.LOCK_TIMEOUTVAL_CR[1] = p_ddr_subsys_cfg->fic.LOCK_TIMEOUTVAL_2_CR;
    p_ddr_subsys_regs->fic.LOCK_TIMEOUT_EN_CR = p_ddr_subsys_cfg->fic.LOCK_TIMEOUT_EN_CR;

    /*--------------------------------------------------------------------------
     * Enable DDR.
     */
    p_ddr_subsys_regs->ddrc.DYN_SOFT_RESET_CR = 0x01u;
    
    while(0x0000u == p_ddr_subsys_regs->ddrc.DDRC_SR)
    {
        ;
    }        
}

#endif

/***************************************************************************//**
 * Configure SERDES interfaces.
 */
#if MSS_SYS_SERDES_CONFIG_BY_CORTEX

static void configure_serdes_intf(void)
{
#if MSS_SYS_SERDES_0_CONFIG_BY_CORTEX
    config_by_addr_value(g_m2s_serdes_0_config, SERDES_0_CFG_NB_OF_PAIRS);
#endif

#if MSS_SYS_SERDES_1_CONFIG_BY_CORTEX
    config_by_addr_value(g_m2s_serdes_1_config, SERDES_1_CFG_NB_OF_PAIRS);
#endif

#if MSS_SYS_SERDES_2_CONFIG_BY_CORTEX
    config_by_addr_value(g_m2s_serdes_2_config, SERDES_2_CFG_NB_OF_PAIRS);
#endif

#if MSS_SYS_SERDES_3_CONFIG_BY_CORTEX
    config_by_addr_value(g_m2s_serdes_3_config, SERDES_3_CFG_NB_OF_PAIRS);
#endif
}

#endif

/*------------------------------------------------------------------------------
  Retrieve silicon revision from system registers.
 */
static uint32_t get_silicon_revision(void)
{
    uint32_t silicon_revision;
    uint32_t device_version;
    
    device_version = SYSREG->DEVICE_VERSION;
    switch(device_version)
    {
        case 0x0000F802:
            silicon_revision = M2S050_REV_A_SILICON;
            break;
            
        case 0x0001F802:
            silicon_revision = M2S050_REV_B_SILICON;
            break;
            
        default:
            silicon_revision = UNKNOWN_SILICON_REV;
            break;
    }
    
    return silicon_revision;
}

/*------------------------------------------------------------------------------
  Workarounds for various silicon versions.
 */
static void silicon_workarounds(void)
{
    uint32_t silicon_revision;
    
    silicon_revision = get_silicon_revision();
    
    switch(silicon_revision)
    {
        case M2S050_REV_A_SILICON:
            m2s050_rev_a_workarounds();
            break;
            
        case M2S050_REV_B_SILICON:
            /* Fall through. */
        case UNKNOWN_SILICON_REV:
            /* Fall through. */
        default:
            break;
    }
}

/*------------------------------------------------------------------------------
  Silicon workarounds for M2S050 rev A.
 */
static void m2s050_rev_a_workarounds(void)
{
    /*--------------------------------------------------------------------------
     * Work around a couple of silicon issues:
     */
    /* DDR_CLK_EN <- 1 */
    SYSREG->MSSDDR_FACC1_CR |= (uint32_t)1 << DDR_CLK_EN_SHIFT;
    
    /* CONTROLLER_PLL_INIT <- 0 */
    SYSREG->MSSDDR_FACC1_CR = SYSREG->MSSDDR_FACC1_CR & ~CONTROLLER_PLL_INIT_MASK;
}

/*------------------------------------------------------------------------------
  Complete clock configuration if requested by Libero.
 */
#if (MSS_SYS_FACC_INIT_BY_CORTEX == 1)
static void complete_clock_config(void)
{
    uint32_t pll_locked;
    
    /* Wait for fabric PLL to lock. */
    do {
        pll_locked = SYSREG->MSSDDR_PLL_STATUS & FAB_PLL_LOCK_MASK;
    } while(!pll_locked);
    
    /* Negate MPLL bypass. */
    SYSREG->MSSDDR_PLL_STATUS_HIGH_CR &= ~FACC_PLL_BYPASS_MASK;
    
    /* Wait for MPLL to lock. */
    do {
        pll_locked = SYSREG->MSSDDR_PLL_STATUS & MPLL_LOCK_MASK;
    } while(!pll_locked);
    
    /* Switch FACC from standby to run mode. */
    SYSREG->MSSDDR_FACC1_CR &= ~FACC_GLMUX_SEL_MASK;

    /* Negate FPGA_SOFTRESET to de-assert MSS_RESET_N_M2F in the fabric */
    SYSREG->SOFT_RST_CR &= ~SYSREG_FPGA_SOFTRESET_MASK;
}
#endif

