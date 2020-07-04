#ifndef __DSP_H
#define __DSP_H

#include <stdint.h>
#include <hal/hal_ustimer.h>


#define MDCONFIG_base           0x0
#define MODEM2G_TOPSM_base      0x0
#define MD2GCONFG_base          0x0
#define SHAREG2_base            0x0

typedef volatile unsigned short     *SRAMADDR;      /* SRAM addr is 16 bits  */
typedef volatile unsigned short      SRAMDATA;      /* SRAM data is 16 bits  */
typedef volatile unsigned short     *APBADDR;       /* APB addr is 32 bits   */
typedef volatile unsigned short      APBDATA;       /* APB data is 16 bits   */
typedef volatile unsigned long      *APBADDR32;     /* APB addr is 32 bits   */
typedef volatile unsigned long       APBDATA32;     /* APB data is 16 bits   */
typedef volatile unsigned short     *DPRAMADDR;     /* DPRAM addr is 16 bits */
typedef volatile signed short       *DPRAMADDR_S;   /* DPRAM addr is 16 bits */
typedef volatile unsigned short      DPRAMDATA;     /* DPRAM data is 16 bits */

#define HW_WRITE(ptr, data) (*(ptr) = (data))
#define HW_READ(ptr) (*(ptr))

#define DSP_READ(addr) HW_READ(addr)
#define DSP_WRITE(addr, data) HW_WRITE(addr, (unsigned short)(data))

//l1_interface.h

#define  MODEM_SLV_SM_WAKEUP_REQ     ((APBADDR32)(MDCONFIG_base     +0x0500)) /* modem software control TOPSM wakeup request reg */
#define  SW_PWR_FORCE_ON_SET         ((APBADDR32)(MODEM2G_TOPSM_base+0x0A14)) /* software power force on set   */
#define  SW_PWR_FORCE_ON_CLR         ((APBADDR32)(MODEM2G_TOPSM_base+0x0A18)) /* software power force on clear */
#define  SW_PWR_STA                  ((APBADDR32)(MODEM2G_TOPSM_base+0x0A24)) /* Software Power Status         */

#define HW_MD2G_POWER_ON()                                                            \
    {                                                                                 \
        HW_WRITE(MODEM_SLV_SM_WAKEUP_REQ, HW_READ(MODEM_SLV_SM_WAKEUP_REQ) | 0x1);    \
        HW_WRITE(SW_PWR_FORCE_ON_SET, 0x1);                                           \
        delay_u(66);                                                                  \
        while (HW_READ(SW_PWR_STA) != 0x1)                                            \
            ;                                                                         \
        HW_WRITE(MODEM_SLV_SM_WAKEUP_REQ, HW_READ(MODEM_SLV_SM_WAKEUP_REQ) & (~0x1)); \
    }
#define HW_MD2G_POWER_OFF()                                                           \
    {                                                                                 \
        HW_WRITE(SW_PWR_FORCE_ON_CLR, 0x1);                                           \
        HW_WRITE(MODEM_SLV_SM_WAKEUP_REQ, HW_READ(MODEM_SLV_SM_WAKEUP_REQ) | 0x1);    \
        ust_busy_wait(33);                                                            \
        HW_WRITE(MODEM_SLV_SM_WAKEUP_REQ, HW_READ(MODEM_SLV_SM_WAKEUP_REQ) & (~0x1)); \
    }

/*--------------------------------------------*/
/* MACRO to enable MD2GSYS bus clock gating   */
/*--------------------------------------------*/
#define MD2GSYS_AHB_CON0 ((APBADDR)(MD2GCONFG_base + 0x0214))
#define HW_MD2GSYS_BUS_CG_ENA()                                           \
    {                                                                     \
        HW_WRITE(MD2GSYS_AHB_CON0, HW_READ(MD2GSYS_AHB_CON0) | (1 << 9)); \
    }

/*--------------------------------------------*/
/* MACRO to set reset and unlock set FD216    */
/*--------------------------------------------*/
#define SHARE_DSPCON ((APBADDR)(SHAREG2_base + 0x0000)) /* DSP Control Register */
#define DSP_RESET_BEGIN 0x0006
#define DSP_RESET_END 0x0007
#define HW_FD216_RESET()                                                   \
    {                                                                      \
        DSP_WRITE(SHARE_DSPCON, DSP_RESET_BEGIN); /* reset FD216 */        \
        DSP_WRITE(SHARE_DSPCON, DSP_RESET_END);   /* unlock reset FD216 */ \
    }

#endif
