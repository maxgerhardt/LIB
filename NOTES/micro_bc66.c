// get micro() seconds ... BC66

#include <stdint.h>

typedef struct
{
    union {
        struct
        {
            volatile uint8_t EN;
            volatile uint8_t MODE;
            volatile uint8_t SW_CG;
            const volatile uint8_t RESERVED[1];
        } GPT_CON_CELLS;
        volatile uint32_t GPT_CON;
    } GPT_CON_UNION;
    volatile uint32_t GPT_CLR;
    volatile uint32_t GPT_CLK;
    volatile uint32_t GPT_IRQ_EN;
    const volatile uint32_t GPT_IRQ_STA;
    volatile uint32_t GPT_IRQ_ACK;
    volatile uint32_t GPT_COUNT;
    volatile uint32_t GPT_COMPARE;
    volatile uint32_t GPT_COUNTH;
    volatile uint32_t GPT_COMPAREH;
} GPT_REGISTER_T;

#define PDN_COND0_F_PDR_COND0 (*(volatile uint32_t *)0xA21D0300)
#define PDN_SETD0_F_PDR_SETD0 (*(volatile uint32_t *)0xA21D0310)
#define PDN_CLRD0_F_PDR_CLRD0 (*(volatile uint32_t *)0xA21D0320)

void gpt_open_clock_source(void) // uSec base clock
{
    if (PDN_COND0_F_PDR_COND0 & 0x800000)
        PDN_CLRD0_F_PDR_CLRD0 = 0x800000; // enable clock source, maybe is enabled by default
}

void gpt_start_free_run_timer(GPT_REGISTER_T *gpt, uint32_t clock_source, uint32_t divide)
{
    gpt_open_clock_source();
    gpt->GPT_CLK = clock_source | divide;
    gpt->GPT_CON_UNION.GPT_CON = 0x301;
}

uint32_t get_current(GPT_REGISTER_T *gpt)
{
    return gpt->GPT_COUNT;
}

void start_us_timer(void)
{
    gpt_start_free_run_timer((GPT_REGISTER_T *)0xA21300D0, 0, 12u); // this timer is free for use,  used from Ql_Delay_us()
}

#define MICRO() ((GPT_REGISTER_T *)0xA21300D0)->GPT_COUNT

int test(void)
{
    start_us_timer();
    int a = MICRO();
    int b = MICRO();
    return a + MICRO() + b;
}