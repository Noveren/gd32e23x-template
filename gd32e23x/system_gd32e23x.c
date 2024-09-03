
#include "gd32e23x_libopt.h"
// #define __SYSTEM_CLOCK_24M_PLL_IRC8M_DIV2        (uint32_t)24000000
// #define __SYSTEM_CLOCK_32M_PLL_IRC8M_DIV2        (uint32_t)32000000
// #define __SYSTEM_CLOCK_72M_PLL_IRC8M_DIV2        (uint32_t)72000000


#define SEL_IRC8M       0x00
#define SEL_HXTAL       0x01
#define SEL_PLL         0x02

//   AHB      WSCNT
// <= 24MHz     0
// <= 48MHz     1
// <= 72MHz     2
#ifdef __SYSTEM_CLOCK_24M_PLL_IRC8M_DIV2
    uint32_t SystemCoreClock = __SYSTEM_CLOCK_24M_PLL_IRC8M_DIV2;
    #define IRC8M_DIV2_PLL_MULx  RCU_PLL_MUL6
    #define WS_WSCNT_x           WS_WSCNT_0
#elif defined (__SYSTEM_CLOCK_32M_PLL_IRC8M_DIV2)
    uint32_t SystemCoreClock = __SYSTEM_CLOCK_32M_PLL_IRC8M_DIV2;
    #define IRC8M_DIV2_PLL_MULx  RCU_PLL_MUL8
    #define WS_WSCNT_x           WS_WSCNT_1
#elif defined (__SYSTEM_CLOCK_72M_PLL_IRC8M_DIV2)
    uint32_t SystemCoreClock = __SYSTEM_CLOCK_72M_PLL_IRC8M_DIV2;
    #define IRC8M_DIV2_PLL_MULx  RCU_PLL_MUL18
    #define WS_WSCNT_x           WS_WSCNT_2
#else
    #error "Please select the __SYSTEM_CLOCK__xx (look at clock.c) used in your application (in conf.h file)"
#endif

static void system_clock_config(void) {
        uint32_t timeout = 0U;
        uint32_t stab_flag = 0U;

        // Enable IRC8M
        RCU_CTL0 |= RCU_CTL0_IRC8MEN;
        do {
            timeout++;
            stab_flag = (RCU_CTL0 & RCU_CTL0_IRC8MSTB);
        } while((0U == stab_flag) && (IRC8M_STARTUP_TIMEOUT != timeout));
        if (0U == (RCU_CTL0 & RCU_CTL0_IRC8MSTB)) {
            // Failed to enable IRC8M
            while(1);
        }

        // Flash Wait Status: 72MHz
        FMC_WS = (FMC_WS & (~FMC_WS_WSCNT)) | WS_WSCNT_x;

        RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;                      // AHB  = CK_SYS
        RCU_CFG0 |= RCU_APB2_CKAHB_DIV1;                     // APB2 = AHB
        RCU_CFG0 |= RCU_APB1_CKAHB_DIV1;                     // APB1 = AHB
        RCU_CFG0 &= ~(RCU_CFG0_PLLSEL | RCU_CFG0_PLLMF);
        RCU_CFG0 |= (RCU_PLLSRC_IRC8M_DIV2 | IRC8M_DIV2_PLL_MULx);

        // Enable PLL and Select PLL as system clock
        RCU_CTL0 |= RCU_CTL0_PLLEN;
        while (0U == (RCU_CTL0 & RCU_CTL0_PLLSTB));
        RCU_CFG0 &= ~RCU_CFG0_SCS;                           // CK_SYS = PLL
        RCU_CFG0 |= RCU_CKSYSSRC_PLL;
        while (RCU_SCSS_PLL != (RCU_CFG0 & RCU_CFG0_SCSS));
    }

void SystemInit(void) {
    // Enable IRC8M
    RCU_CTL0 |= RCU_CTL0_IRC8MEN;
    while(0U == (RCU_CTL0 & RCU_CTL0_IRC8MSTB));

    // Reset RCU
    RCU_CFG0 &= ~RCU_CFG0_SCS;  // Select IRC8M as system clock source
    RCU_CTL0 &= ~(RCU_CTL0_HXTALEN | RCU_CTL0_CKMEN | RCU_CTL0_PLLEN | RCU_CTL0_HXTALBPS);
    RCU_CFG0 &= ~(RCU_CFG0_SCS | RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC |\
                  RCU_CFG0_ADCPSC | RCU_CFG0_CKOUTSEL | RCU_CFG0_CKOUTDIV | RCU_CFG0_PLLDV);
    RCU_CFG0 &= ~(RCU_CFG0_PLLSEL | RCU_CFG0_PLLMF | RCU_CFG0_PLLMF4 | RCU_CFG0_PLLDV);
    RCU_CFG1 &= ~(RCU_CFG1_PREDV);
    RCU_CFG2 &= ~(RCU_CFG2_USART0SEL | RCU_CFG2_ADCSEL);
    RCU_CFG2 &= ~RCU_CFG2_IRC28MDIV;
    RCU_CFG2 &= ~RCU_CFG2_ADCPSC2;
    RCU_CTL1 &= ~RCU_CTL1_IRC28MEN;
    RCU_INT = 0x00000000U;

    system_clock_config();

    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x00);
}

void SystemCoreClockUpdate(void) {
    uint32_t sws = 0U;
    uint32_t pllmf = 0U, pllmf4 = 0U, pllsel = 0U, prediv = 0U, idx = 0U, clk_exp = 0U;
    /* exponent of AHB clock divider */
    const uint8_t ahb_exp[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

    sws = GET_BITS(RCU_CFG0, 2, 3);
    switch(sws){
    /* IRC8M is selected as CK_SYS */
    case SEL_IRC8M:
        SystemCoreClock = IRC8M_VALUE;
        break;
    /* HXTAL is selected as CK_SYS */
    case SEL_HXTAL:
        SystemCoreClock = HXTAL_VALUE;
        break;
    /* PLL is selected as CK_SYS */
    case SEL_PLL:
        /* get the value of PLLMF[3:0] */
        pllmf = GET_BITS(RCU_CFG0, 18, 21);
        pllmf4 = GET_BITS(RCU_CFG0, 27, 27);
        /* high 16 bits */
        if (1U == pllmf4) {
            pllmf += 17U;
        } else if(15U == pllmf) {
            pllmf = 16U;
        } else {
            pllmf += 2U;
        }
        
        /* PLL clock source selection, HXTAL or IRC8M/2 */
        pllsel = GET_BITS(RCU_CFG0, 16, 16);
        if (0U != pllsel) {
            prediv = (GET_BITS(RCU_CFG1, 0, 3) + 1U);
            SystemCoreClock = (HXTAL_VALUE / prediv) * pllmf;
        } else {
            SystemCoreClock = (IRC8M_VALUE >> 1) * pllmf;
        }
        break;
    /* IRC8M is selected as CK_SYS */
    default:
        SystemCoreClock = IRC8M_VALUE;
        break;
    }
    /* calculate AHB clock frequency */
    idx = GET_BITS(RCU_CFG0, 4, 7);
    clk_exp = ahb_exp[idx];
    SystemCoreClock >>= clk_exp;
}