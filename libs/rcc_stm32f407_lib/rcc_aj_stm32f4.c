/*******************************************************************************
 * File Name:    rcc_aj_stm32f4.c
 *
 * Description:
 * Contains function definition for RCC peripheral.
 *
 * Related Document: See README.md
 *
 ******************************************************************************/
/* The functions in this file may be called by the user in "system_stm32f4xx.c"
 *  in the SystemInit() function also.
 *
 * * Sample usage shown below -
 *
 *   void SystemInit(void)
 *    {
 *        // Configure MCO and HSE clock below - temporary code segment remove if not
 *        working on it.
 *
 *        MCO_Config(MCO_CHANNEL_1, MCO1_CLOCK_SOURCE_PLL, MCO_PRESCALER_BY_4);
 *
 *        // Done as per reference manual for compensating CPU clock period and Flash
 *        memory access time.
 *        FLASH->ACR |= (5 << FLASH_ACR_LATENCY_Pos) | (1 << FLASH_ACR_PRFTEN_Pos)
 *        | (1 << FLASH_ACR_ICEN_Pos) | (1 << FLASH_ACR_DCEN_Pos);
 *
 *        // Configures the PLL and routes it to system clock.
 *        RCC_System_Clock_Source_Config(SYS_CLOCK_SOURCE_PLL, PLL_CLOCK_SOURCE_HSE);
 *
 *        //Needs to be called as mentioned in it's description.
 *        SystemCoreClockUpdate();
 *
 *        #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
 *        SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
 *        #endif
 *
 *        #if defined (DATA_IN_ExtSRAM) || defined (DATA_IN_ExtSDRAM)
 *        SystemInit_ExtMemCtl();
 *        #endif
 *
 *        #ifdef VECT_TAB_SRAM
 *          SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;
 *        #else
 *          SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
 *        #endif
 *    }
 */
#include "rcc_aj_stm32f4.h"

/*******************************************************************************
* Function Name: MCO_Config
********************************************************************************
* Summary:
*   Configures the MCO to output the clock signals generated by the selected clock
* sources.
*
* Parameters:
*  mco_channel:         MCO channel to output the clock signal on it's assosciated
                        pin.
*
*  mco_clock_source:    The MCO pin will output this clock's signal.
*
* Return :
*  void
*
*******************************************************************************/
void MCO_Config(MCO_CHANNEL_e mco_channel, MCO_CLOCK_SOURCE_e mco_clock_source,
                MCO_PRESCALER_e mco_prescaler)
{
    if (MCO_CHANNEL_1 == mco_channel)
    {
        /* enable GPIOA clock */
        RCC->AHB1ENR |= (1U << RCC_AHB1ENR_GPIOAEN_Pos);

        /* alternate function(AF) for GPIOA (PA8) */
        GPIOA->MODER |= (GPIOA->MODER & ~(GPIO_MODER_MODER8_Msk))|
                                                (0x2U << GPIO_MODER_MODER8_Pos);

        /* since AF is MCO, setting very high speed for GPIOA (PA8) */
        GPIOA->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED8_Pos);

        /* select MCO1 clock source */
        RCC->CFGR |= (RCC->CFGR & (~(3U << RCC_CFGR_MCO1_Pos))) |
                     (mco_clock_source << RCC_CFGR_MCO1_Pos);

        /* select MCO1 prescaler */
        RCC->CFGR |= (RCC->CFGR & (~(7U << RCC_CFGR_MCO1PRE_Pos))) |
                     (mco_prescaler << RCC_CFGR_MCO1PRE_Pos);
    }

    else if (MCO_CHANNEL_2 == mco_channel)
    {
        /* enable GPIOC clock */
        RCC->AHB1ENR |= (1U << RCC_AHB1ENR_GPIOCEN_Pos);

        /* alternate function for GPIOC (PC9) */
        GPIOC->MODER |= (GPIOC->MODER & ~(GPIO_MODER_MODER9_Msk))|
                                                    (2U << (GPIO_MODER_MODER9_Pos));

        /* since AF is MCO, setting very high speed for GPIOC (PC9) */
        GPIOC->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED9_Pos);

        /* select MCO2 clock source */
        RCC->CFGR |= (RCC->CFGR & (~(3U << RCC_CFGR_MCO2_Pos))) |
                     (mco_clock_source << RCC_CFGR_MCO2_Pos);

        /* select MCO2 prescaler */
        RCC->CFGR |= (RCC->CFGR & (~(7U << RCC_CFGR_MCO2PRE_Pos))) |
                     (mco_prescaler << RCC_CFGR_MCO2PRE_Pos);
    }
}

/*******************************************************************************
 * Function Name: RCC_System_Clock_Source_Config
 ********************************************************************************
 * Summary:
 *   Configures the system clock based on selected clock source.
 *
 * IMP NOTE: Before calling this function configure the FLASH access control.
 *
 * Parameters:
 *  system_clock_source:     Clock source for system clock.
 *
 *  pll_clock_source:        Clock source for the PLL (pass 0 if PLL not used)
 *
 *  rcc_pll_config_param_t:  PLL config divider values (pass NULL if PLL not used)
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void RCC_System_Clock_Source_Config(int system_clock_source, int pll_clock_source,
                                RCC_PLL_CONFIG_PARAMS_t *rcc_pll_config_param_t)
{
    int i = 0;

    /* If HSI needed as system clock */
    if (SYS_CLOCK_SOURCE_HSI == system_clock_source)
    {
        /* switch on HSI and turn off other sources */
        RCC->CR |= (1U << RCC_CR_HSION_Pos) & (~(1U << RCC_CR_HSEON_Pos)) &
                   (~(1U << RCC_CR_PLLON_Pos));

        /* wait till HSI is ready. */
        while (!((1U << RCC_CR_HSIRDY_Pos) & ((RCC->CR))));
    }

    /* If HSE needed as system clock */
    else if (SYS_CLOCK_SOURCE_HSE == system_clock_source)
    {
        /* switch on HSE and turn off other sources */
        RCC->CR |= (1U << RCC_CR_HSEON_Pos) & (~(1U << RCC_CR_HSION_Pos)) &
                   (~(1 << RCC_CR_PLLON_Pos));

        /* wait till HSE is ready. */
        while (!((1U << RCC_CR_HSERDY_Pos) & ((RCC->CR))));
    }

    else if (SYS_CLOCK_SOURCE_PLL == system_clock_source)
    {
        /* Disable the pll before configuring it. */
        RCC->CR &= (~(1U << RCC_CR_PLLON_Pos));
        while (((RCC->CR >> RCC_CR_PLLRDY_Pos) & (0x1U)));

        /* Convert the PLLP value into bits as required by the PLLP field of
           RCC_PLLCFGR */
        rcc_pll_config_param_t->PLLP = (rcc_pll_config_param_t->PLLP) / 2 - 1;

        if (PLL_CLOCK_SRC_HSI == pll_clock_source)
        {
            /* Switch on HSI and turn off other sources. */
            RCC->CR |= (1U << RCC_CR_HSION_Pos) & (~(1U << RCC_CR_HSEON_Pos)) &
                       (~(1U << RCC_CR_PLLON_Pos));

            /* Wait till HSI is ready */
            while (!((1U << RCC_CR_HSIRDY_Pos) & ((RCC->CR))));
        }

        else if (PLL_CLOCK_SRC_HSE == pll_clock_source)
        {
            /* Switch on HSE and turn off other sources */
            RCC->CR |= (1U << RCC_CR_HSEON_Pos) & (~(1U << RCC_CR_HSION_Pos)) &
                       (~(1U << RCC_CR_PLLON_Pos));

            /* Wait till HSE is ready */
            while (!((1U << RCC_CR_HSERDY_Pos) & ((RCC->CR))));
        }

        /* Random delay to ensure the register setting properly done (try improv) */
        for (i = 0; i < 15000; i++);

        /* clear old PLL config value */
        RCC->PLLCFGR &= (~(RCC_PLLCFGR_PLLM_Msk)) & (~(RCC_PLLCFGR_PLLN_Msk)) &
                        (~(RCC_PLLCFGR_PLLP_Msk)) & (~(RCC_PLLCFGR_PLLQ_Msk))
                                                & (~RCC_PLLCFGR_PLLSRC_Msk);

        /* set the PLL config value */
        RCC->PLLCFGR |= (rcc_pll_config_param_t->PLLM << RCC_PLLCFGR_PLLM_Pos) |
                        (rcc_pll_config_param_t->PLLN << RCC_PLLCFGR_PLLN_Pos) |
                        (rcc_pll_config_param_t->PLLP << RCC_PLLCFGR_PLLP_Pos) |
                        (rcc_pll_config_param_t->PLLQ << RCC_PLLCFGR_PLLQ_Pos) |
                        (pll_clock_source << RCC_PLLCFGR_PLLSRC_Pos);
   
        /* random delay to ensure the register setting properly done */
        for (i = 0; i < 15000; i++);

        /* clear old values in CFGR,  */
        RCC->CFGR &= (~(RCC_CFGR_HPRE_Msk)) & (~(RCC_CFGR_PPRE1_Msk)) &
                     (~(RCC_CFGR_PPRE2_Msk));

        /* Random delay to ensure the register setting properly done (try improv) */
        for (i = 0; i < 15000; i++);

        /* Random delay to ensure the register setting properly done. */
        for (i = 0; i < 15000; i++);

        /* Enable the pll */
        RCC->CR |= (1U << RCC_CR_PLLON_Pos);

        /* Wait till PLL locked */
        while (!((RCC->CR >> RCC_CR_PLLRDY_Pos) & (0x1U)));
    }

    /* Select the configured system clock source */
    RCC->CFGR |= (RCC->CFGR & (~(3U << RCC_CFGR_SW_Pos))) |
                 (system_clock_source << RCC_CFGR_SW_Pos);

    /* Wait till system clock switched to selected source */
    while (!(((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3U) == system_clock_source));

    /*
     * All the system RCC clock buses like AHB(HPRE), APB1(PPRE1),
     * APB2(PPRE2) frequencies should be within the parameters
     * as mentioned in datasheet or user-manual, this will be done in the
     * function - system_clock_setting(), called after SystemCoreClockUpdate().
     */

    /* Currently AHB clock is undivided and will be maximum based on above
     * settings.
     */
    RCC->CFGR &= (~(0xF << RCC_CFGR_HPRE_Pos));
}

/*******************************************************************************
* Function Name: system_clock_setting()
********************************************************************************
* Summary:
*   Configure the clocks of buses like APB1(PPRE1), APB2(PPRE2) and RTC with their
* max values within the parameters as mentioned in user-manual and datasheet.
*
* This function always needs to be called after system core clocks are updated.
*
* Parameters:
*   sys_core_clock:     Updated and latest System core clock value.
*                       Should be considered after SystemCoreClockUpdate().
*
* sys_bus_clk_cfg:  Pointer to system bus clock configs.
*
* Return :
*  void
*
*******************************************************************************/
void system_clock_setting(uint32_t sys_core_clock, system_bus_clk_cfg_t *sys_bus_clk_cfg)
{
    /* Set the PPRE1 (APB1) bus clock prescaler */
    RCC->CFGR |= (uint32_t)((RCC->CFGR & (~(RCC_CFGR_PPRE1_Msk))) |
                            ((sys_bus_clk_cfg->ppre1_apb1_pre)));

    /* Set the PPRE2 (APB2) bus clock prescaler */
    RCC->CFGR |= (uint32_t)((RCC->CFGR & (~(RCC_CFGR_PPRE2_Msk))) |
                        ((sys_bus_clk_cfg->ppre2_apb2_pre)));

    /* Set the RTC bus clock prescaler */
    RCC->CFGR |= (uint32_t)((RCC->CFGR & (~(RCC_CFGR_RTCPRE_Msk))) |
									((sys_bus_clk_cfg->rtcpre_pre)));
}

/*******************************************************************************
 * Function Name: systick_config_init()
 ********************************************************************************
 * Summary:
 *   Configures and initializes the systick timer based on the (reload value + 1)
 *   supplied as ticks and the interrupt mode.
 *
 * The clock source for systick initialized here will be the processor clock.
 * No calibration field used.
 *
 * (reload value + 1) = (reqd_time_period_sec)/(source_clock_period_sec);
 *
 * NOTE: The "ticks" parameter required for this function may need to be
 * calculated carefully, keeping in mind the internal delays of the HW and SW.
 *
 * Parameters:
 *  ticks:               Pass the (reload value + 1) as calculated above.
 *  interrupt_mode:      Enable/disable systick interrupt.
 *
 * Return :
 *  int:                 Result of systick init.
 *
 *******************************************************************************/
int systick_config_init(uint32_t interrupt_mode, uint32_t ticks)
{
    if ((ticks - 1) > SYSTICK_RELOAD_VAL_MAX)
    {
        return RCC_FAIL_FLAG;
    }

    /* Disable the systick counter */
    SysTick->CTRL = 0;

    /* Program the reload value, systick counts down from this value to zero */
    SysTick->LOAD = ticks - 1UL;

    /* Clear the current value of systick timer, any value can be written here */
    SysTick->VAL = 0;

    /* Configure systick control register, currently will use the processor clock */
    SysTick->CTRL |= (1UL << SysTick_CTRL_ENABLE_Pos) |
                     (interrupt_mode << SysTick_CTRL_TICKINT_Pos) |
                     (1UL << SysTick_CTRL_CLKSOURCE_Pos);

    return RCC_SUCCESS_FLAG;
}

/*******************************************************************************
 * Function Name: delay_ms_systick()
 ********************************************************************************
 * Summary:
 *       Creates a mili-second delay of specified value.
 *
 * NOTE:
 *  1. This function assumes systick is configured for 1us time period.
 *  2. Accuracy may need improvement. Currently may give error of about 2%.
 *       Accuracy will depend on the systick config.
 *
 * Parameters:
 *       ms_delay:   Required Delay amount in ms. The result of this value
 *                   multiplied with 1000 should be in range of uint32_t, inside
 *                   the function this supplied parameter value will be
 *                   multiplied with 1000 for internal use.
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void delay_ms_systick(uint32_t ms_delay)
{
    ms_delay *= 1000;

    while (ms_delay--)
    {
        SysTick->VAL = 0;
        while (!(SysTick->CTRL & (1UL << SysTick_CTRL_COUNTFLAG_Pos)));
    }
}

/*******************************************************************************
 * Function Name: delay_us_systick()
 ********************************************************************************
 * Summary:
 *       Creates a micro-second delay of specified value.
 *
 * NOTE:
 *  1. This function assumes systick is configured for 1us time period.
 *  2. Accuracy may need improvement. Currently may give error of about 2%.
 *       Accuracy will depend on the systick config.
 *
 * Parameters:
 *       us_delay:   Delay amount in us.
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void delay_us_systick(uint32_t us_delay)
{
    while (us_delay--)
    {
        SysTick->VAL = 0;
        while (!(SysTick->CTRL & (1UL << SysTick_CTRL_COUNTFLAG_Pos)));
    }
}

/* To Do */
void systick_deconfig()
{
}
